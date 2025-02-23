#pragma once

#include "ioi.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>

class IoAdapter : public IoIntf
{
  public:
    auto inet_pton(int family, const char* address, void* buf) const -> int override
    {
        return ::inet_pton(family, address, buf);
    };

    [[nodiscard]] auto socket(int domain, int type, int protocol) const -> int override
    {
        return ::socket(domain, type, protocol);
    };

    auto
    connect(int sockfd, const struct sockaddr* addr, socklen_t len) const -> int override
    {
        return ::connect(sockfd, addr, len);
    };

    auto close(int sockfd) const -> int override { return ::close(sockfd); };

    auto send(int sockfd, const void* buf, size_t len, int flags) const -> size_t override
    {
        return ::send(sockfd, buf, len, flags);
    };

    auto recv(int sockfd, void* buf, size_t len, int flags) const -> size_t override
    {
        return ::recv(sockfd, buf, len, flags);
    };

    auto getsockopt(int sockfd, int level, int optname, void* optval, socklen_t* optlen)
        const -> int override
    {
        return ::getsockopt(sockfd, level, optname, optval, optlen);
    };

    [[nodiscard]] auto
    poll(struct pollfd* fds, nfds_t count, int timeout) const -> int override
    {
        return ::poll(fds, count, timeout);
    };
};
