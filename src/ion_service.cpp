#include "ion_service.h"
#include "logs.h"
#include "sticky_socket.h"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <thread>
#include <vector>
#include <memory>
#include <utility>

#include <sys/poll.h>

constexpr int EVENT_WINDOW = 21;
constexpr int CHILL_WINDOW = 13;

IonService::IonService()
    : running(false)
    , healthy(true)
{
}

IonService::~IonService() { stop(); }

void IonService::start()
{
    const bool alreadyRunning = running.exchange(true);
    if (alreadyRunning) {
        log_warning("service is already running, start ignored.\n");
        return;
    }
    worker = std::thread(&IonService::loop, this);
}

void IonService::stop()
{
    running = false;
    if (worker.joinable()) {
        worker.join();
    }
}

auto IonService::attach(std::unique_ptr<StickySocket> skt) -> int
{
    auto key = skt->getDescriptor();
    if (key <= 0) {
        return StickySocket::INVALID_SOCKET;
    }
    auto [it, inserted] = connections.try_emplace(key, std::move(skt));
    return inserted ? key : StickySocket::INVALID_SOCKET;
}

auto IonService::detach(int descriptor) -> std::unique_ptr<StickySocket>
{
    if (auto skt = connections.extract(descriptor); skt) {
        return std::move(skt.mapped());
    }
    return nullptr;
}

auto IonService::rebuild_poll_params() const -> std::vector<struct pollfd>
{
    std::vector<struct pollfd> requests;
    requests.reserve(connections.size());
    for (const auto& skt : connections) {
        if (skt.second->isAlive()) {
            requests.push_back({ skt.first, POLLIN | POLLPRI | POLLOUT, 0 });
        }
    }
    return requests;
}

void IonService::loop()
{
    log_info("Starting event loop.");

    while (running) {
        healthy = true;

        // TODO: refactor big time
        auto params = rebuild_poll_params();
        int err = poll(params.data(), params.size(), EVENT_WINDOW);
        if (err < 0) {
            log_error("Polling error: %d.", err);
        } else if (err > 0) {
            for (const auto response : params) {
                auto& skt = connections[response.fd];
                /*log_revents(response.revents);*/
                skt->eval(response);
            }
        }

        if (std::ranges::none_of(connections, [](const auto& entry) {
                return entry.second->isAlive();
            })) {
            log_warning("all connections are dead");
            running = false;
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(CHILL_WINDOW));
        }
    }

    log_info("Closing event loop.\n");
}

void IonService::resetHealth() { healthy.store(false); }

auto IonService::isHealthy() const -> bool { return healthy; }

auto IonService::isRunning() const -> bool { return running; }

auto IonService::connectionsCount() const -> size_t { return connections.size(); }
