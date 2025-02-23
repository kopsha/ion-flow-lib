#include "console.h"
#include "easy_socket.h"
#include "ioi.h"
#include "ipv4_socket.h"

#include <arpa/inet.h>
#include <cstdint>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>

IPv4Socket::IPv4Socket(const IoIntf& ioRef, std::string host, uint16_t port)
    : EasySocketIntf(std::move(host), port)
    , io(ioRef)
    , rxBuffer()
{
    rxBuffer.fill(0);
}

IPv4Socket::IPv4Socket(IPv4Socket&& other) noexcept
    : EasySocketIntf(std::move(other.host), other.port)
    , io(other.io)
    , rxBuffer(other.rxBuffer)
{
}

IPv4Socket::~IPv4Socket()
{
    if (descriptor != INVALID_SOCKET)
    {
        disconnect();
    }
}

auto IPv4Socket::enter(const ConnectionState newState) -> bool
{
    if (state == newState)
    {
        return false;
    }

    state = newState;
    if (state == ConnectionState::Connected)
    {
        wentOnline();
    }
    else if (state == ConnectionState::Disconnected)
    {
        wentOffline();
    }

    return true;
}

auto IPv4Socket::eval(const struct pollfd& response) -> bool
{
    if (state == ConnectionState::Disconnected)
    {
        console::error("disconnected socket cannot evaluate events.");
        return false;
    }

    const ConnectionState last { state };

    if (response.revents & (POLLNVAL | POLLERR | POLLHUP))
    {
        enter(ConnectionState::Disconnected);
    }
    else if (response.revents & (POLLIN | POLLPRI))
    {
        canReceive();
    }
    else if (response.revents & POLLOUT)
    {
        canSend();
    }

    return (state != last);
}

void IPv4Socket::canReceive()
{
    if (state == ConnectionState::Connected)
    {
        auto data = receive();
        if (data.empty())
        {
            enter(ConnectionState::Disconnected);
        }
        else
        {
            didReceived(data);
        }
    }
}

void IPv4Socket::canSend()
{
    static int error = 0;
    static socklen_t len = sizeof(error);

    if (state == ConnectionState::Connecting)
    {
        // WARN: POLLOUT is not enough
        error = io.getsockopt(descriptor, SOL_SOCKET, SO_ERROR, &error, &len);
        if (error == 0)
        {
            enter(ConnectionState::Connected);
        }
        else
        {
            enter(ConnectionState::Disconnected);
        }
    }
}

auto IPv4Socket::connect() -> bool
{
    if (state != ConnectionState::Disconnected)
    {
        console::error("Socket is already connecting/connected to {}", host);
        return false;
    }

    // validate server IP address
    struct sockaddr_in serverAddr {
        .sin_family = AF_INET,
        .sin_port = htons(port),
        .sin_addr = { .s_addr = INADDR_ANY },
    };

    if (io.inet_pton(AF_INET, host.c_str(), &serverAddr.sin_addr) <= 0)
    {
        console::error("address {} is invalid or not supported.", host);
        return false;
    }

    // create non-blocking socket
    descriptor = io.socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (descriptor == INVALID_SOCKET)
    {
        console::error("Cannot create socket, reason: {}", strerror(errno));
        return false;
    }

    // open socket connection
    int err = io.connect(
        descriptor, reinterpret_cast<struct sockaddr*>(&serverAddr), sizeof(serverAddr)
    );

    if (err == -1 && errno != EINPROGRESS)
    {
        console::error("failed to connect to {} (code: {}).", host, errno);
        descriptor = INVALID_SOCKET;
        return false;
    }

    enter(ConnectionState::Connecting);
    return true;
}

void IPv4Socket::disconnect()
{
    if (descriptor != INVALID_SOCKET)
    {
        io.close(descriptor);
        descriptor = INVALID_SOCKET;
        console::debug("(closed)");
    }

    enter(ConnectionState::Disconnected);
}

auto IPv4Socket::send(std::span<const uint8_t> buffer) -> int
{
    return io.send(descriptor, buffer.data(), buffer.size(), 0);
}

auto IPv4Socket::receive() -> std::span<const uint8_t>
{
    size_t bytes = io.recv(descriptor, rxBuffer.data(), rxBuffer.size(), 0);
    if (bytes <= 0)
    {
        return {};
    }
    return { rxBuffer.data(), bytes };
}

void IPv4Socket::didReceived(std::span<const uint8_t> data)
{
    console::buffer(">>> " + host, data);
}

void IPv4Socket::wentOnline() { CONSOLE_TRACE(host); }

void IPv4Socket::wentOffline() { CONSOLE_TRACE(host); }
