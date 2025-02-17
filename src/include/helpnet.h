#pragma once

#include <string>
#include <vector>

namespace helpnet {
struct Interface {
    std::string name;
    std::string mac;
};

auto readHostname() -> std::string;
auto listInterfaces() -> std::vector<Interface>;
}
