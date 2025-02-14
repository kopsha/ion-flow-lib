#ifndef HELPERS_H
#define HELPERS_H

#include <string>

auto get_hostname() -> std::string;
auto get_mac_address() -> std::string;

auto sockaddr_as_string(const struct sockaddr* addr) -> std::string;
void log_addrinfo(const struct addrinfo* info);
void log_revents(short int revents);

#endif // !HELPERS_H
