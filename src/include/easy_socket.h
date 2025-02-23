#pragma once

#include <cstdint>
#include <netdb.h>
#include <string>
#include <sys/socket.h>

#include <span>

class EasySocketIntf
{
  public:
    static constexpr int INVALID_SOCKET = -1;
    static constexpr int BUFFER_SIZE = 4096;
    enum class ConnectionState : uint8_t
    {
        Disconnected,
        Connecting,
        Connected,
    };

    EasySocketIntf(std::string host, uint16_t port);
    virtual ~EasySocketIntf();
    EasySocketIntf(EasySocketIntf&&) noexcept = default;

    // Delete copy operations
    EasySocketIntf(const EasySocketIntf&) = delete;
    auto operator=(const EasySocketIntf&) -> EasySocketIntf& = delete;

    // inspectors
    [[nodiscard]] auto getDescriptor() const -> int;
    [[nodiscard]] auto getHost() const -> const std::string&;
    [[nodiscard]] auto getState() const -> ConnectionState;
    [[nodiscard]] auto getStatus() const -> const std::string&;
    [[nodiscard]] auto isOnline() const -> bool;

    // actions
    virtual auto eval(const struct pollfd& response) -> bool = 0;
    virtual auto enter(ConnectionState newState) -> bool = 0;
    virtual auto connect() -> bool = 0;
    virtual void disconnect() = 0;
    virtual auto send(std::span<const uint8_t> buffer) -> int = 0;
    virtual auto receive() -> std::span<const uint8_t> = 0;

    // notifications
    virtual void didReceived(std::span<const uint8_t> data) = 0;
    virtual void wentOnline() = 0;
    virtual void wentOffline() = 0;

  protected:
    // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
    int descriptor;
    ConnectionState state;
    std::string host;
    uint16_t port;
    // NOLINTEND(misc-non-private-member-variables-in-classes)
};
