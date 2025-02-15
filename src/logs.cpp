#include "logs.h"

#include <cstddef>
#include <iomanip>
#include <iostream>
#include <span>
#include <sstream>
#include <string>
#include <string_view>

// Function to log binary buffer content in hex format
void log_buffer(const std::string& name, std::span<const std::byte> buffer)
{
    std::ostringstream oss;
    oss << name << " (" << buffer.size() << " bytes): ";

    for (const std::byte& byte : buffer) {
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte)
            << " ";
    }

    log_debug("{}", oss.str());
}
