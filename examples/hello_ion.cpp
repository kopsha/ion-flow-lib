#include <poll.h>

#include "ion_service.h"
#include "logs.h"

#include <atomic>
#include <chrono>
#include <csignal>
#include <exception>
#include <thread>

namespace {

constexpr int SUPERVISOR_CYCLE = 233;
std::atomic<bool> pleaseStop { false };

void signalHandler(int signal)
{
    if (signal == SIGINT || signal == SIGTERM) {
        log_info("Stop signal received (%d)\n", signal);
        pleaseStop = true;
    }
}

void supervisor(IonService& agent)
{
    while (agent.isRunning() && !pleaseStop) {
        std::this_thread::sleep_for(std::chrono::seconds(3));
        if (!agent.isHealthy()) {
            log_warning("Service appears unresponsive. Restarting...\n");
            agent.stop();
            agent.start();
        } else {
            agent.resetHealth();
        }
    }
}
}

auto main() -> int
{
    log_info("Starting service and supervisor threads...\n");
    IonService agent;

    // enter supervisor context
    auto prevIntH = std::signal(SIGINT, signalHandler);
    auto prevTermH = std::signal(SIGTERM, signalHandler);
    std::thread runner(supervisor, std::ref(agent));

    try {
        agent.start(); // as a separate thread

        while (!pleaseStop && agent.isRunning()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(SUPERVISOR_CYCLE));
        }

        if (agent.isRunning()) {
            agent.stop();
        }
    } catch (const std::exception& err) {
        log_error(
            "FATALITY: Something horrible happened: %s %s\n", typeid(err).name(),
            err.what()
        );
    }

    // exit supervisor context
    runner.join();
    (void)std::signal(SIGINT, prevIntH);
    (void)std::signal(SIGTERM, prevTermH);

    log_info("Service and monitor closed gracefully.\n");
    return 0;
}
