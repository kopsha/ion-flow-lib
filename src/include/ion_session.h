#pragma once

#include "ioi.h"
#include "sticky_socket.h"

#include <cstdint>
#include <span>
#include <string>

class IonSession : public StickySocket
{
  public:
    IonSession(const IoIntf& useIo, std::string host, uint16_t port);

    // notifications
    void didReceived(std::span<const uint8_t> data) override;
    void wentOnline() override;
    void wentOffline() override;
};
