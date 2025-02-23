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
    int setup();
    void start(std::stop_token superToken);
    void stop();
    void resetHealth();

    // inspectors
    [[nodiscard]] auto isRunning() const -> bool;
    [[nodiscard]] auto isHealthy() const -> bool;

    // notifications
    void onEntry();
    void onExit();

  protected:
    void loop(std::stop_token token);

  private:
    std::atomic<bool> healthy;
    std::jthread worker;
    StickyEngine engine;
};
