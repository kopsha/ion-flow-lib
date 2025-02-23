#pragma once

#include "ioi.h"
#include "ipv4_socket.h"

#include <cstddef>
#include <cstdint>
#include <string>

/***
 * Once connected it sticks, at least until retry limit was reached
 */
class StickySocket : public IPv4Socket
{
  public:
    // numbers and types
    static constexpr size_t DEFAULT_RETRIES = 4;
    static constexpr size_t BACKOFF_MULTIPLIER = 144;

    // you know
    StickySocket(
        const IoIntf& useIo,
        std::string host,
        uint16_t port,
        size_t retries = DEFAULT_RETRIES
    );

    // actions
    auto connect() -> bool override;
    void disconnect() override;
    auto reconnect() -> bool;
    auto eval(const struct pollfd& response) -> bool override;
    virtual auto step() -> bool;

  private:
    bool keepTrying;
    size_t maxRetries;
    size_t attempts;
    size_t backOff;
};
