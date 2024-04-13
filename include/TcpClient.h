#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <mutex>
#include <atomic>

#include "TcpConnection.h"

class Connector;
using ConnectorPtr = std::shared_ptr<Connector>;

class TcpClient : noncopyable
{
public:
    TcpClient(EventLoop* loop, const InetAddress& serverAddr, const std::string& name);
    ~TcpClient();

    void connect();
    void disconnect();
    void stop();

    TcpConnectionPtr connection() const
    {
        std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(_mutex));
        return _connection;
    }
    
    EventLoop* getLoop() const { return _loop; }
    bool retry() const { return _retry; }
    void enableRetry() { _retry = true; }

    const std::string& name() const { return _name; }

    void setConnectionCallback(const ConnectionCallback& cb) { _connectionCallback = cb; }
    void setMessageCallback(const MessageCallback& cb) { _messageCallback = cb; }
    void setWriteCompleteCallback(const WriteCompleteCallback& cb) { _writeCompleteCallback = cb;}

private:
    void newConnection(int sockfd);
    void removeConnection(const TcpConnectionPtr& conn);

    EventLoop* _loop;
    ConnectorPtr _connector;
    const std::string _name;

    ConnectionCallback _connectionCallback;
    MessageCallback _messageCallback;
    WriteCompleteCallback _writeCompleteCallback;

    std::atomic_bool _retry;
    std::atomic_bool _connect;

    int _nextConnId;

    std::mutex _mutex;
    TcpConnectionPtr _connection;
};

#endif