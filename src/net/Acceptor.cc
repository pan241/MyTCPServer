#include "Acceptor.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include "Logger.h"
#include "SocketOperation.h"

#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>

 Acceptor::Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport)
    : _loop(loop),
      _acceptSocket(sockets::createNonblockingOrDie(listenAddr.family())),
      _acceptChannel(loop, _acceptSocket.fd()),
      _listening(false),
    _idleFd(::open("/dev/null", O_RDONLY | O_CLOEXEC))
{
    _acceptSocket.setReuseAddr(true);
    _acceptSocket.setReusePort(reuseport);
    _acceptSocket.bindAddress(listenAddr);
    _acceptChannel.setReadCallback(std::bind(&Acceptor::handleRead, this));
}

Acceptor::~Acceptor()
{
    _acceptChannel.disableAll();
    _acceptChannel.remove();
    ::close(_idleFd);
}

void Acceptor::listen()
{
    _listening = true;
    _acceptSocket.listen();
    _acceptChannel.enableReading();
}

void Acceptor::handleRead()
{
    InetAddress peerAddr;

    int connfd = _acceptSocket.accept(&peerAddr);
    if (connfd >= 0)
    {
        if (_newConnectionCallback)
        {
            _newConnectionCallback(connfd, peerAddr);
        }
        else
        {
            ::close(connfd);
        }
    }
    else
    {
        LOG_ERROR << "in Acceptor::handleRead";
        if (errno == EMFILE)
        {
            /*
            * ::close(_idleFd);
            * _idleFd = ::accept(_acceptSocket.fd(), NULL, NULL);
            * ::close(_idleFd);
            * _idleFd = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
            */
            LOG_ERROR << "socket read limit";
        }
    }
}