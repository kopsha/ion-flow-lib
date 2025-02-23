#pragma once

#include "ioi.h"
#include "sticky_engine.h"

#include <atomic>
#include <thread>

class IonService
{
  public:
    IonService(const IoIntf& useIo);
    ~IonService();

    // bad luck
    IonService(const IonService&) = delete;
    IonService& operator=(const IonService&) = delete;
    IonService(IonService&&) = delete;
    IonService& operator=(IonService&&) = delete;

    // actions
    void start();
    void stop();
    void resetHealth();

    // inspectors
    [[nodiscard]] auto isRunning() const -> bool;
    [[nodiscard]] auto isHealthy() const -> bool;

  protected:
    void loop();

  private:
    std::atomic<bool> healthy;
    std::atomic<bool> running;
    std::thread worker;
    StickyEngine engine;
};
