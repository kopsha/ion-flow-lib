#ifndef RC_AGENT_H
#define RC_AGENT_H

#include "sticky_socket.h"

#include <atomic>
#include <cstddef>
#include <memory>
#include <thread>
#include <unordered_map>
#include <vector>

class IonService {
public:
    IonService();
    ~IonService();

    // actions
    void start();
    void stop();
    void resetHealth();

    auto attach(std::unique_ptr<StickySocket> skt) -> int;
    auto detach(int descriptor) -> std::unique_ptr<StickySocket>;

    // inspectors
    auto isRunning() const -> bool;
    auto isHealthy() const -> bool;
    auto connectionsCount() const -> size_t;

private:
    void loop();
    auto rebuild_poll_params() const -> std::vector<struct pollfd>;

    // members
    std::atomic<bool> healthy;
    std::atomic<bool> running;
    std::thread worker;
    std::unordered_map<int, std::unique_ptr<StickySocket>> connections;
};

#endif // RC_AGENT_H
