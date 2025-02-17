#include "ionflow.h"

#include <gtest/gtest.h>

TEST(HelpNet, can_read_hostname) {
    std::string hostname = helpnet::readHostname();
    EXPECT_FALSE(hostname.empty());
}

TEST(HelpNet, can_list_interfaces) {
    auto interfaces = helpnet::listInterfaces();

    EXPECT_FALSE(interfaces.empty());
    for (const auto& intf : interfaces) {
        EXPECT_FALSE(intf.name.empty());
        EXPECT_FALSE(intf.mac.empty());
    }
}
