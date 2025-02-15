#include "sticky_socket.h"

#include <gtest/gtest.h>

#include <poll.h>
#include <sys/poll.h>

static constexpr int ANY_PORT = 9999;
static constexpr std::string A_HOST = "127.0.0.1";

TEST(StickySocket, new_instance__is_disconnected)
{
    StickySocket skt(A_HOST, ANY_PORT);

    EXPECT_EQ(skt.getState(), StickySocket::ConnectionState::Disconnected);
    EXPECT_FALSE(skt.isOnline());
    EXPECT_FALSE(skt.isAlive());
    EXPECT_EQ(skt.getHost(), A_HOST);
}

TEST(StickySocket, connect_valid_host__is_connecting)
{
    StickySocket skt(A_HOST, ANY_PORT);

    skt.connect();

    EXPECT_EQ(skt.getState(), StickySocket::ConnectionState::Connecting);
}

TEST(StickySocket, connect_invalid_host__is_retry)
{
    StickySocket skt("", ANY_PORT);

    skt.connect();

    EXPECT_EQ(skt.getState(), StickySocket::ConnectionState::Retry);
}

TEST(StickySocket, disconnect_while_connecting__is_disconnected)
{
    StickySocket skt(A_HOST, ANY_PORT);
    skt.connect();
    EXPECT_EQ(skt.getState(), StickySocket::ConnectionState::Connecting);

    skt.disconnect();

    EXPECT_EQ(skt.getState(), StickySocket::ConnectionState::Disconnected);
}

TEST(StickySocket, disconnect_can_be_called_twice)
{
    StickySocket skt(A_HOST, ANY_PORT);

    skt.disconnect();
    skt.disconnect();

    EXPECT_EQ(skt.getState(), StickySocket::ConnectionState::Disconnected);
}

TEST(StickySocket, eval_disconnected_socket_does_nothing)
{
    struct pollfd fake;
    StickySocket skt(A_HOST, ANY_PORT);

    auto newState = skt.eval(fake);

    EXPECT_EQ(newState, StickySocket::ConnectionState::None);
}

TEST(StickySocket, eval_no_events__is_connecting)
{
    struct pollfd fake;
    StickySocket skt(A_HOST, ANY_PORT);
    skt.connect();

    auto newState = skt.eval(fake);

    EXPECT_EQ(newState, StickySocket::ConnectionState::None);
    EXPECT_EQ(skt.getState(), StickySocket::ConnectionState::Connecting);
}

TEST(StickySocket, eval_write_while_connecting__is_connected)
{
    struct pollfd fake = { .revents = POLLOUT };
    StickySocket skt(A_HOST, ANY_PORT);
    skt.connect();

    auto newState = skt.eval(fake);

    EXPECT_EQ(newState, StickySocket::ConnectionState::Connected);
    EXPECT_EQ(skt.getState(), StickySocket::ConnectionState::Connected);
}

TEST(StickySocket, disconnect_while_connected__is_disconnected)
{
    struct pollfd fake = { .revents = POLLOUT };
    StickySocket skt(A_HOST, ANY_PORT);
    skt.connect();
    auto newState = skt.eval(fake);
    EXPECT_EQ(newState, StickySocket::ConnectionState::Connected);

    skt.disconnect();

    EXPECT_EQ(skt.getState(), StickySocket::ConnectionState::Disconnected);
}
