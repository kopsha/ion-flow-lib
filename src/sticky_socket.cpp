#include "helpers.h"
#include "logs.h"
#include "sticky_socket.h"

#include <cstring>
#include <stdexcept>
#include <string>

#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>

constexpr int INVALID_SOCKET = -1;

StickySocket::StickySocket(const std::string& host, int port, const int retries)
    : descriptor { INVALID_SOCKET }
    , host { host }
    , port { std::to_string(port) }
    , maxRetries(retries)
    , attempts(0)
    , backOff(0)
    , status(ConnectionState::Disconnected)
    , online(false)
{
    std::memset(rxBuffer, 0, sizeof(rxBuffer));
    open();
}

StickySocket::~StickySocket() { disconnect(); }

bool StickySocket::open()
{
    if (descriptor != INVALID_SOCKET) {
        /*log_debug("open discarded, socket already connecting %s\n", host.c_str());*/
        return true;
    }

    struct addrinfo hints;
    struct addrinfo* results = nullptr;
    std::memset(&hints, 0, sizeof(hints));

    // Request address info
    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = SOCK_STREAM; // TCP
    int rc = getaddrinfo(host.c_str(), port.c_str(), &hints, &results);
    if (rc != 0) {
        log_error("Cannot resolve %s, reason: %s", host.c_str(), gai_strerror(rc));
        descriptor = INVALID_SOCKET;
        return false;
    }

    // Find the first valid socket with requested addr info
    for (const auto* result = results; result; result = result->ai_next) {
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
        rc = ::connect(descriptor, result->ai_addr, result->ai_addrlen);
        if (rc < 0) {
            if (errno == EINPROGRESS) {
                // Connection is in progress, which is expected for non-blocking sockets
                break;
            } else {
                // Other connection error
                log_error("Connection failed: %s", std::strerror(errno));
                ::close(descriptor);
                descriptor = INVALID_SOCKET;
                continue; // Try the next address
            }
        } else if (rc == 0) {
            // Connection succeeded immediately (unlikely for non-blocking sockets)
            break;
        }
    }

    freeaddrinfo(results);

    /*log_debug("open returns %d\n", descriptor != INVALID_SOCKET);*/
    return descriptor != INVALID_SOCKET;
}

void StickySocket::close()
{
    ::close(descriptor);
    descriptor = INVALID_SOCKET;
}

void StickySocket::connect()
{
    /*log_debug("connect call %s\n", host.c_str());*/
    if (status != ConnectionState::Disconnected) {
        log_warning("%s connection is already in progress.\n", host.c_str());
        return;
    }
    attempts = 0;
    reconnect();
}

void StickySocket::reconnect()
{
    attempts++;
    if (open()) {
        enter(ConnectionState::Connecting);
        /*log_debug("%s is %s\n", host.c_str(), getStatus().c_str());*/
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

    bool previous = online;
    switch (newState) {
    case ConnectionState::Disconnected:
        online = false;
        close();
        break;
    case ConnectionState::Retry:
        online = false;
        close();
        backOff = (1 << (attempts + 1)) * 100;
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

StickySocket::ConnectionState StickySocket::eval(const struct pollfd& response)
{
    if (status == ConnectionState::Disconnected) {
        log_error("%s should not evaluate poll for disconnected socket.\n", host.c_str());
        return ConnectionState::None;
    }

    ConnectionState lastStatus = status;
    int lastAttempts = attempts;

    if (response.revents & (POLLNVAL | POLLERR | POLLHUP)) {
        if (status == ConnectionState::Connecting) {
            if (attempts >= maxRetries) {
                log_warning("%s reached its retry limit.\n", host.c_str());
                enter(ConnectionState::Disconnected);
            } else {
                enter(ConnectionState::Retry);
            }
        } else if (status == ConnectionState::Retry) {
            if (backOff) {
                backOff--;
            } else {
                reconnect();
            }
        }
    } else if (response.revents & (POLLIN | POLLPRI)) {
        if (status == ConnectionState::Connected) {
            auto data = receive();
            if (data.size() == 0) {
                attempts = 0;
                enter(ConnectionState::Retry);
            } else {
                didReceived(data);
            }
        }
    } else if (response.revents & POLLOUT) {
        if (status == ConnectionState::Connecting) {
            enter(ConnectionState::Connected);
        }
    } else {
        // nothing happened...
    }
    if (status != lastStatus || attempts != lastAttempts) {
        return status;
    }
    return ConnectionState::None;
}

void StickySocket::send(const std::span<const byte> buffer)
{
    if (status != ConnectionState::Connected) {
        log_warning("%s cannot send anything, is not connected.\n", host.c_str());
        return;
    }

    // TODO: maybe switch to "sending?"
    size_t bytes = ::send(descriptor, buffer.data(), buffer.size(), 0);
    if (bytes < 0) {
        log_error("Failed to send to %s.\n", host.c_str());
    }
}

std::span<byte> StickySocket::receive()
{
    if (status != ConnectionState::Connected) {
        log_warning("%s cannot receive anything, is not connected.\n", host.c_str());
        return std::span<byte>();
    }

    size_t bytes = ::recv(descriptor, rxBuffer, BUFFER_SIZE, 0);
    if (bytes < 0) {
        log_error("%s receive failed.\n");
        return std::span<byte>();
    } else if (bytes == 0) {
        log_error("%s connection maybe lost.\n");
        return std::span<byte>();
    }

    return std::span<byte>(rxBuffer, bytes);
}

bool StickySocket::isAlive() const { return status != ConnectionState::Disconnected; }

int StickySocket::getDescriptor() const { return descriptor; }

const std::string& StickySocket::getHost() const { return host; }

const std::string& StickySocket::getStatus() const
{
    static std::string textual;
    switch (status) {
    case ConnectionState::Disconnected:
        textual = "Disconnected";
        break;
    case ConnectionState::Connecting:
        textual = "Connecting";
        break;
    case ConnectionState::Retry:
        textual = "Retry" + std::to_string(attempts);
        break;
    case ConnectionState::Connected:
        textual = "Connected";
        break;
    case ConnectionState::None:
        textual = "None";
        break;
    }
    return textual;
}

void StickySocket::wentOnline() { log_debug("%s is online.\n", host.c_str()); }

void StickySocket::wentOffline() { log_debug("%s is offline.\n", host.c_str()); }

void StickySocket::didReceived(const std::span<byte> buffer) { log_buffer(host, buffer); }
