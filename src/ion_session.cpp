#include "console.h"
#include "ion_session.h"
#include "sticky_socket.h"
#include <cstdint>
#include <string_view>

using namespace std::literals;

IonSession::IonSession(const IoIntf& useIo, std::string host, uint16_t port)
    : StickySocket(useIo, host, port)
{
    CONSOLE_TRACE(host);
}

void IonSession::didReceived(std::span<const uint8_t> data)
{
    console::info("got {} bytes", data.size());
}

void IonSession::wentOnline()
{
    console::info("connected");
    const std::string_view text { "hello"sv };
    send(std::span{reinterpret_cast<const uint8_t *>(text.data()), text.size()});
}

void IonSession::wentOffline() { console::info("disconnected"); }
