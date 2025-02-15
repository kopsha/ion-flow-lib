#ifndef STICKY_SOCKET_H
#define STICKY_SOCKET_H

#include "sizes.h"

#include <cstdint>
#include <span>
#include <string>

/***
 * Once connected it sticks, at least until retry limit was reached
 */
class StickySocket {
public:
    // numbers and types
    static constexpr int INVALID_SOCKET = -1;
    static constexpr size_t DEFAULT_RETRIES = 5;
    static constexpr int BACKOFF_MULTIPLIER = 233;
    enum class ConnectionState : uint_fast8_t {
        None,
        Disconnected,
        Connecting,
        Retry,
        Connected
    };

    // you know
    StickySocket(std::string host, int port, int retries = DEFAULT_RETRIES);
    ~StickySocket();

    // actions
    void connect();
    void disconnect();
    auto eval(const struct pollfd& response) -> ConnectionState;
    void send(std::span<const std::byte> buffer);

    // inspectors
    [[nodiscard]] auto getHost() const -> const std::string&;
    [[nodiscard]] auto getStatus() const -> const std::string&;
    [[nodiscard]] auto getState() const -> ConnectionState;
    [[nodiscard]] auto getDescriptor() const -> int;
    [[nodiscard]] auto isAlive() const -> bool;
    [[nodiscard]] auto isOnline() const -> bool;

    // notifications
    virtual void wentOnline();
    virtual void wentOffline();
    virtual void didReceived(std::span<std::byte> buffer);

private:
    auto receive() -> std::span<std::byte>;
    void reconnect();
    auto open() -> bool;
    void close();
    void enter(ConnectionState newState);
    void handlePollError();
    void handleDataReceive();
    void handlePollOut();

    // members
    int descriptor;
    ConnectionState status;
    bool online;
    std::string host;
    std::string port;
    int maxRetries;
    int attempts;
    int backOff;
    std::array<std::byte, BUFFER_SIZE> rxBuffer;
};

#endif // !STICKY_SOCKET_H
