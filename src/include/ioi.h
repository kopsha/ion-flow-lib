#pragma once

#include <sys/poll.h>
#include <sys/socket.h>

class IoIntf
{
  public:
    IoIntf() = default;
    IoIntf(IoIntf&&) = default;
    virtual ~IoIntf() = default;
    auto operator=(IoIntf&&) -> IoIntf& = default;

    IoIntf(const IoIntf&) = delete;
    auto operator=(const IoIntf&) -> IoIntf& = delete;

    virtual auto inet_pton(int family, const char* address, void* buf) const -> int = 0;
    virtual auto socket(int domain, int type, int protocol) const -> int = 0;
    virtual auto connect(int sockfd, const struct sockaddr* addr, socklen_t len) const
        -> int = 0;
    virtual auto close(int sockfd) const -> int = 0;
    virtual auto send(int sockfd, const void* buf, size_t len, int flags) const
        -> size_t = 0;
    virtual auto recv(int sockfd, void* buf, size_t len, int flags) const -> size_t = 0;
    virtual auto
    getsockopt(int sockfd, int level, int optname, void* optval, socklen_t* optlen) const
        -> int = 0;
    virtual auto poll(struct pollfd* fds, nfds_t count, int timeout) const -> int = 0;
};
