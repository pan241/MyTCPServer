#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/tcp.h>

#include "InetAddress.h"
#include "Socket.h"
#include "InetAddress.h"
#include "Logger.h"
#include "SocketOperation.h"

Socket::~Socket()
{
    sockets::close(_sockfd);
}

void Socket::bindAddress(const InetAddress& addr)
{
    if (0 != ::bind(_sockfd, (sockaddr*)addr.getSockAddr(), sizeof(sockaddr_in)))
    {
        LOG_FATAL << "bind sockfd: " << _sockfd << " fail";
    }
}

void Socket::listen()
{
    if (0 != ::listen(_sockfd, 1024))
    {
        LOG_FATAL << "listen sockfd: " << _sockfd << " fail";
    }

}

int Socket::accept(InetAddress* peeraddr)
{
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    memset(&addr, 0, len);
    int connfd = ::accept4(_sockfd, (sockaddr*)&addr, &len, SOCK_NONBLOCK | SOCK_CLOEXEC);
    if(connfd >= 0)
    {
        peeraddr->setSockAddr(addr);
    }
    return connfd;
}

void Socket::shutdownWrite()
{
    if (::shutdown(_sockfd, SHUT_WR) < 0)
    {
        LOG_ERROR << "shutdownWrite error";
    }
}

void Socket::setTcpNoDelay(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(_sockfd, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval));
}

void Socket::setReuseAddr(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
}


void Socket::setReusePort(bool on)
{
    int optval = on ? 1 : 0;
    int ret = ::setsockopt(_sockfd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
    if (ret < 0 && on)
    {
        LOG_ERROR << "SO_REUSEPORT failed";
    }
}

void Socket::setKeepAlive(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(_sockfd, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval));
}