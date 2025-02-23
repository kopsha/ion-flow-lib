#include "console.h"
#include "ioi.h"
#include "ion_service.h"

#include <chrono>
#include <cstdio>
#include <cstring>
#include <exception>
#include <thread>

constexpr int EVENT_WINDOW = 55;
constexpr int CHILL_WINDOW = 8;
constexpr int MAX_CONSECUTIVE_FAILS = 5;

IonService::IonService(const IoIntf& useIo)
    : engine(useIo)
    , healthy(true)
{
}

IonService::~IonService() { stop(); }

void IonService::start(std::stop_token superToken)
{
    if (worker.joinable())
    {
        console::warning("service is already running, start ignored.\n");
        return;
    }
    worker = std::jthread([this, superToken]() { loop(superToken); });
}

int IonService::setup()
{
    CONSOLE_TRACE("pass config someday");
    return 0;
}

void IonService::stop()
{
    if (worker.joinable())
    {
        worker.request_stop();
        worker.join();
        console::info("Service stopped gracefully.");
    }
}

void IonService::onEntry()
{
    CONSOLE_TRACE("soon");

    auto& ref = engine.makeSocket("127.0.0.1", 5000);
    ref.connect();
}

void IonService::onExit() { CONSOLE_TRACE("just now"); }

void IonService::loop(std::stop_token token)
{
    CONSOLE_TRACE(token.stop_possible());

    console::info("Enter event loop.");
    onEntry();

    int failCount = 0;
    while (!token.stop_requested())
    {
        try
        {
            healthy = true;
            engine.poll(EVENT_WINDOW);

            if (failCount)
            {
                failCount--;
            }
        }
        catch (const std::exception& err)
        {
            failCount++;
            console::error(
                "Event loop failed {} times, reason: {}.", failCount, err.what()
            );
            if (failCount > MAX_CONSECUTIVE_FAILS)
            {
                console::error("Aborting due to {} repeated failures.", failCount);
                break;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(CHILL_WINDOW));
    }

    console::info("Exit event loop.");
    onExit();
}

void IonService::resetHealth() { healthy.store(false); }

auto IonService::isHealthy() const -> bool { return healthy; }

auto IonService::isRunning() const -> bool { return worker.joinable(); }
