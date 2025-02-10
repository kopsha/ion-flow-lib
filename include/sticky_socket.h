#ifndef STICKY_SOCKET_H
#define STICKY_SOCKET_H

#include <netdb.h>
#include <span>
#include <string>

#define BUFFER_SIZE (1024u)
typedef unsigned char byte;

class StickySocket {
public:
    StickySocket(
        const std::string& name, const std::string& host, int port, const int retries = 5);
    ~StickySocket();

    bool isConnected() const;
    int getDescriptor() const;
    const std::string& getName() const;

    void open();
    void close();
    void send(const std::span<byte> buffer);
    std::span<byte> receive();

    enum class Event { None, Received, Failed, Connected };
    enum class ConnectionState { Disconnected, Connecting, Connected };

    const Event parse(const struct pollfd& response);
    struct ConnectionInfo {
        int family; // Address family (e.g., AF_INET)
        int socktype; // Socket type (e.g., SOCK_STREAM)
        int protocol; // Protocol (e.g., IPPROTO_TCP)
        struct sockaddr_storage addr; // Generic address storage
        socklen_t addrlen; // Length of the address
    };

private:
    std::string name;
    std::string host;
    std::string port;
    ConnectionState status;

    ConnectionInfo useConnection;
    int descriptor;
    std::array<byte, BUFFER_SIZE> rxBuffer;
    const int maxRetries;
    int retry;
};

#endif // !STICKY_SOCKET_H
