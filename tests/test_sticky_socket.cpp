#include "iomock.h"
#include "sticky_socket.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>

constexpr int ANY_PORT = 9999;
constexpr std::string A_HOST = "localhost";

using ::testing::Return;

/*class SocketTest : public ::testing::Test*/
/*{*/
/*  protected:*/
/*    IoMockAdapter iom;*/
/*    std::unique_ptr<StickySocket> skt;*/
/**/
/*    void SetUp() override*/
/*    {*/
/*        skt = std::make_unique<StickySocket>(iom, A_HOST, ANY_PORT);*/
/*        // Set default return values*/
/*        ON_CALL(iom, create).WillByDefault(Return(1));*/
/*        ON_CALL(iom, setNonBlocking).WillByDefault(Return(true));*/
/*        ON_CALL(iom, connect).WillByDefault(Return(0));*/
/*        ON_CALL(iom, send).WillByDefault(Return(10));    // 10 bytes sent*/
/*        ON_CALL(iom, receive).WillByDefault(Return(20)); // 20 bytes received*/
/*    }*/
/**/
/*    void TearDown() override { skt.reset(); }*/
/*};*/

/*TEST_F(SocketTest, new_instance_is_disconnected)*/
/*{*/
/*    EXPECT_EQ(skt->getState(), StickySocket::ConnectionState::Disconnected);*/
/*    EXPECT_FALSE(true);*/
/*}*/

/*TEST(SocketTest, connect_moves_to_connecting_state)*/
/*{*/
/*    std::shared_ptr<ISocket> mockIo = std::make_shared<SocketMock>();*/
/*    StickySocket skt(mockIo, A_HOST, ANY_PORT);*/
/**/
/*    skt.connect();*/
/**/
/*    EXPECT_EQ(skt.getState(), StickySocket::ConnectionState::Connecting);*/
/*}*/

/*TEST(SocketTest, connect_while_connecting_state_is_ignored)*/
/*{*/
/*    std::shared_ptr<ISocket> mockIo = std::make_shared<SocketMock>();*/
/*    StickySocket skt(mockIo, A_HOST, ANY_PORT);*/
/**/
/*    skt.connect();*/
/*    EXPECT_EQ(skt.getState(), StickySocket::ConnectionState::Connecting);*/
/**/
/*    skt.connect();*/
/*    console::info("is {}", skt.getStatus());*/
/*    EXPECT_EQ(skt.getState(), StickySocket::ConnectionState::Connecting);*/
/*}*/
