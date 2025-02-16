#pragma once

#include "sticky_socket.h"

class IonSession : public StickySocket {
public:
    IonSession(std::string host, int port, int retries = DEFAULT_RETRIES)
        : StickySocket(std::move(host), port, retries)
    {
    }

private:
    void wentOffline() override;
    void wentOnline() override;
    void didReceived(std::span<std::byte> buffer) override;
};
