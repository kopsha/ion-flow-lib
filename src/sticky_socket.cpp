#include "helpers.h"
#include "logs.h"
#include "sticky_socket.h"

#include <bit>
#include <cerrno>
#include <cstddef>
#include <cstring>
#include <span>
#include <string>
#include <system_error>
#include <utility>

#include <fcntl.h>
#include <netdb.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>

namespace {

auto setSocketNonBlocking(int socket_fd) -> bool
{
    int flags = fcntl(socket_fd, F_GETFL, 0);
    if (flags < 0) {
        return false;
    }
    flags |= O_NONBLOCK;
    auto err = fcntl( // NOLINT(cppcoreguidelines-pro-type-vararg)
        socket_fd, F_SETFL, std::bit_cast<int>(flags)
    );
    return err == 0;
}

} // anonymous namespace

StickySocket::StickySocket(std::string host, int port, const int retries)
    : descriptor { INVALID_SOCKET }
    , host { std::move(host) }
    , port { std::to_string(port) }
    , maxRetries(retries)
    , attempts(0)
    , backOff(0)
    , status(ConnectionState::Disconnected)
    , online(false)
    , rxBuffer {}
{
    open(); // why?
}

StickySocket::~StickySocket() { disconnect(); }

auto StickySocket::open() -> bool
{
    if (descriptor != INVALID_SOCKET) {
        log_debug("socket open discarded, socket already connecting %s\n", host.c_str());
        return true;
    }

    struct addrinfo hints {};
    struct addrinfo* results = nullptr;
    std::memset(&hints, 0, sizeof(hints));

    // Request address info
    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = SOCK_STREAM; // TCP
    int err = getaddrinfo(host.c_str(), port.c_str(), &hints, &results);
    if (err != 0) {
        log_error("Cannot resolve %s, reason: %s", host.c_str(), gai_strerror(err));
        descriptor = INVALID_SOCKET;
        return false;
    }

    // Find the first valid socket with requested addr info
    for (const auto* result = results; result != nullptr; result = result->ai_next) {
        descriptor = ::socket(result->ai_family, result->ai_socktype, result->ai_protocol);
        if (descriptor == INVALID_SOCKET) {
            continue; // Try the next address
        }

        // Set the socket to non-blocking mode
        if (!setSocketNonBlocking(descriptor)) {
            ::close(descriptor);
            descriptor = INVALID_SOCKET;
            continue; // Try the next address
        }

        // Initiate the connection (non-blocking)
        err = ::connect(descriptor, result->ai_addr, result->ai_addrlen);
        if (err < 0) {
            if (errno == EINPROGRESS) {
                // Connection is in progress, which is expected for non-blocking sockets
                break;
            }

            // Other connection error
            log_error("Connection failed: %s", std::system_category().message(errno));
            ::close(descriptor);
            descriptor = INVALID_SOCKET;
        } else if (err == 0) {
            // Connection succeeded immediately (unlikely for non-blocking sockets)
            break;
        }
    }

    freeaddrinfo(results);

    return descriptor != INVALID_SOCKET;
}

void StickySocket::close()
{
    ::close(descriptor);
    descriptor = INVALID_SOCKET;
}

void StickySocket::connect()
{
    log_debug("connect call %s\n", host.c_str());
    if (status != ConnectionState::Disconnected) {
        log_warning("%s connection is already in progress.\n", host.c_str());
        return;
    }
    attempts = 0;
    reconnect();
}

void StickySocket::reconnect()
{
    log_debug(" -> reconnect() on %s\n", host.c_str());
    attempts++;
    if (open()) {
        enter(ConnectionState::Connecting);
        log_debug("%s is %s\n", host.c_str(), getStatus().c_str());
    } else {
        if (attempts >= maxRetries) {
            log_warning("%s reached its retry limit.\n", host.c_str());
            enter(ConnectionState::Disconnected);
        } else {
            enter(ConnectionState::Retry);
        }
    }
}

void StickySocket::disconnect() { enter(ConnectionState::Disconnected); }

void StickySocket::enter(ConnectionState newState)
{
    if (status == newState) {
        log_debug("ignored same state transition, %s\n", getStatus().c_str());
        return;
    }

    const bool previous = online;
    switch (newState) {
    case ConnectionState::Disconnected:
        online = false;
        close();
        break;
    case ConnectionState::Retry:
        online = false;
        close();
        backOff = (1 << (attempts + 1)) * BACKOFF_MULTIPLIER;
        break;
    case ConnectionState::Connecting:
        online = false;
        break;
    case ConnectionState::Connected:
        online = true;
        break;
    case ConnectionState::None:
        break;
    }

    status = newState;

    if (online != previous) {
        if (online) {
            wentOnline();
        } else {
            wentOffline();
        }
    }
}

void StickySocket::handlePollError()
{
    if (status == ConnectionState::Connecting) {
        if (attempts >= maxRetries) {
            log_warning("%s reached its retry limit.\n", host.c_str());
            enter(ConnectionState::Disconnected);
        } else {
            enter(ConnectionState::Retry);
            log_debug("%s from Connecting to %s\n", host.c_str(), getStatus().c_str());
        }
    } else if (status == ConnectionState::Retry) {
        if (backOff) {
            backOff--;
        } else {
            reconnect();
        }
    }
}

void StickySocket::handleDataReceive()
{
    if (status == ConnectionState::Connected) {
        auto data = receive();
        if (data.empty()) {
            attempts = 0;
            enter(ConnectionState::Retry);
            log_debug("%s from Connected to %s\n", host.c_str(), getStatus().c_str());
        } else {
            didReceived(data);
        }
    }
}

void StickySocket::handlePollOut()
{
    if (status == ConnectionState::Connecting) {
        enter(ConnectionState::Connected);
    }
}

auto StickySocket::eval(const struct pollfd& response) -> ConnectionState
{
    if (status == ConnectionState::Disconnected) {
        log_error("%s should not evaluate poll for disconnected socket.\n", host.c_str());
        return ConnectionState::None;
    }

    const ConnectionState lastStatus { status };
    const int lastAttempts { attempts };

    if (response.revents & (POLLNVAL | POLLERR | POLLHUP)) {
        handlePollError();
    } else if (response.revents & (POLLIN | POLLPRI)) {
        handleDataReceive();
    } else if (response.revents & POLLOUT) {
        handlePollOut();
    }

    return (status != lastStatus || attempts != lastAttempts) ? status
                                                              : ConnectionState::None;
}

void StickySocket::send(const std::span<const std::byte> buffer)
{
    if (status != ConnectionState::Connected) {
        log_warning("%s cannot send anything, is not connected.\n", host.c_str());
        return;
    }

    // TODO: maybe switch to "sending?"
    const size_t bytes = ::send(descriptor, buffer.data(), buffer.size(), 0);
    if (bytes < 0) {
        log_error("Failed to send to %s.\n", host.c_str());
    } else {
        log_buffer("<<" + host + "<<", buffer);
    }
}

auto StickySocket::receive() -> std::span<std::byte>
{
    if (status != ConnectionState::Connected) {
        log_warning("%s cannot receive anything, is not connected.\n", host.c_str());
        return std::span<std::byte> {};
    }

    const size_t bytes = ::recv(descriptor, rxBuffer.data(), rxBuffer.size(), 0);
    if (bytes < 0) {
        log_error("%s receive failed.\n");
        return std::span<std::byte> {};
    }
    if (bytes == 0) {
        log_error("%s connection maybe lost.\n");
        return std::span<std::byte> {};
    }

    return std::span<std::byte> { rxBuffer.data(), bytes };
}

auto StickySocket::isAlive() const -> bool
{
    return status != ConnectionState::Disconnected;
}

auto StickySocket::getDescriptor() const -> int { return descriptor; }

auto StickySocket::getHost() const -> const std::string& { return host; }

auto StickySocket::getState() const -> ConnectionState { return status; }

auto StickySocket::getStatus() const -> const std::string&
{
    static std::string textual;
    switch (status) {
    case ConnectionState::None:
        textual = "None";
        break;
    case ConnectionState::Disconnected:
        textual = "Disconnected";
        break;
    case ConnectionState::Connecting:
        textual = "Connecting";
        break;
    case ConnectionState::Connected:
        textual = "Connected";
        break;
    case ConnectionState::Retry:
        textual = "Retry" + std::to_string(attempts);
        break;
    }
    return textual;
}

void StickySocket::wentOnline() { log_debug("%s is online.\n", host.c_str()); }

void StickySocket::wentOffline() { log_debug("%s is offline.\n", host.c_str()); }

void StickySocket::didReceived(const std::span<std::byte> buffer)
{
    log_buffer(">>" + host + ">>", buffer);
}
