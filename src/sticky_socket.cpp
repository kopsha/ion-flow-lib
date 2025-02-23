#include "sticky_socket.h"
#include "console.h"
#include "easy_socket.h"
#include "ipv4_socket.h"

#include <cerrno>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string>

#include <netinet/in.h>
#include <sys/poll.h>

std::string get_current_time()
{
    using namespace std::chrono;

    auto now = system_clock::now();
    auto time = system_clock::to_time_t(now);
    auto local_time = *std::localtime(&time);

    // Format time manually using stringstream
    std::ostringstream oss;
    oss << std::put_time(&local_time, "%H:%M:%S");

    // Get milliseconds
    auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;
    oss << '.' << std::setfill('0') << std::setw(3) << ms.count();

    return oss.str();
}

StickySocket::StickySocket(
    const IoIntf& useIo,
    std::string host,
    uint16_t port,
    size_t retries
)
    : IPv4Socket(useIo, std::move(host), port)
    , maxRetries(retries)
    , attempts(0)
    , backOff(0)
    , keepTrying(false)
{
    CONSOLE_TRACE(this->host);
}

auto StickySocket::connect() -> bool
{
    CONSOLE_TRACE(host);
    console::debug("{}", host);

    keepTrying = true;
    attempts = 0;
    return reconnect();
}

void StickySocket::disconnect()
{
    CONSOLE_TRACE(host);
    console::debug("{}", host);

    keepTrying = false;
    IPv4Socket::disconnect();
}

auto StickySocket::reconnect() -> bool
{
    bool willConnect = false;

    if (keepTrying && state == ConnectionState::Disconnected)
    {
        if (backOff)
        {
            backOff--;
        }
        else
        {
            willConnect = IPv4Socket::connect();
        }
    }

    return willConnect;
}

auto StickySocket::eval(const struct pollfd& response) -> bool
{
    ConnectionState last { state };
    auto hasChanged = IPv4Socket::eval(response);
    if (last == ConnectionState::Connecting && state == ConnectionState::Disconnected)
    {
        attempts++;
        if (keepTrying)
        {
            backOff = (1 << (std::min(attempts, maxRetries) + 1)) * BACKOFF_MULTIPLIER;
            console::warning(
                "{} => {} connection timeout // next connection attempt in {} cycles.",
                get_current_time(), host, backOff
            );
        }
    }
    else if (last == ConnectionState::Connecting && state == ConnectionState::Connected)
    {
        attempts = 0;
        keepTrying = true;
        backOff = 0;
    }
    return hasChanged;
}

auto StickySocket::step() -> bool { return false; }
