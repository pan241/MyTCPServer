#include "Connector.h"

#include <errno.h>

#include "Logger.h"
#include "SocketOperation.h"

const int Connector::kMaxRetryDelayMs;

Connector::Connector(EventLoop* loop, const InetAddress& serverAddr)
    : _loop(loop),
      _serverAddr(serverAddr),
      _connect(false),
      _state(kDisconnected),
      _retryDelayMs(kInitRetryDelayMs)
{
    LOG_DEBUG << "ctor[" << this << "]";
}

Connector::~Connector()
{
    LOG_DEBUG << "dtor[" << this << "]";
}

void Connector::start()
{
    _connect = true;
    _loop->runInLoop(std::bind(&Connector::startInLoop, this));
}

void Connector::restart()
{
    setState(kDisconnected);
    _retryDelayMs = kInitRetryDelayMs;
    _connect = true;
    startInLoop();
}

void Connector::stop()
{
    _connect = false;
    _loop->queueInLoop(std::bind(&Connector::startInLoop, this));
}

void Connector::startInLoop()
{
    if (_connect)
    {
        connect();
    }
    else
    {
        LOG_DEBUG << "do not connect";
    }
}

void Connector::stopInLoop()
{
    if (_state == kConnecting)
    {
        setState(kDisconnected);
        int sockfd = removeAndResetChannel();
        retry(sockfd);
    }
}

void Connector::connect()
{
    int sockfd = sockets::createNonblockingOrDie(_serverAddr.family());
    int ret = sockets::connect(sockfd, (sockaddr*)_serverAddr.getSockAddr());
    int savedErrno = (ret == 0) ? 0 : errno;
    switch(savedErrno)
    {
        case 0:
        case EINPROGRESS:
        case EINTR:
        case EISCONN:
        {
            connecting(sockfd);
            break;
        }

        case EAGAIN:
        case EADDRINUSE:
        case EADDRNOTAVAIL:
        case ECONNREFUSED:
        case ENETUNREACH:
        {
            retry(sockfd);
            break;
        }

        case EACCES:
        case EPERM:
        case EAFNOSUPPORT:
        case EALREADY:
        case EBADF:
        case EFAULT:
        case ENOTSOCK:
        {
            LOG_ERROR << "connect error in Connector::startInLoop" << savedErrno;
            sockets::close(sockfd);
            break;
        }

        default:
        {
            LOG_ERROR << "Unexpected  in Connector::startInLoop" << savedErrno;
            sockets::close(sockfd);
            break;
        }
    }
}

void Connector::connecting(int sockfd)
{
    setState(kConnecting);
    _channel.reset(new Channel(_loop, sockfd));
    _channel->setWriteCallback(std::bind(&Connector::handleWrite, this));
    _channel->setErrorCallback(std::bind(&Connector::handleError, this));
    _channel->enableWriting();
}

void Connector::handleWrite()
{
    if (_state == kConnecting)
    {
        int sockfd = removeAndResetChannel();
        int err = sockets::getSocketError(sockfd);
        if (err)
        {
            retry(sockfd);
        }
        else if (sockets::isSelfConnect(sockfd))
        {
            retry(sockfd);
        }
        else
        {
            setState(kConnected);
            if (_connect)
            {
                _newConnectionCallback(sockfd);
            }
            else
            {
                ::close(sockfd);
            }
        }
    }
    else
    {
    }
}

void Connector::handleError()
{
    LOG_ERROR << "Connector::handleError state =" << _state;
    if (_state == kConnecting)
    {
        int sockfd = removeAndResetChannel();
        int err = sockets::getSocketError(sockfd);
    }
}

void Connector::retry(int sockfd)
{
    ::close(sockfd);
    setState(kDisconnected);
    if (_connect)
    {
        LOG_INFO << "Connector::retry - Retry connecting to " << _serverAddr.toIpPort()
                 << " in " << _retryDelayMs << "milliseconds";
        _loop->runAfter(_retryDelayMs / 1000.0, std::bind(&Connector::startInLoop, shared_from_this()));
        _retryDelayMs = std::min(_retryDelayMs * 2, kMaxRetryDelayMs);
    }
    else
    {
        LOG_DEBUG << "do not connect";
    }
}

int Connector::removeAndResetChannel()
{
    _channel->disableAll();
    _channel->remove();
    int sockfd = _channel->fd();
    _loop->queueInLoop(std::bind(&Connector::resetChannel, this));
    return sockfd;
}

void Connector::resetChannel()
{
    _channel.reset();
}