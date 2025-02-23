#include "ionflow.h"

#include <atomic>
#include <chrono>
#include <csignal>
#include <exception>
#include <thread>

constexpr int SUPERVISOR_CYCLE = 233;
static std::atomic<bool> pleaseStop { false };

static void signalHandler(int signal)
{
    if (signal == SIGINT || signal == SIGTERM)
    {
        console::info("Stop signal received ({}).\n", signal);
        pleaseStop = true;
    }
}

static void supervisor(IonService& service)
{
    while (!pleaseStop)
    {
        std::this_thread::sleep_for(std::chrono::seconds(3));
        if (service.isRunning())
        {
            if (!service.isHealthy())
            {
                console::warning("Service appears unresponsive. Restarting...\n");
                service.stop();
                service.start();
            }
            else
            {
                service.resetHealth();
            }
        }
    }
}

auto main() -> int
{
    auto prevIntH = std::signal(SIGINT, signalHandler);
    auto prevTermH = std::signal(SIGTERM, signalHandler);

    IoAdapter netw;
    IonService flow(netw);

    console::info("Enter supervised context.");
    std::thread runner(supervisor, std::ref(flow));
    try
    {
        flow.start(); // as a separate thread
    }
    catch (const std::exception& err)
    {
        console::error(
            "FATALITY: Cannot start service thread: %s %s\n", typeid(err).name(),
            err.what()
        );
        pleaseStop = true;
        runner.join();
        return -1;
    }

    try
    {
        while (!pleaseStop && flow.isRunning())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(SUPERVISOR_CYCLE));
        }
    }
    catch (const std::exception& err)
    {
        console::error(
            "FATALITY: Something horrible happened: %s %s\n", typeid(err).name(),
            err.what()
        );
    }

    if (flow.isRunning())
    {
        flow.stop();
    }

    // exit supervisor context
    runner.join();
    (void)std::signal(SIGINT, prevIntH);
    (void)std::signal(SIGTERM, prevTermH);

    console::info("Service and monitor closed gracefully.\n");
    return 0;
}
