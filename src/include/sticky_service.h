#ifndef RC_AGENT_H
#define RC_AGENT_H

#include "sticky_socket.h"

#include <atomic>
#include <thread>
#include <unordered_map>
#include <vector>

class IonService {
public:
    IonService();
    ~IonService();

    void start();
    void stop();
    void resetHealth();

    bool isRunning() const;
    bool isHealthy() const;

private:
    void loop();
    std::vector<struct pollfd> make_poll_descriptors() const;

private:
    std::atomic<bool> healthy;
    std::atomic<bool> running;
    std::thread worker;

    std::unordered_map<int, StickySocket*> connections;
};

#endif // RC_AGENT_H
