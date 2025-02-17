#include "helpnet.h"
#include "logs.h"

#include <array>
#include <fstream>
#include <utility>

#include <dirent.h>
#include <unistd.h>
#ifdef __USE_POSIX
  #include <bits/posix1_lim.h>
#endif

#include <filesystem>
#include <string>
#include <vector>

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

} // namespace helpnet
