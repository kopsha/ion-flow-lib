#include "ionflow.h"

#include <atomic>
#include <chrono>
#include <csignal>
#include <exception>
#include <memory>
#include <thread>
#include <utility>

namespace {

constexpr int SUPERVISOR_CYCLE = 233;
constexpr int SICP_PORT = 5000;

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
    log_info("Starting service and supervisor threads...");
    IonService flow;

    // enter supervisor context
    auto prevIntH = std::signal(SIGINT, signalHandler);
    auto prevTermH = std::signal(SIGTERM, signalHandler);
    std::thread runner(supervisor, std::ref(flow));

    try {
        flow.start(); // as a separate thread
        std::unique_ptr<IonSession> logic = std::make_unique<IonSession>("localhost", SICP_PORT);
        flow.attach(std::move(logic));

        while (!pleaseStop && flow.isRunning()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(SUPERVISOR_CYCLE));
        }

        if (flow.isRunning()) {
            flow.stop();
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
