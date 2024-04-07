#include "InetAddress.h"

InetAddress::InetAddress(uint16_t port, std::string ip)
{
    ::bzero(&_addr, sizeof(_addr));
    _addr.sin_family = AF_INET;
    _addr.sin_port = ::htons(port);
    //_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    _addr.sin_addr.s_addr = inet_addr(ip.c_str());
}


std::string InetAddress::toIp() const
{
    char buf[64] = {0};
    ::inet_ntop(AF_INET, &_addr.sin_addr, buf, sizeof(buf));
    return buf;
}

std::string InetAddress::toIpPort() const
{
    char buf[64] = {0};
    ::inet_ntop(AF_INET, &_addr.sin_addr, buf, sizeof(buf));
    size_t end = ::strlen(buf);
    uint16_t port = ::ntohs(_addr.sin_port);
    sprintf(buf + end, ":%u", port);
    return buf;
}

uint16_t InetAddress::toPort() const
{
    return ::ntohs(_addr.sin_port);
}
