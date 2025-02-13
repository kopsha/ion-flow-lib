#ifndef HELPERS_H
#define HELPERS_H

#include <string>

const std::string get_hostname();
const std::string get_mac_address();

bool setSocketNonBlocking(int socket_fd);
std::string sockaddr_as_string(const struct sockaddr* addr);
void print_addrinfo(const struct addrinfo* info);

#endif // !HELPERS_H
