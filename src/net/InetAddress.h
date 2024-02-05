#ifndef INETADDRESS_H
#define INETADDRESS_H

#include <arpa/inet.h>

#include <string>


class InetAddress
{
public:
    explicit InetAddress(uint16_t port = 0, std::string ip = "127.0.0.1");

private:
    sockaddr_in _addr;
};

#endif