#pragma once

#include "ioi.h"
#include "sticky_socket.h"

#include <cstdint>
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

    // actions
    auto makeSocket(std::string host, uint16_t port) -> StickySocket&;
    int poll(int duration);

  protected:
    void rebuild_poll_params();

  private:
    const IoIntf& io;
    std::vector<StickySocket> connections;
    std::vector<struct pollfd> responses;
};
