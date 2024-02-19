#ifndef SOCKET_H
#define SOCKET_H

#include "../base/noncopyable.h"

class InetAddress;

class Socket : noncopyable
{
public:
    explicit Socket(int sockfd)
        : _sockfd(sockfd)
    {}

    ~Socket();

    int fd() const { return _sockfd; }

    void bindAddress(const InetAddress& localaddr);
    void listen();
    void accept(InetAddress* peeraddr);

    void shutdownWrite();
    void serTcpNoDelay(bool on);
    void serReuseAddr(bool on);
    void serReusePort(bool on);
    void serKeepAlive(bool on);

private:
    const int _sockfd;
};

#endif