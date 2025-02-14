#include "ion_service.h"
#include <chrono>
#include <gtest/gtest.h>
#include <memory>
#include <thread>

static constexpr int A_PORT = 9999;

TEST(ServiceUnit, new_instance_is_healthy)
{
    IonService svc;
    EXPECT_TRUE(svc.isHealthy());
    EXPECT_FALSE(svc.isRunning());
    EXPECT_EQ(svc.connectionsCount(), 0);
}

TEST(ServiceUnit, resetHealth_makes_instance_not_healthy)
{
    IonService svc;
    svc.resetHealth();
    EXPECT_FALSE(svc.isHealthy());
}

TEST(ServiceUnit, start_puts_service_in_running)
{
    IonService svc;

    svc.start();

    EXPECT_TRUE(svc.isRunning());
    EXPECT_TRUE(svc.isHealthy());
}

TEST(ServiceUnit, double_start_is_tolerated)
{
    IonService svc;

    svc.start();
    svc.start();

    EXPECT_TRUE(svc.isRunning());
    EXPECT_TRUE(svc.isHealthy());
}

TEST(ServiceUnit, stop_does_nothing_on_new_instance)
{
    IonService svc;

    svc.stop();

    EXPECT_FALSE(svc.isRunning());
    EXPECT_TRUE(svc.isHealthy());
}

TEST(ServiceUnit, stop_puts_instance_in_not_running)
{
    IonService svc;
    svc.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(3));

    svc.stop();

    EXPECT_FALSE(svc.isRunning());
    EXPECT_TRUE(svc.isHealthy());
}

TEST(ServiceUnit, double_stop_is_tolerated)
{
    IonService svc;
    svc.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(3));

    svc.stop();
    svc.stop();

    EXPECT_FALSE(svc.isRunning());
    EXPECT_TRUE(svc.isHealthy());
}

TEST(ServiceUnit, can_attach_sockets)
{
    IonService svc;

    std::unique_ptr<StickySocket> sock1
        = std::make_unique<StickySocket>("127.0.0.1", A_PORT);
    int key = svc.attach(std::move(sock1));

    EXPECT_GT(key, 0);
    EXPECT_EQ(svc.connectionsCount(), 1);
}

TEST(ServiceUnit, cannot_attach_invalid_sockets)
{
    IonService svc;

    std::unique_ptr<StickySocket> sock1
        = std::make_unique<StickySocket>("127.0.0.1", A_PORT);
    sock1->connect();
    sock1->disconnect();

    int key = svc.attach(std::move(sock1));

    EXPECT_EQ(key, StickySocket::INVALID_SOCKET);
    EXPECT_EQ(svc.connectionsCount(), 0);
}

TEST(ServiceUnit, can_dettach_sockets_once)
{
    IonService svc;

    std::unique_ptr<StickySocket> sock1
        = std::make_unique<StickySocket>("127.0.0.1", A_PORT);
    int key = svc.attach(std::move(sock1));
    EXPECT_GT(key, 0);

    std::unique_ptr<StickySocket> sockAlt = svc.detach(key);
    EXPECT_NE(sockAlt, nullptr);
    EXPECT_EQ(svc.connectionsCount(), 0);

    std::unique_ptr<StickySocket> sockNull = svc.detach(key);
    EXPECT_EQ(sockNull, nullptr);
    EXPECT_EQ(svc.connectionsCount(), 0);
}
