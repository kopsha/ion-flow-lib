#include "ion_session.h"
#include "console.h"
#include "sticky_socket.h"

IonSession::IonSession(const IoIntf& useIo, std::string host, uint16_t port)
    : StickySocket(useIo, host, port)
{
    CONSOLE_TRACE(host);
}
