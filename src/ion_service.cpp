#include "console.h"
#include "ioi.h"
#include "ion_service.h"

#include <chrono>
#include <cstdio>
#include <cstring>
#include <thread>

constexpr int EVENT_WINDOW = 55;
constexpr int CHILL_WINDOW = 8;

IonService::IonService(const IoIntf& useIo)
    : engine(useIo)
    , healthy(true)
    , running(false)
{
}

IonService::~IonService() { stop(); }

void IonService::start()
{
    const bool alreadyRunning = running.exchange(true);
    if (alreadyRunning)
    {
        console::warning("service is already running, start ignored.\n");
        return;
    }
    worker = std::thread(&IonService::loop, this);
}

void IonService::stop()
{
    running = false;
    if (worker.joinable())
    {
        worker.join();
    }
}

void IonService::loop()
{

    auto& ref = engine.makeSocket("127.0.0.1", 5000);
    ref.connect();

    console::info("Event loop started...");
    while (running)
    {
        healthy = true;
        engine.poll(EVENT_WINDOW);
        std::this_thread::sleep_for(std::chrono::milliseconds(CHILL_WINDOW));
    }

    console::info("Closing event loop.");
}

void IonService::resetHealth() { healthy.store(false); }

auto IonService::isHealthy() const -> bool { return healthy; }

auto IonService::isRunning() const -> bool { return running; }
