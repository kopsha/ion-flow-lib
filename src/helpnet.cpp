#include "helpnet.h"
#include "logs.h"

#include <array>
#include <bitset>
#include <filesystem>
#include <fstream>
#include <string>
#include <utility>
#include <vector>

#include <dirent.h>
#include <unistd.h>
#include <sys/poll.h>
#ifdef __USE_POSIX
  #include <bits/posix1_lim.h>
#endif

namespace fs = std::filesystem;

namespace {

auto fromFile(const fs::path& addrpath) -> std::string
{
    std::ifstream file(addrpath);
    if (!file.is_open()) {
        log_error("Cannot open {}.\n", addrpath.string());
        return {};
    }

    std::string address;
    if (!std::getline(file, address)) {
        log_error("Cannot read {}.\n", addrpath.string());
        return {};
    }
    return std::move(address);
}

} // anonymous namespace

namespace helpnet {

auto readHostname() -> std::string
{
    std::array<char, HOST_NAME_MAX + 1> hostname {};

    const int err = gethostname(hostname.data(), HOST_NAME_MAX);
    if (err < 0) {
        log_error("Cannot read hostname.\n");
        return std::move(std::string());
    }
    hostname[HOST_NAME_MAX] = '\0';

    return std::move(std::string(hostname.data()));
}

auto listInterfaces() -> std::vector<Interface>
{
    std::vector<Interface> interfaces;
    const fs::path netDir { "/sys/class/net" };

    if (!fs::exists(netDir) || !fs::is_directory(netDir)) {
        log_error("Directory {} is not accessible.\n", netDir.string());
        return std::move(interfaces);
    }

    for (const auto& entry : fs::directory_iterator(netDir)) {
        if (entry.is_directory() || entry.is_symlink()) {
            const fs::path address = entry.path() / "address";
            log_info("Checking {}\n", address.string());
            if (fs::exists(address)) {
                struct Interface intf {
                    .name = entry.path().filename().string(),
                    .mac = fromFile(address),
                };
                interfaces.push_back(intf);
            }
        }
    }

    return std::move(interfaces);
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
} // namespace helpnet
