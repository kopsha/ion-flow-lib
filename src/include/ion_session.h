#pragma once

#include "ioi.h"
#include "sticky_socket.h"

#include <span>
#include <string>
#include <cstdint>

class IonSession : public StickySocket
{
  public:
    IonSession(
        const IoIntf& useIo,
        std::string host,
        uint16_t port
    );
    ~IonSession() override;

    // notifications
    void didReceived(std::span<const uint8_t> data) override;
    void wentOnline() override;
    void wentOffline() override;
};
