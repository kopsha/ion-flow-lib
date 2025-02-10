#include "sticky_socket.h"
#include <iostream>
#include <poll.h>
#include <span>

int main()
{
    struct {
        std::string name;
        std::string host;
        int port;
    } conn = { "x", "localhost", 5000 };
    StickySocket ss(conn.name, conn.host, conn.port);
    std::cout << "before connect" << conn.host << std::endl;
    ss.open();
    std::cout << "connecting" << conn.host << std::endl;

    int rc;
    StickySocket::Event ev, last;
    std::span<byte> data;

    while (true) {
        struct pollfd pfds[]
            = { { .fd = ss.getDescriptor(), .events = POLLIN | POLLPRI | POLLOUT } };
        rc = poll(pfds, 1, 8);
        if (rc > 0) {
            ev = ss.parse(pfds[0]);
            if (ev != last) {
                last = ev;
                switch (ev) {
                case StickySocket::Event::Connected:
                    std::cout << "Connected" << std::endl;
                    break;
                case StickySocket::Event::Received:
                    data = ss.receive();
                    std::cout << "Received " << data.size() << " bytes: " << data.data()
                              << std::endl;
                    break;
                case StickySocket::Event::Failed:
                    ss.close();
                    std::cout << "Closed" << std::endl;
                    break;
                default:
                    break;
                }
            }
        }
    }
    return 0;
}
