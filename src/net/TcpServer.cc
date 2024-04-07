#include "TcpServer.h"
#include "Logger.h"

static EventLoop* checkNotNull(EventLoop* loop)
{
    if (loop == nullptr)
    {
        LOG_FATAL << "Loop is null";
    }
    return loop;
}

TcpServer::TcpServer(EventLoop* loop,
              const InetAddress& listenAddr,
              const std::string& name,
              Option option)
    : _loop(checkNotNull(loop)),
      _ipPort(listenAddr.toIpPort()),
      _name(name),
      _acceptor(new Acceptor(loop, listenAddr, option == kReusePort)),
      _threadPool(new EventLoopThreadPoll(loop, _name)),
      _connectionCallback(),
      _messageCallback(),
      _threadInitCallback(),
      _started(0),
      _nextConnId(1)
{
    _acceptor->setNewConnectionCallback(std::bind(&TcpServer::newConnection, this, std::placeholders::_1,std::placeholders::_2));
}

TcpServer::~TcpServer()
{
    for (auto& item : _connections)
    {
        TcpConnectionPtr conn(item.second);
        item.second.reset();
        conn->getLoop()->runInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
    }
}

void TcpServer::setThreadNum(int numThreads)
{
    _threadPool->setThreadNum(numThreads);
}

void TcpServer::start()
{
    if (_started == 0)
    {
        _threadPool->start(_threadInitCallback);
        _loop->runInLoop(std::bind(&Acceptor::listen, _acceptor.get()));
    }
    _started++;
}

// 新客户端连接会调用该回调
void TcpServer::newConnection(int sockfd, const InetAddress& peerAddr)
{
    EventLoop* ioLoop = _threadPool->getNextLoop(); // 轮询选择subloop
    char buf[64];
    snprintf(buf, sizeof(buf), "-%s#%d", _ipPort.c_str(), _nextConnId);
    ++_nextConnId;
    std::string connName = _name + buf;

    sockaddr_in local;
    ::memset(&local, 0 ,sizeof(local));
    socklen_t addrlen = sizeof(local);
    if (::getsockname(sockfd, (sockaddr*)&local, &addrlen) < 0)
    {
        LOG_ERROR << "sockets::getLocalAddr() failed";
    }
    InetAddress localAddr(local);

    TcpConnectionPtr conn(new TcpConnection(ioLoop, connName, sockfd, localAddr, peerAddr));
    _connections[connName] = conn;
    // 用户设置TcpServer->TcpConnection->Channel->Poller->Channel
    conn->setConnectionCallback(_connectionCallback);
    conn->setMessageCallback(_messageCallback);
    conn->setWriteCompleteCallback(_writeCompleteCallback);
    // 如何关闭连接
    conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));
    ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));
}

void TcpServer::removeConnection(const TcpConnectionPtr& conn)
{
    _loop->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& conn)
{
    LOG_INFO << "TcpServer::removeConnectionInLoop [" << _name.c_str() 
        <<  "] - connection " << conn->name(); 
    
    size_t n = _connections.erase(conn->name());
    EventLoop* ioLoop = conn->getLoop();
    ioLoop->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
}