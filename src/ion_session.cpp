#include "console.h"
#include "sticky_socket.h"
#include "ion_session.h"

IonSession::IonSession(const IoIntf& useIo, std::string host, uint16_t port)
    : StickySocket(useIo, host, port)
{
    CONSOLE_TRACE(host);
}
