#ifndef STICKY_SOCKET_H
#define STICKY_SOCKET_H

#include "rc_limits.h"
#include <netdb.h>
#include <span>
#include <string>

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

    void send(const std::span<const byte> buffer);

    const std::string& getHost() const;
    const std::string& getStatus() const;
    int getDescriptor() const;
    bool isAlive() const;

    virtual void wentOnline();
    virtual void wentOffline();
    virtual void didReceived(const std::span<byte> buffer);

private:
    std::span<byte> receive();
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
    byte rxBuffer[BUFFER_SIZE];
};

#endif // !STICKY_SOCKET_H
