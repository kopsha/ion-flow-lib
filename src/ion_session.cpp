#include "ion_session.h"
#include "logs.h"

void IonSession::wentOnline()
{
    LOG_TRACE();
    std::string anything { "hello world!" };
    send(std::span<std::byte>(reinterpret_cast<std::byte*>(anything.data()), anything.size()));
}

void IonSession::wentOffline() { LOG_TRACE(); }

void IonSession::didReceived(std::span<std::byte> buffer) { LOG_TRACE(); }
