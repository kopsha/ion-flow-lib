#include "easy_socket.h"

EasySocketIntf::EasySocketIntf(std::string host, uint16_t port)
    : descriptor(INVALID_SOCKET)
    , state(ConnectionState::Disconnected)
    , host(std::move(host))
    , port(port)
{
}

EasySocketIntf::~EasySocketIntf() = default;

auto EasySocketIntf::getDescriptor() const -> int { return descriptor; }

auto EasySocketIntf::getState() const -> ConnectionState { return state; }

auto EasySocketIntf::getStatus() const -> const std::string&
{
    static std::string textual;
    switch (state)
    {
    case ConnectionState::Disconnected:
        textual = "Disconnected";
        break;
    case ConnectionState::Connecting:
        textual = "Connecting";
        break;
    case ConnectionState::Connected:
        textual = "Connected";
        break;
    }
    return textual;
}

auto EasySocketIntf::getHost() const -> const std::string& { return host; }

auto EasySocketIntf::isOnline() const -> bool
{
    return state == ConnectionState::Connected;
}
