#pragma once

#include "ioi.h"
#include "sticky_socket.h"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

class StickyEngine
{
  public:
    StickyEngine(const IoIntf& useIo);
    ~StickyEngine();

    // bad luck
    StickyEngine(const StickyEngine&) = delete;
    StickyEngine& operator=(const StickyEngine&) = delete;
    StickyEngine(StickyEngine&&) = delete;
    StickyEngine& operator=(StickyEngine&&) = delete;

    // factory methods
    template <typename T> StickySocket& makeSocket(std::string host, uint16_t port)
    {
        static_assert(
            std::is_base_of<StickySocket, T>::value, "T must be derived from StickySocket."
        );

        auto pSocket = std::make_unique<T>(io, host, port);
        connections.push_back(std::move(pSocket));
        responses.reserve(connections.size());
        return *connections.back();
    }

    // actions
    int poll(int duration);

  protected:
    void rebuild_poll_params();

  private:
    const IoIntf& io;
    std::vector<std::unique_ptr<StickySocket>> connections;
    std::vector<struct pollfd> responses;
};
