#ifndef ION_SESSION_H
#define ION_SESSION_H

#include "sticky_socket.h"

class IonSession : public StickySocket {
    void wentOffline() override;
    void wentOnline() override;
    void didReceived(std::span<std::byte> buffer) override;
};

#endif
