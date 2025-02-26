#include "helpnet.h"
#include "console.h"

#include <array>
#include <filesystem>
#include <fstream>
#include <string>
#include <utility>
#include <vector>

#include <dirent.h>
#include <unistd.h>
#ifdef __USE_POSIX
  #include <bits/posix1_lim.h>
#endif

namespace fs = std::filesystem;

namespace
{

auto fromFile(const fs::path& addrpath) -> std::string
{
    std::ifstream file(addrpath);
    if (!file.is_open())
    {
        console::error("Cannot open {}.", addrpath.string());
        return {};
    }

    std::string address;
    if (!std::getline(file, address))
    {
        console::error("Cannot read {}.", addrpath.string());
        return {};
    }
    return std::move(address);
}

} // anonymous namespace

namespace helpnet
{

auto readHostname() -> std::string
{
    std::array<char, HOST_NAME_MAX + 1> hostname {};

    const int err = gethostname(hostname.data(), HOST_NAME_MAX);
    if (err < 0)
    {
        console::error("Cannot read hostname.");
        return std::move(std::string());
    }
    hostname[HOST_NAME_MAX] = '\0';

    return std::move(std::string(hostname.data()));
}

auto listInterfaces() -> std::vector<Interface>
{
    std::vector<Interface> interfaces;
    const fs::path netDir { "/sys/class/net" };

    if (!fs::exists(netDir) || !fs::is_directory(netDir))
    {
        /*console::error("Directory {} is not accessible.", netDir.string());*/
        return std::move(interfaces);
    }

    for (const auto& entry : fs::directory_iterator(netDir))
    {
        if (entry.is_directory() || entry.is_symlink())
        {
            const fs::path address = entry.path() / "address";
            if (fs::exists(address))
            {
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

auto guessMacAddress() -> std::string
{
    auto interfaces = listInterfaces();
    std::string result;
    for (const auto& intf : interfaces)
    {
        console::info("netw_if: {} @ {}", intf.name, intf.mac);
        if (intf.name != "lo")
        {
            result = intf.mac;
            break;
        }
    }
    return std::move(result);
}

} // namespace helpnet
