#pragma once

#include <atomic>
#include <unordered_map>
#include <functional>

#include "TcpConnection.h"
#include "EventLoop.h"
#include "EventLoopThreadPoll.h"
#include "Acceptor.h"
#include "noncopyable.h"
#include "InetAddress.h"
#include "Callback.h"

class TcpServer : noncopyable
{
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;

    enum Option
    {
        kNoReusePort,
        kReusePort,
    };

    TcpServer(EventLoop* loop,
              const InetAddress& listenAddr,
              const std::string& name,
              Option option = kNoReusePort);
    ~TcpServer();

    void setThreadNum(int numThreads); // 设置subloop个数

    std::shared_ptr<EventLoopThreadPoll> threadPool() { return _threadPool; }

    const std::string& ipPort() const { return _ipPort; }
    const std::string& name() const { return _name; }
    EventLoop* getLoop() const { return _loop; }

    // 开始监听
    void start();

    void setConnectionCallback(const ConnectionCallback& cb) { _connectionCallback = cb; }
    void setMessageCallback(const MessageCallback& cb) { _messageCallback = cb; }
    void setWriteCompleteCallback(const WriteCompleteCallback& cb) { _writeCompleteCallback = cb;}
    void setThreadInitCallback(const ThreadInitCallback& cb) { _threadInitCallback = cb; }

private:
    void newConnection(int sockfd, const InetAddress& peerAddr);
    void removeConnection(const TcpConnectionPtr& conn);
    void removeConnectionInLoop(const TcpConnectionPtr& conn);

    using ConnectionMap = std::unordered_map<std::string, TcpConnectionPtr>;

    EventLoop* _loop; // base loop
    const std::string _ipPort;
    const std::string _name;

    std::unique_ptr<Acceptor> _acceptor; // mainloop监听事件
    std::shared_ptr<EventLoopThreadPoll> _threadPool;

    ConnectionCallback _connectionCallback;
    MessageCallback _messageCallback;
    WriteCompleteCallback _writeCompleteCallback;
    
    ThreadInitCallback _threadInitCallback;
    std::atomic_int _started;

    int _nextConnId;
    ConnectionMap _connections;
};
