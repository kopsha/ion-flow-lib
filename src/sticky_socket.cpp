
#include "sticky_socket.h"

#include <arpa/inet.h>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <netdb.h>
#include <stdexcept>
#include <sys/poll.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

constexpr int INVALID_SOCKET = -1;

void setSocketNonBlocking(int socket_fd)
{
    // WARNING: this may fail, check if flags are positive someday
    int flags = fcntl(socket_fd, F_GETFL, 0);
    fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK);
}

std::string sockaddr_to_string(const struct sockaddr* addr)
{
    char ip_str[INET6_ADDRSTRLEN];

    if (addr->sa_family == AF_INET) {
        // IPv4 address
        struct sockaddr_in* ipv4 = (struct sockaddr_in*)addr;
        inet_ntop(AF_INET, &(ipv4->sin_addr), ip_str, sizeof(ip_str));
        return std::string(ip_str);
    } else if (addr->sa_family == AF_INET6) {
        // IPv6 address
        struct sockaddr_in6* ipv6 = (struct sockaddr_in6*)addr;
        inet_ntop(AF_INET6, &(ipv6->sin6_addr), ip_str, sizeof(ip_str));
        return std::string(ip_str);
    } else {
        return "Unknown address family";
    }
}

void print_addrinfo(const struct addrinfo* info)
{
    while (info != nullptr) {
        std::cout << "Flags: " << info->ai_flags << "\n";
        std::cout << "Family: " << info->ai_family << " ("
                  << (info->ai_family == AF_INET           ? "IPv4"
                             : info->ai_family == AF_INET6 ? "IPv6"
                                                           : "Other")
                  << ")\n";
        std::cout << "Socket Type: " << info->ai_socktype << "\n";
        std::cout << "Protocol: " << info->ai_protocol << "\n";
        std::cout << "Address Length: " << info->ai_addrlen << "\n";

        if (info->ai_addr != nullptr) {
            std::cout << "Address: " << sockaddr_to_string(info->ai_addr) << "\n";
        } else {
            std::cout << "Address: (null)\n";
        }

        if (info->ai_canonname != nullptr) {
            std::cout << "Canonical Name: " << info->ai_canonname << "\n";
        } else {
            std::cout << "Canonical Name: (null)\n";
        }

        std::cout << "----------------------------------\n";

        info = info->ai_next;
    }
}

int socket_from_results(const struct addrinfo* results) { return INVALID_SOCKET; }

StickySocket::StickySocket(
    const std::string& name, const std::string& host, int port, const int retries)
    : descriptor { INVALID_SOCKET }
    , name { name }
    , host { host }
    , port { std::to_string(port) }
    , maxRetries(retries)
    , retry(retries)
    , status(ConnectionState::Disconnected)
{
    std::memset(rxBuffer.data(), 0, sizeof(rxBuffer));

    // Prepare addres hints
    struct addrinfo hints;
    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; // IPv4 only for simplicity
    hints.ai_socktype = SOCK_STREAM;

    // Retrieve possible addresses
    struct addrinfo* results = nullptr;
    int rc = getaddrinfo(host.c_str(), this->port.c_str(), &hints, &results);
    if (rc != 0) {
        throw std::runtime_error(
            name + " cannot resolve " + host + ", reason " + gai_strerror(rc));
    }
    for (const auto* result = results; result; result = result->ai_next) {
        descriptor = ::socket(result->ai_family, result->ai_socktype, result->ai_protocol);
        if (descriptor != INVALID_SOCKET) {
            setSocketNonBlocking(descriptor);
            useConnection.family = result->ai_family;
            useConnection.socktype = result->ai_socktype;
            useConnection.protocol = result->ai_protocol;
            std::memcpy(&useConnection.addr, result->ai_addr, result->ai_addrlen);
            useConnection.addrlen = result->ai_addrlen;
            break;
        }
    }
    freeaddrinfo(results);

    if (descriptor == INVALID_SOCKET) {
        throw std::runtime_error("Failed to create socket");
    }
}

void StickySocket::open()
{
    printf("status: %d\n", status);
    if (status != ConnectionState::Disconnected) {
        std::cerr << "Hold your horses, we're getting there." << std::endl;
        return;
    }

    status = ConnectionState::Connecting;
    int rc = ::connect(descriptor,
        reinterpret_cast<const struct sockaddr*>(&useConnection.addr),
        useConnection.addrlen);

    if (rc < 0) {
        if (errno != EWOULDBLOCK && errno != EINPROGRESS) {
            ::close(descriptor);
            descriptor = INVALID_SOCKET;
            throw std::runtime_error("");
        }
    }
}

const StickySocket::Event StickySocket::parse(const struct pollfd& response)
{
    Event detected = Event::None;
    if (response.revents & POLLNVAL) {
        detected = Event::Failed;
    } else if (response.revents & POLLIN) {
        detected = (status == ConnectionState::Connected) ? Event::Received : Event::None;
    } else if (response.revents & POLLOUT) {
        detected = Event::Connected;
        status = ConnectionState::Connected;
    } else if (response.revents & POLLERR) {
        detected = Event::Failed;
    } else if (response.revents & POLLHUP) {
        detected = Event::Failed;
    } else if (response.revents & POLLPRI) {
        detected = Event::Received;
    } else {
        detected = Event::None;
    }
    return detected;
}

void StickySocket::close()
{
    if (status == ConnectionState::Disconnected) {
        return;
    }
    if (descriptor != INVALID_SOCKET) {
        ::close(descriptor);
        descriptor = INVALID_SOCKET;
    }
}

StickySocket::~StickySocket() { close(); }

bool StickySocket::isConnected() const { return descriptor != INVALID_SOCKET; }
int StickySocket::getDescriptor() const { return descriptor; }
const std::string& StickySocket::getName() const { return host; }

void StickySocket::send(const std::span<byte> buffer)
{
    size_t bytes = ::send(descriptor, buffer.data(), buffer.size(), 0);
    if (bytes < 0) {
        throw std::runtime_error(name + " failed to send.");
    }
}

std::span<byte> StickySocket::receive()
{
    size_t bytes = ::recv(descriptor, rxBuffer.data(), rxBuffer.size(), 0);
    if (bytes < 0) {
        std::cerr << name << " cannot read from socket." << std::endl;
    } else if (bytes == 0) {
        std::cerr << name << " closed connection." << std::endl;
        close();
    } else {
    }
    return std::span<byte>(rxBuffer.data(), bytes);
}

int connect_with_timeout(
    int sockfd, const struct sockaddr* addr, socklen_t addrlen, unsigned int timeout_ms)
{
    int rc = 0;
    // Set O_NONBLOCK
    int sockfd_flags_before;
    if ((sockfd_flags_before = fcntl(sockfd, F_GETFL, 0) < 0))
        return -1;
    if (fcntl(sockfd, F_SETFL, sockfd_flags_before | O_NONBLOCK) < 0)
        return -1;
    // Start connecting (asynchronously)
    do {
        if (connect(sockfd, addr, addrlen) < 0) {
            // Did connect return an error? If so, we'll fail.
            if ((errno != EWOULDBLOCK) && (errno != EINPROGRESS)) {
                rc = -1;
            }
            // Otherwise, we'll wait for it to complete.
            else {
                // Set a deadline timestamp 'timeout' ms from now (needed b/c poll can be
                // interrupted)
                struct timespec now;
                if (clock_gettime(CLOCK_MONOTONIC, &now) < 0) {
                    rc = -1;
                    break;
                }
                struct timespec deadline = { .tv_sec = now.tv_sec,
                    .tv_nsec = now.tv_nsec + timeout_ms * 1000000l };
                // Wait for the connection to complete.
                do {
                    // Calculate how long until the deadline
                    if (clock_gettime(CLOCK_MONOTONIC, &now) < 0) {
                        rc = -1;
                        break;
                    }
                    int ms_until_deadline = (int)((deadline.tv_sec - now.tv_sec) * 1000l
                        + (deadline.tv_nsec - now.tv_nsec) / 1000000l);
                    if (ms_until_deadline < 0) {
                        rc = 0;
                        break;
                    }
                    // Wait for connect to complete (or for the timeout deadline)
                    struct pollfd pfds[]
                        = { { .fd = sockfd, .events = POLLIN | POLLPRI | POLLOUT } };
                    rc = poll(pfds, 1, ms_until_deadline);
                    // If poll 'succeeded', make sure it *really* succeeded
                    if (rc > 0) {
                        int error = 0;
                        socklen_t len = sizeof(error);
                        int retval
                            = getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &len);
                        if (retval == 0)
                            errno = error;
                        if (error != 0)
                            rc = -1;
                    }
                }
                // If poll was interrupted, try again.
                while (rc == -1 && errno == EINTR);
                // Did poll timeout? If so, fail.
                if (rc == 0) {
                    errno = ETIMEDOUT;
                    rc = -1;
                }
            }
        }
    } while (0);
    // Restore original O_NONBLOCK state
    if (fcntl(sockfd, F_SETFL, sockfd_flags_before) < 0)
        return -1;
    // Success
    return rc;
}
