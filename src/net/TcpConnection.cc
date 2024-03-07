#include "TcpConnection.h"

#include <string.h>



#include "../log/Logging.h"
#include "Socket.h"
#include "Channel.h"
#include "EventLoop.h"

static EventLoop* checkLoop(EventLoop* loop)
{
    if (loop == nullptr)
    {
        LOG_FATAL << "mainLoop is nullpte";
    }
    return loop;
}

TcpConnection::TcpConnection(EventLoop* loop,
                const std::string& name,
                int sockfd,
                const InetAddress& localAddr,
                const InetAddress& peerAddr)
    : _loop(checkLoop(loop)),
      _name(name),
      _state(kConnecting),
      _reading(true),
      _socket(new Socket(sockfd)),
      _channel(new Channel(loop, sockfd)),
      _localAddr(localAddr),
      _peerAddr(peerAddr),
      _highWaterMark(64 * 1024 * 1024)
{
    _channel->setReadCallback(std::bind(&TcpConnection::handleRead, this, std::placeholders::_1));
    _channel->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
    _channel->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
    _channel->setErrorCallback(std::bind(&TcpConnection::handleError, this));

    LOG_INFO << "TcpConnection::ctor[" << _name << "] at " << this
            << " fd=" << sockfd;
    _socket->serKeepAlive(true);
}

TcpConnection::~TcpConnection()
{
    LOG_INFO << "TcoConnection::dtor[" << _name.c_str() << "] at " << this
            << " fd=" << _channel->fd()
            << " state=" << static_cast<int>(_state);
}

void TcpConnection::send(const void* message, int len)
{
    send(std::string(static_cast<const char*>(message)));
}

void TcpConnection::send(const std::string& buf)
{
    if (_state == kConnected)
    {
        if (_loop->isInLoopThread())
        {
            sendInLoop(buf.c_str(), buf.size());
        }
        else
        {
            void(TcpConnection::*fp)(const void* data, size_t len) = &TcpConnection::sendInLoop;
            _loop->runInLoop(std::bind(fp, this, buf.c_str(), buf.size()));
        }
    }
}

void TcpConnection::send(Buffer* buf)
{
    if (_state == kConnected)
    {
        if (_loop->isInLoopThread())
        {
            sendInLoop(buf->peek(), buf->readableBytes());
            buf->rertrieveAll();
        }
        else
        {
            void(TcpConnection::*fp)(const std::string& message) = &TcpConnection::sendInLoop;
            _loop->runInLoop(std::bind(fp, this, buf->retrieveAllAsString()));
        }
    } 
}

void TcpConnection::sendInLoop(const std::string& message)
{
    sendInLoop(message.data(), message.size());
}

void TcpConnection::sendInLoop(const void* data, size_t len)
{
    ssize_t nwrote = 0;
    size_t remaining = len;
    bool faultError = false;

    if (_state == kDisconnected)
    {
        LOG_ERROR << "disconnected, give up writing";
        return;
    }

    if (!_channel->isWriting() && _outputBuffer.readableBytes() == 0)
    {
        nwrote = ::write(_channel->fd(), data, len);
        if (nwrote >= 0)
        {
            remaining = len - nwrote;
            if (remaining == 0 && _writeCompleteCallback)
            {
                _loop->queueInLoop(std::bind(_writeCompleteCallback, shared_from_this()));
            }
        }
        else
        {
            nwrote = 0;
            if (errno != EWOULDBLOCK)
            {
                LOG_ERROR << "TcpConnection::sendInLoop";
                if (errno == EPIPE || errno == ECONNRESET)
                {
                    faultError = false;
                }
            }
        }
    }

    if (!faultError && remaining > 0)
    {
        ssize_t oldLen = _outputBuffer.readableBytes();
        if (oldLen + remaining >= _highWaterMark && oldLen < _highWaterMark && _highWaterMarkCallback)
        {
            _loop->queueInLoop(std::bind(_highWaterMarkCallback, shared_from_this(), oldLen + remaining));
        }
        _outputBuffer.append((char*) data + nwrote, remaining);
        if (!_channel->isWriting())
        {
            _channel->enableWriting();
        }
    }
}

void TcpConnection::shutdown()
{
    if (_state == kConnected)
    {
        setState(kDisconnecting);
        _loop->runInLoop(std::bind(&TcpConnection::shutdownInLoop, this));
    }
}

void TcpConnection::shutdownInLoop()
{
    if (!_channel->isWriting())
    {
        _socket->shutdownWrite();
    }
}

void TcpConnection::forceClose()
{
    if (_state == kConnected || _state == kDisconnecting)
    {
        setState(kDisconnecting);
        _loop->queueInLoop(std::bind(&TcpConnection::forceCloseInLoop, shared_from_this()));
    }
}

void TcpConnection::forceCloseInLoop()
{
    if (_state == kConnected || _state == kDisconnecting)
    {
        handleClose();
    }
}

void TcpConnection::connectEstablished()
{
    setState(kConnected);
    _channel->tie(shared_from_this());
    _channel->enableReading();
}

void TcpConnection::connectDestroyed()
{
    if (_state == kConnected)
    {
        setState(kDisconnected);
        _channel->disableAll();
        _connectionCallback(shared_from_this());
    }
    _channel->remove();
}

void TcpConnection::handleRead(Timestamp receiveTime)
{
    int savedErrno = 0;
    ssize_t n = _inputBuffer.readFd(_channel->fd(), &savedErrno);
    if (n > 0)
    {
        _messageCallback(shared_from_this(), &_inputBuffer, receiveTime);
    }
    else if (n == 0)
    {
        handleClose();
    }
    else
    {
        errno = savedErrno;
        LOG_ERROR << "TcpConnection::handleRead() failed";
        handleError();
    }
}

void TcpConnection::handleWrite()
{
    if (_channel->isWriting())
    {
        int savedErrno = 0;
        ssize_t n = _outputBuffer.writeFd(_channel->fd(), &savedErrno);
        if (n > 0)
        {
            _outputBuffer.retrieve(n);
            if (_outputBuffer.readableBytes() == 0)
            {
                _channel->disableWriting();
                if (_writeCompleteCallback)
                {

                }
                if (_state == kDisconnecting)
                {
                    shutdownInLoop();
                }
            }
        }
        else
        {
            LOG_ERROR << "TcpConnection::handleWrite() failed";
        }
    }
    else
    {
        LOG_ERROR << "TcpConnection fd=" << _channel->fd() << " is down";
    }

}

void TcpConnection::handleClose()
{
    setState(kDisconnected);
    _channel->disableAll();

    TcpConnectionPtr connPtr(shared_from_this());
    _connectionCallback(connPtr);
    _closeCallback(connPtr);
}

void TcpConnection::handleError()
{
    int optval;
    socklen_t optlen = sizeof(optval);
    int err = 0;
    if (::getsockopt(_channel->fd(), SOL_SOCKET, SO_ERROR, &optval, &optlen))
    {
        err = errno;
    }
    else
    {
        err = optval;
    }
    LOG_ERROR << "TcpConnection::handleError name:" << _name.c_str() << " - SO_ERROR:" << err;
}
