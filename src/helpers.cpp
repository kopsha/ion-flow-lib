#include "helpers.h"
#include "logs.h"

#include <array>
#include <bitset>
#include <climits>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <sstream>
#include <stdexcept>

#include <arpa/inet.h>
#include <fcntl.h>
#include <net/if.h>
#include <netdb.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <unistd.h>

auto get_hostname() -> std::string
{
    std::array<char, HOST_NAME_MAX + 1> hostname;

    int err = gethostname(hostname.data(), HOST_NAME_MAX);
    if (err < 0) {
        return std::move(std::string());
    }
    hostname[HOST_NAME_MAX] = '\0';

    return std::move(std::string(hostname.data()));
}

auto get_mac_address() -> std::string
{
    std::stringstream builder;
    struct ifreq ifr;

    int descriptor = socket(AF_INET, SOCK_DGRAM, 0);
    if (descriptor < 0) {
        return std::move(std::string());
    }

    // TODO: consider listing all network interfaces
    std::strncpy(ifr.ifr_name, "eno1", IFNAMSIZ - 1);
    if (ioctl(descriptor, SIOCGIFHWADDR, &ifr) == 0) {
        auto* hwaddr = (unsigned char*)ifr.ifr_hwaddr.sa_data;
        builder << std::hex << std::setfill('0') << std::setw(2)
                << static_cast<int>(hwaddr[0]) << ":" << std::setw(2)
                << static_cast<int>(hwaddr[1]) << ":" << std::setw(2)
                << static_cast<int>(hwaddr[2]) << ":" << std::setw(2)
                << static_cast<int>(hwaddr[3]) << ":" << std::setw(2)
                << static_cast<int>(hwaddr[4]) << ":" << std::setw(2)
                << static_cast<int>(hwaddr[5]); // NOLINT(anything else will look worse)
    }
    close(descriptor);

    return std::move(builder.str());
}

auto sockaddr_as_string(const struct sockaddr* addr) -> std::string
{
    std::array<char, INET6_ADDRSTRLEN> ip_str;

    if (addr->sa_family == AF_INET) {
        // IPv4 address
        auto* ipv4 = (struct sockaddr_in*)addr;
        inet_ntop(AF_INET, &(ipv4->sin_addr), ip_str.data(), sizeof(ip_str));
        return std::move(std::string { ip_str.data() });
    }

    if (addr->sa_family == AF_INET6) {
        // IPv6 address
        auto* ipv6 = (struct sockaddr_in6*)addr;
        inet_ntop(AF_INET6, &(ipv6->sin6_addr), ip_str.data(), ip_str.size());
        return std::move(std::string { ip_str.data() });
    }

    return std::move(std::string { "Unknown address family" });
}

void log_addrinfo(const struct addrinfo* info)
{
    while (info != nullptr) {
        log_debug("Flags: %d\n", info->ai_flags);
        log_debug(
            "Family: %d (%s)\n", info->ai_family,
            (info->ai_family == AF_INET        ? "IPv4"
                 : info->ai_family == AF_INET6 ? "IPv6" // NOLINT(i know this sucks)
                                               : "Other")
        );
        log_debug("Socket Type: %d\n", info->ai_socktype);
        log_debug("Protocol: %d\n", info->ai_protocol);
        log_debug("Address Length: %zu\n", info->ai_addrlen);

        log_debug(
            "Address: %s\n",
            (info->ai_addr != nullptr) ? sockaddr_as_string(info->ai_addr).c_str()
                                       : "(null)"
        );
        log_debug(
            "Canonical Name: %s\n",
            (info->ai_canonname != nullptr) ? info->ai_canonname : "(null)"
        );
        log_debug("----------------------------------\n");
        info = info->ai_next;
    }
}

void log_revents(short int revents)
{
    log_debug(
        "Revents (0b%s):\n", std::bitset<sizeof(revents) * 2>(revents).to_string().c_str()
    );

    // NOLINTBEGIN(readability-implicit-bool-conversion)
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
    // NOLINTEND(readability-implicit-bool-conversion)
}
