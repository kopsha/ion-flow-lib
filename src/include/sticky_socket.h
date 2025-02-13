#ifndef STICKY_SOCKET_H
#define STICKY_SOCKET_H

#include <netdb.h>
#include <span>
#include <string>

static constexpr size_t BUFFER_SIZE = 1024;

/***
 * Once connected it sticks, at least until retry limit was reached
 */
class StickySocket {
public:
    enum class ConnectionState { Disconnected, Connecting, Retry, Connected, None };

public:
    StickySocket(const std::string& host, int port, const int retries = 5);
    ~StickySocket();

    void connect();
    void disconnect();
    ConnectionState eval(const struct pollfd& response);

    void send(const std::span<const std::byte> buffer);

    const std::string& getHost() const;
    const std::string& getStatus() const;
    int getDescriptor() const;
    bool isAlive() const;

    virtual void wentOnline();
    virtual void wentOffline();
    virtual void didReceived(const std::span<std::byte> buffer);

private:
    std::span<std::byte> receive();
    void reconnect();
    bool open();
    void close();
    void enter(ConnectionState newState);

protected:
    bool online;

private:
    int descriptor;
    ConnectionState status;
    std::string host;
    std::string port;
    int maxRetries;
    int attempts;
    int backOff;
    std::byte rxBuffer[BUFFER_SIZE];
};

#endif // !STICKY_SOCKET_H
