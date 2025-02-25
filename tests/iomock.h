#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/poll.h>

#include "ioi.h"

class IoMockAdapter : public IoIntf
{
  public:
    MOCK_METHOD(int, inet_pton, (int, const char*, void*), (const, override));
    MOCK_METHOD(int, socket, (int, int, int), (const, override));
    MOCK_METHOD(int, connect, (int, const struct sockaddr*, socklen_t), (const, override));
    MOCK_METHOD(int, close, (int), (const, override));
    MOCK_METHOD(size_t, send, (int, const void*, size_t, int), (const, override));
    MOCK_METHOD(size_t, recv, (int, void*, size_t, int), (const, override));
    MOCK_METHOD(int, getsockopt, (int, int, int, void*, socklen_t*), (const, override));
    MOCK_METHOD(int, poll, (struct pollfd*, nfds_t, int), (const, override));
};
