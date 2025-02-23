#include "ionflow.h"

#include <chrono>
#include <csignal>
#include <stop_token>
#include <thread>

constexpr int SUPERVISOR_CYCLE = 233;
static std::stop_source super;

static void signalHandler(int signal)
{
    if (signal == SIGINT || signal == SIGTERM)
    {
        console::info("Stop signal received ({}).", signal);
        super.request_stop();
    }
}

static void supervise(IonService& service)
{
    console::info("Supervisor thread is running...");
    auto token = super.get_token();
    while (!token.stop_requested())
    {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        if (service.isRunning())
        {
            if (!service.isHealthy())
            {
                console::warning("Service appears unresponsive. Restarting...\n");
                service.stop();
                service.start(super.get_token());
            }
            else
            {
                service.resetHealth();
            }
        }
        else
        {
            service.start(super.get_token());
        }
    }
    service.stop();
    console::info("Supervisor thread gracefully ended.");
}

int main()
{

    auto prevIntH = std::signal(SIGINT, signalHandler);
    auto prevTermH = std::signal(SIGTERM, signalHandler);
    IoAdapter netw;
    IonService flow(netw);

    if (flow.setup() < 0)
    {
        console::error("service configuration failed.");
        return -1;
    }

    std::jthread runner([&flow]() { supervise(flow); });

    while (!super.stop_requested())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(SUPERVISOR_CYCLE));
    }

    (void)std::signal(SIGINT, prevIntH);
    (void)std::signal(SIGTERM, prevTermH);

    return 0;
}
