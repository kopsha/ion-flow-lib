#include "helpers.h"
#include "logs.h"

#include <array>
#include <bitset>
#include <climits>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <ios>
#include <span>
#include <sstream>
#include <string>
#include <utility>

#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>
#ifdef __USE_POSIX
  #include <bits/posix1_lim.h>
#endif

auto get_hostname() -> std::string
{
    std::array<char, HOST_NAME_MAX + 1> hostname {};

    const int err = gethostname(hostname.data(), HOST_NAME_MAX);
    if (err < 0) {
        return std::move(std::string());
    }
    hostname[HOST_NAME_MAX] = '\0';

    return std::move(std::string(hostname.data()));
}

auto get_mac_address() -> std::string
{
    constexpr uint_fast8_t MAC_ADDR_LEN = 6;
    std::stringstream builder;
    struct ifreq ifr {};

    const int descriptor = socket(AF_INET, SOCK_DGRAM, 0);
    if (descriptor < 0) {
        return std::move(std::string());
    }

    // TODO: consider listing all network interfaces
    std::strncpy((char*)ifr.ifr_name, "eno1", IFNAMSIZ - 1);
    const int err = ioctl( // NOLINT(cppcoreguidelines-pro-type-vararg)
        descriptor, SIOCGIFHWADDR, &ifr
    );
    if (err == 0) {
        const auto hwaddr = std::span { ifr.ifr_hwaddr.sa_data };

        builder << std::hex << std::setfill('0');
        bool first = true;
        for (const auto& byte : hwaddr) {
            if (!first) {
                builder << ":";
            }
            first = false;
            builder << std::setw(2) << static_cast<int>(byte);
        }
    }
    close(descriptor);

    return std::move(builder.str());
}

void log_revents(short int revents)
{
    log_debug(
        "Revents (0b%s):\n", std::bitset<sizeof(revents) * 2>(revents).to_string().c_str()
    );

    if (revents & POLLIN) {
        log_debug("  POLLIN: Data to read\n");
    }
    if (revents & POLLPRI) {
        log_debug("  POLLPRI: Urgent data to read\n");
    }
    if (revents & POLLOUT) {
        log_debug("  POLLOUT: Ready for output\n");
    }
    if (revents & POLLERR) {
        log_debug("  POLLERR: Error condition\n");
    }
    if (revents & POLLHUP) {
        log_debug("  POLLHUP: Hang up\n");
    }
    if (revents & POLLNVAL) {
        log_debug("  POLLNAL: Invalid request\n");
    }
}
