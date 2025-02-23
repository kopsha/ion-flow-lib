#pragma once

#include "easy_socket.h"
#include "ioi.h"

#include <cstdint>

class IPv4Socket : public EasySocketIntf
{
  public:
    IPv4Socket(const IoIntf& ioRef, std::string host, uint16_t port);
    ~IPv4Socket() override;
    IPv4Socket(IPv4Socket&&) noexcept;

    // actions
    auto enter(ConnectionState newState) -> bool override;
    auto connect() -> bool override;
    void disconnect() override;
    auto eval(const struct pollfd& response) -> bool override;
    auto send(std::span<const uint8_t> buffer) -> int override;
    auto receive() -> std::span<const uint8_t> override;

    // notifications
    void didReceived(std::span<const uint8_t> data) override;
    void wentOnline() override;
    void wentOffline() override;

  private:
    void canReceive();
    void canSend();

    const IoIntf& io;
    std::array<uint8_t, BUFFER_SIZE> rxBuffer;
};
