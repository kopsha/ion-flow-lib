#include "helpers.h"
#include "logs.h"

#include <climits>
#include <cstring>
#include <iomanip>
#include <sstream>
#include <stdexcept>

#include <arpa/inet.h>
#include <fcntl.h>
#include <net/if.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <unistd.h>


const std::string get_hostname()
{
    char hostname[HOST_NAME_MAX + 1];

    int rc = gethostname(hostname, HOST_NAME_MAX);
    if (rc < 0) {
        throw std::runtime_error("Failed to read hostname.");
    }
    hostname[HOST_NAME_MAX] = '\0';

    return std::string(hostname);
}

const std::string get_mac_address()
{
    std::stringstream builder;
    struct ifreq ifr;

    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        throw std::runtime_error("Failed to read mac address.");
    }

    // TODO: consider listing all network interfaces
    std::strncpy(ifr.ifr_name, "eno1", IFNAMSIZ - 1);
    if (ioctl(fd, SIOCGIFHWADDR, &ifr) == 0) {
        unsigned char* hwaddr = (unsigned char*)ifr.ifr_hwaddr.sa_data;
        builder << std::hex << std::setfill('0') << std::setw(2)
                << static_cast<int>(hwaddr[0]) << ":" << std::setw(2)
                << static_cast<int>(hwaddr[1]) << ":" << std::setw(2)
                << static_cast<int>(hwaddr[2]) << ":" << std::setw(2)
                << static_cast<int>(hwaddr[3]) << ":" << std::setw(2)
                << static_cast<int>(hwaddr[4]) << ":" << std::setw(2)
                << static_cast<int>(hwaddr[5]);
    }
    close(fd);

    return builder.str();
}

bool setSocketNonBlocking(int socket_fd)
{
    // WARNING: this may fail, check if flags are positive someday
    int flags = fcntl(socket_fd, F_GETFL, 0);
    int rc = fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK);
    return rc == 0;
}

std::string sockaddr_as_string(const struct sockaddr* addr)
{
    char ip_str[INET6_ADDRSTRLEN];

    if (addr->sa_family == AF_INET) {
        // IPv4 address
        struct sockaddr_in* ipv4 = (struct sockaddr_in*)addr;
        inet_ntop(AF_INET, &(ipv4->sin_addr), ip_str, sizeof(ip_str));
        return std::string(ip_str);
    } else if (addr->sa_family == AF_INET6) {
        // IPv6 address
        struct sockaddr_in6* ipv6 = (struct sockaddr_in6*)addr;
        inet_ntop(AF_INET6, &(ipv6->sin6_addr), ip_str, sizeof(ip_str));
        return std::string(ip_str);
    } else {
        return "Unknown address family";
    }
}

void print_addrinfo(const struct addrinfo* info)
{
    while (info != nullptr) {
        log_debug("Flags: %d\n", info->ai_flags);
        log_debug(
            "Family: %d (%s)\n", info->ai_family,
            (info->ai_family == AF_INET        ? "IPv4"
                 : info->ai_family == AF_INET6 ? "IPv6"
                                               : "Other")
        );
        log_debug("Socket Type: %d\n", info->ai_socktype);
        log_debug("Protocol: %d\n", info->ai_protocol);
        log_debug("Address Length: %zu\n", info->ai_addrlen);

        log_debug(
            "Address: %s\n",
            (info->ai_addr) ? sockaddr_as_string(info->ai_addr).c_str() : "(null)"
        );
        log_debug(
            "Canonical Name: %s\n", (info->ai_canonname) ? info->ai_canonname : "(null)"
        );
        log_debug("----------------------------------\n");
        info = info->ai_next;
    }
}
