#include "TcpClient.h"

#include <stdio.h>

#include "Connector.h"
#include "Logger.h"
#include "EventLoop.h"
#include "SocketOperation.h"

namespace detail
{
void removeConnection(EventLoop* loop, const TcpConnectionPtr& conn)
{
    loop->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
}

void removeConnector(const ConnectorPtr& connector)
{
    
}

};

static EventLoop* checkNotNull(EventLoop* loop)
{
    if (loop == nullptr)
    {
        LOG_FATAL << "Loop is null";
    }
    return loop;
}

TcpClient::TcpClient(EventLoop* loop, const InetAddress& serverAddr, const std::string& name)
    : _loop(checkNotNull(loop)),
      _connector(new Connector(loop, serverAddr)),
      _name(name),
      _connectionCallback(),
      _messageCallback(),
      _retry(false),
      _connect(true),
      _nextConnId(1)
{
    _connector->setNewConnectionCallback(std::bind(&TcpClient::newConnection, this, std::placeholders::_1));
    LOG_INFO << "TcpClient::TcpClient[" << _name << "] - connector" << _connector.get();
    
}

TcpClient::~TcpClient()
{
    LOG_INFO << "TcpClient::~TcpClient[" << _name << "] - connector" << _connector.get();
    TcpConnectionPtr conn;
    bool unique = false;
    {
        std::lock_guard<std::mutex> lock(_mutex);
        unique = _connection.unique();
        conn = _connection;
    }

    if (conn)
    {
        CloseCallback cb = std::bind(&detail::removeConnection, _loop, std::placeholders::_1);
        _loop->runInLoop(std::bind(&TcpConnection::setCloseCallback, conn, cb));
        if (unique)
        {
            // conn->forceClose();
        }
    }
    else
    {
        _connector->stop();
        _loop->runAfter(1, std::bind(&detail::removeConnector, _connector));
    }
}

void TcpClient::connect()
{
    LOG_INFO << "TcpClient::connect[" << _name << "] - connecting to " << _connector->serverAddress().toIpPort();
    _connect = true;
    _connector->start();
}

void TcpClient::disconnect()
{
    _connect = false;

    {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_connection)
        {
            _connection->shutdown();
        }
    }
}

void TcpClient::stop()
{
    _connect = false;
    _connector->stop();
}

void TcpClient::newConnection(int sockfd)
{
    auto addr = sockets::getPeerAddr(sockfd);
    InetAddress peerAddr(*(sockaddr_in*)&addr);
    char buf[32];
    snprintf(buf, sizeof(buf), ":%s#%d", peerAddr.toIpPort().c_str(), _nextConnId);
    ++_nextConnId;
    std::string connName = _name + buf;

    addr = sockets::getLocalAddr(sockfd);
    InetAddress localAddr(*(sockaddr_in*)&addr);

    TcpConnectionPtr conn(new TcpConnection(_loop, connName, sockfd, localAddr, peerAddr));
    conn->setConnectionCallback(_connectionCallback);
    conn->setMessageCallback(_messageCallback);
    conn->setWriteCompleteCallback(_writeCompleteCallback);
    conn->setCloseCallback(std::bind(&TcpClient::removeConnection, this, std::placeholders::_1));
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _connection = conn;
    }
    conn->connectEstablished();
}

void TcpClient::removeConnection(const TcpConnectionPtr& conn)
{
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _connection.reset();
    }

    _loop->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
    if (_retry && _connect)
    {
        LOG_INFO << "TcpClient::connect[" << _name << "] - Reconnectiong to " << _connector->serverAddress().toIpPort();
        _connector->restart();
    } 
}
