#include "easy_socket.h"
#include "iomock.h"
#include "ipv4_socket.h"

#include <cerrno>
#include <cstdint>
#include <cstring>

#include <asm-generic/socket.h>
#include <string_view>
#include <sys/poll.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

constexpr std::string A_HOST = "127.0.0.1";
constexpr std::string INVALID_HOST = "::1";
constexpr uint16_t ANY_PORT = 9999;

constexpr int GOOD_DESCRIPTOR = 3;
constexpr int BAD_DESCRIPTOR = EasySocketIntf::INVALID_SOCKET;
constexpr int GOOD_ADDRESS = 1;
constexpr int BAD_ADDRESS = 0;
constexpr int GOOD_SOCK_OPT = 0;
constexpr int BAD_SOCK_OPT = -1;
constexpr char TEST_DATA[] = "Hello"; // The data to simulate in the buffer
constexpr size_t TEST_DATA_SIZE = sizeof(TEST_DATA);

using ::testing::_;
using ::testing::DoAll;
using ::testing::Return;
using ::testing::SetErrnoAndReturn;
using ::testing::WithArg;

class IPv4SocketTest : public ::testing::Test
{
  protected:
    IoMockAdapter iomock;

    void SetUp() override { EXPECT_CALL(iomock, close(_)).WillRepeatedly(Return(0)); }

    void TearDown() override {}
};

TEST_F(IPv4SocketTest, new_socket_has_invalid_descriptor)
{
    IPv4Socket skt(iomock, A_HOST, ANY_PORT);

    EXPECT_EQ(skt.getHost(), A_HOST);
    EXPECT_EQ(skt.getDescriptor(), EasySocketIntf::INVALID_SOCKET);
    EXPECT_EQ(skt.getState(), EasySocketIntf::ConnectionState::Disconnected);
}

TEST_F(IPv4SocketTest, new_socket_with_invalid_host)
{
    IPv4Socket skt(iomock, std::string(), ANY_PORT);

    EXPECT_EQ(skt.getHost(), std::string());
    EXPECT_EQ(skt.getDescriptor(), EasySocketIntf::INVALID_SOCKET);
    EXPECT_EQ(skt.getState(), EasySocketIntf::ConnectionState::Disconnected);
}

TEST_F(IPv4SocketTest, can_move_instance)
{
    IPv4Socket left(iomock, A_HOST, ANY_PORT);

    IPv4Socket right(std::move(left));

    EXPECT_EQ(right.getHost(), A_HOST);
    EXPECT_EQ(right.getDescriptor(), EasySocketIntf::INVALID_SOCKET);
    EXPECT_EQ(right.getState(), EasySocketIntf::ConnectionState::Disconnected);
}

TEST_F(IPv4SocketTest, connect_moves_to_connecting)
{
    EXPECT_CALL(iomock, inet_pton(AF_INET, ::testing::StrEq(A_HOST.c_str()), _))
        .WillOnce(Return(GOOD_ADDRESS));
    EXPECT_CALL(iomock, socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0))
        .WillOnce(Return(GOOD_DESCRIPTOR));
    EXPECT_CALL(iomock, connect(3, _, sizeof(sockaddr_in)))
        .WillOnce(SetErrnoAndReturn(EINPROGRESS, -1));

    IPv4Socket skt(iomock, A_HOST, ANY_PORT);

    bool willConnect = skt.connect();

    EXPECT_TRUE(willConnect);
    EXPECT_EQ(skt.getState(), EasySocketIntf::ConnectionState::Connecting);
    EXPECT_EQ(skt.getHost(), A_HOST);
    EXPECT_EQ(skt.getDescriptor(), GOOD_DESCRIPTOR);
}

TEST_F(IPv4SocketTest, connect_twice_is_ignored)
{
    EXPECT_CALL(iomock, inet_pton(AF_INET, ::testing::StrEq(A_HOST.c_str()), _))
        .WillOnce(Return(GOOD_ADDRESS));
    EXPECT_CALL(iomock, socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0))
        .WillOnce(Return(GOOD_DESCRIPTOR));
    EXPECT_CALL(iomock, connect(GOOD_DESCRIPTOR, _, sizeof(sockaddr_in)))
        .WillOnce(SetErrnoAndReturn(EINPROGRESS, -1));

    IPv4Socket skt(iomock, A_HOST, ANY_PORT);

    bool willConnect = skt.connect();
    EXPECT_TRUE(willConnect);
    EXPECT_EQ(skt.getState(), EasySocketIntf::ConnectionState::Connecting);

    willConnect = skt.connect();
    EXPECT_FALSE(willConnect);
    EXPECT_EQ(skt.getDescriptor(), GOOD_DESCRIPTOR);
}

TEST_F(IPv4SocketTest, cannot_connect_to_invalid_host)
{
    EXPECT_CALL(iomock, inet_pton(AF_INET, _, _)).WillOnce(Return(BAD_ADDRESS));

    IPv4Socket skt(iomock, std::string(), ANY_PORT);

    bool willConnect = skt.connect();

    EXPECT_FALSE(willConnect);
    EXPECT_EQ(skt.getState(), EasySocketIntf::ConnectionState::Disconnected);
    EXPECT_EQ(skt.getDescriptor(), EasySocketIntf::INVALID_SOCKET);
}

TEST_F(IPv4SocketTest, cannot_connect_when_socket_creation_fails)
{
    EXPECT_CALL(iomock, inet_pton(AF_INET, _, _)).WillOnce(Return(GOOD_ADDRESS));
    EXPECT_CALL(iomock, socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0))
        .WillOnce(Return(BAD_DESCRIPTOR));

    IPv4Socket skt(iomock, A_HOST, ANY_PORT);

    bool willConnect = skt.connect();

    EXPECT_FALSE(willConnect);
    EXPECT_EQ(skt.getState(), EasySocketIntf::ConnectionState::Disconnected);
    EXPECT_EQ(skt.getDescriptor(), EasySocketIntf::INVALID_SOCKET);
}

TEST_F(IPv4SocketTest, cannot_connect_when_tcp_connect_fails)
{
    EXPECT_CALL(iomock, inet_pton(AF_INET, _, _)).WillOnce(Return(GOOD_ADDRESS));
    EXPECT_CALL(iomock, socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0))
        .WillOnce(Return(GOOD_DESCRIPTOR));
    EXPECT_CALL(iomock, connect(GOOD_DESCRIPTOR, _, sizeof(sockaddr_in)))
        .WillOnce(SetErrnoAndReturn(EHOSTUNREACH, -1));

    IPv4Socket skt(iomock, A_HOST, ANY_PORT);

    bool willConnect = skt.connect();

    EXPECT_FALSE(willConnect);
    EXPECT_EQ(skt.getState(), EasySocketIntf::ConnectionState::Disconnected);
    EXPECT_EQ(skt.getDescriptor(), EasySocketIntf::INVALID_SOCKET);
}

TEST_F(IPv4SocketTest, eval_POLLOUT_while_connecting_moves_to_connected)
{
    EXPECT_CALL(iomock, inet_pton(AF_INET, ::testing::StrEq(A_HOST.c_str()), _))
        .WillOnce(Return(GOOD_ADDRESS));
    EXPECT_CALL(iomock, socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0))
        .WillOnce(Return(GOOD_DESCRIPTOR));
    EXPECT_CALL(iomock, connect(GOOD_DESCRIPTOR, _, sizeof(sockaddr_in)))
        .WillOnce(SetErrnoAndReturn(EINPROGRESS, -1));
    EXPECT_CALL(iomock, getsockopt(GOOD_DESCRIPTOR, SOL_SOCKET, SO_ERROR, _, _))
        .WillOnce(Return(GOOD_SOCK_OPT));
    IPv4Socket skt(iomock, A_HOST, ANY_PORT);
    skt.connect();
    EXPECT_EQ(skt.getState(), EasySocketIntf::ConnectionState::Connecting);
    struct pollfd response { .revents = POLLOUT };

    skt.eval(response);

    EXPECT_EQ(skt.getState(), EasySocketIntf::ConnectionState::Connected);
}

TEST_F(IPv4SocketTest, eval_zero_while_connected_changes_nothing)
{
    EXPECT_CALL(iomock, inet_pton(AF_INET, ::testing::StrEq(A_HOST.c_str()), _))
        .WillOnce(Return(GOOD_ADDRESS));
    EXPECT_CALL(iomock, socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0))
        .WillOnce(Return(GOOD_DESCRIPTOR));
    EXPECT_CALL(iomock, connect(GOOD_DESCRIPTOR, _, sizeof(sockaddr_in)))
        .WillOnce(SetErrnoAndReturn(EINPROGRESS, -1));
    EXPECT_CALL(iomock, getsockopt(GOOD_DESCRIPTOR, SOL_SOCKET, SO_ERROR, _, _))
        .WillOnce(Return(GOOD_SOCK_OPT));

    IPv4Socket skt(iomock, A_HOST, ANY_PORT);
    skt.connect();
    EXPECT_EQ(skt.getState(), EasySocketIntf::ConnectionState::Connecting);
    struct pollfd canSendResponse { .revents = POLLOUT };
    skt.eval(canSendResponse);
    EXPECT_EQ(skt.getState(), EasySocketIntf::ConnectionState::Connected);

    struct pollfd nothing { .revents = 0 };
    skt.eval(nothing);
    EXPECT_EQ(skt.getState(), EasySocketIntf::ConnectionState::Connected);
}

TEST_F(IPv4SocketTest, eval_connecting_without_access_moves_to_disconnected)
{
    EXPECT_CALL(iomock, inet_pton(AF_INET, ::testing::StrEq(A_HOST.c_str()), _))
        .WillOnce(Return(GOOD_ADDRESS));
    EXPECT_CALL(iomock, socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0))
        .WillOnce(Return(GOOD_DESCRIPTOR));
    EXPECT_CALL(iomock, connect(GOOD_DESCRIPTOR, _, sizeof(sockaddr_in)))
        .WillOnce(SetErrnoAndReturn(EINPROGRESS, -1));
    EXPECT_CALL(iomock, getsockopt(GOOD_DESCRIPTOR, SOL_SOCKET, SO_ERROR, _, _))
        .WillOnce(SetErrnoAndReturn(EACCES, BAD_SOCK_OPT));

    IPv4Socket skt(iomock, A_HOST, ANY_PORT);
    skt.connect();
    EXPECT_EQ(skt.getState(), EasySocketIntf::ConnectionState::Connecting);
    struct pollfd canSendResponse { .revents = POLLOUT };

    skt.eval(canSendResponse);

    EXPECT_EQ(skt.getState(), EasySocketIntf::ConnectionState::Disconnected);
}

TEST_F(IPv4SocketTest, eval_on_disconnected_socket_is_tolerated)
{
    IPv4Socket skt(iomock, A_HOST, ANY_PORT);
    EXPECT_EQ(skt.getState(), EasySocketIntf::ConnectionState::Disconnected);
    struct pollfd canSendResponse { .revents = POLLOUT };

    skt.eval(canSendResponse);

    EXPECT_EQ(skt.getState(), EasySocketIntf::ConnectionState::Disconnected);
}

TEST_F(IPv4SocketTest, eval_POLLERR_moves_to_disconnected)
{
    EXPECT_CALL(iomock, inet_pton(AF_INET, ::testing::StrEq(A_HOST.c_str()), _))
        .WillOnce(Return(GOOD_ADDRESS));
    EXPECT_CALL(iomock, socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0))
        .WillOnce(Return(GOOD_DESCRIPTOR));
    EXPECT_CALL(iomock, connect(GOOD_DESCRIPTOR, _, sizeof(sockaddr_in)))
        .WillOnce(SetErrnoAndReturn(EINPROGRESS, -1));
    EXPECT_CALL(iomock, getsockopt(GOOD_DESCRIPTOR, SOL_SOCKET, SO_ERROR, _, _))
        .WillOnce(Return(GOOD_SOCK_OPT));
    IPv4Socket skt(iomock, A_HOST, ANY_PORT);
    skt.connect();
    EXPECT_EQ(skt.getState(), EasySocketIntf::ConnectionState::Connecting);
    struct pollfd canSendResponse { .revents = POLLOUT };
    skt.eval(canSendResponse);
    EXPECT_EQ(skt.getState(), EasySocketIntf::ConnectionState::Connected);

    struct pollfd errResponse { .revents = POLLOUT | POLLERR };
    skt.eval(errResponse);

    EXPECT_EQ(skt.getState(), EasySocketIntf::ConnectionState::Disconnected);
}

TEST_F(IPv4SocketTest, eval_POLLIN_while_connected_triggers_receive)
{
    class TestSocket : public IPv4Socket
    {
      public:
        std::vector<uint8_t> receivedCopy;

        TestSocket(const IoIntf& useIo, std::string host, uint16_t port)
            : IPv4Socket(useIo, std::move(host), port)
        {
        }

        void didReceived(std::span<const uint8_t> data) override
        {
            receivedCopy.assign(data.begin(), data.end());
        }
    };

    EXPECT_CALL(iomock, inet_pton(AF_INET, ::testing::StrEq(A_HOST.c_str()), _))
        .WillOnce(Return(GOOD_ADDRESS));
    EXPECT_CALL(iomock, socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0))
        .WillOnce(Return(GOOD_DESCRIPTOR));
    EXPECT_CALL(iomock, connect(GOOD_DESCRIPTOR, _, sizeof(sockaddr_in)))
        .WillOnce(SetErrnoAndReturn(EINPROGRESS, -1));
    EXPECT_CALL(iomock, getsockopt(GOOD_DESCRIPTOR, SOL_SOCKET, SO_ERROR, _, _))
        .WillOnce(Return(GOOD_SOCK_OPT));

    // Mock recv to return 5 bytes and populate the buffer with "Hello"
    EXPECT_CALL(iomock, recv(GOOD_DESCRIPTOR, _, EasySocketIntf::BUFFER_SIZE, 0))
        .WillOnce(DoAll(
            WithArg<1>([&](void* buffer) {  // Capture the buffer argument
                std::memcpy(buffer, TEST_DATA, TEST_DATA_SIZE);
            }),
            Return(TEST_DATA_SIZE - 1)  // Return the number of bytes read (without null terminator)
        ));

    TestSocket skt(iomock, A_HOST, ANY_PORT);
    skt.connect();
    EXPECT_EQ(skt.getState(), EasySocketIntf::ConnectionState::Connecting);
    struct pollfd canSendResponse { .revents = POLLOUT };
    skt.eval(canSendResponse);
    EXPECT_EQ(skt.getState(), EasySocketIntf::ConnectionState::Connected);

    struct pollfd canReadResponse { .revents = POLLIN };
    skt.eval(canReadResponse);

    EXPECT_EQ(std::string(skt.receivedCopy.begin(), skt.receivedCopy.end()), "Hello");
}

TEST_F(IPv4SocketTest, receive_zero_bytes_moves_to_disconnected)
{
    EXPECT_CALL(iomock, inet_pton(AF_INET, ::testing::StrEq(A_HOST.c_str()), _))
        .WillOnce(Return(GOOD_ADDRESS));
    EXPECT_CALL(iomock, socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0))
        .WillOnce(Return(GOOD_DESCRIPTOR));
    EXPECT_CALL(iomock, connect(GOOD_DESCRIPTOR, _, sizeof(sockaddr_in)))
        .WillOnce(SetErrnoAndReturn(EINPROGRESS, -1));
    EXPECT_CALL(iomock, getsockopt(GOOD_DESCRIPTOR, SOL_SOCKET, SO_ERROR, _, _))
        .WillOnce(Return(GOOD_SOCK_OPT));

    EXPECT_CALL(iomock, recv(GOOD_DESCRIPTOR, _, EasySocketIntf::BUFFER_SIZE, 0))
        .WillOnce(Return(0)); // read 0 bytes

    IPv4Socket skt(iomock, A_HOST, ANY_PORT);
    skt.connect();
    EXPECT_EQ(skt.getState(), EasySocketIntf::ConnectionState::Connecting);
    struct pollfd canSendResponse { .revents = POLLOUT };
    skt.eval(canSendResponse);
    EXPECT_EQ(skt.getState(), EasySocketIntf::ConnectionState::Connected);

    struct pollfd canReadResponse { .revents = POLLIN };
    skt.eval(canReadResponse);

    EXPECT_EQ(skt.getState(), EasySocketIntf::ConnectionState::Disconnected);
}

TEST_F(IPv4SocketTest, send_while_connected_does_send)
{
    EXPECT_CALL(iomock, inet_pton(AF_INET, ::testing::StrEq(A_HOST.c_str()), _))
        .WillOnce(Return(GOOD_ADDRESS));
    EXPECT_CALL(iomock, socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0))
        .WillOnce(Return(GOOD_DESCRIPTOR));
    EXPECT_CALL(iomock, connect(GOOD_DESCRIPTOR, _, sizeof(sockaddr_in)))
        .WillOnce(SetErrnoAndReturn(EINPROGRESS, -1));
    EXPECT_CALL(iomock, getsockopt(GOOD_DESCRIPTOR, SOL_SOCKET, SO_ERROR, _, _))
        .WillOnce(Return(GOOD_SOCK_OPT));

    // Capture the sent buffer and verify its content
    EXPECT_CALL(iomock, send(GOOD_DESCRIPTOR, _, TEST_DATA_SIZE, 0))
        .WillOnce(DoAll(
            WithArg<1>(
                [&](const void* buffer)
    {
        const char* sentData = static_cast<const char*>(buffer);
        ASSERT_EQ(std::string_view(sentData), std::string_view(TEST_DATA));
    }
            ),
            Return(TEST_DATA_SIZE)
        ));

    IPv4Socket skt(iomock, A_HOST, ANY_PORT);
    skt.connect();
    EXPECT_EQ(skt.getState(), EasySocketIntf::ConnectionState::Connecting);
    struct pollfd canSendResponse { .revents = POLLOUT };
    skt.eval(canSendResponse);
    EXPECT_EQ(skt.getState(), EasySocketIntf::ConnectionState::Connected);

    std::span<const uint8_t> textBuf(
        reinterpret_cast<const unsigned char*>(TEST_DATA), TEST_DATA_SIZE
    );
    skt.send(textBuf);
}
