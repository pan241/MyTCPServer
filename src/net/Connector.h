#ifndef CONNECTOR_H
#define CONNECTOR_H

#include <functional>
#include <memory>

#include "Channel.h"
#include "InetAddress.h"
#include "noncopyable.h"
#include "EventLoop.h"

class Connector : noncopyable,
                public std::enable_shared_from_this<Connector>
{
public:
    using NewConnectionCallback = std::function<void(int sockfd)>;

    Connector(EventLoop* loop, const InetAddress& serverAddr);
    ~Connector();

    void setNewConnectionCallback(const NewConnectionCallback& cb) { _newConnectionCallback = cb; }

    void start();
    void restart();
    void stop();

    const InetAddress& serverAddress() const { return _serverAddr; }

private:
    enum States
    {
        kDisconnected,
        kConnecting,
        kConnected
    };
    static const int kMaxRetryDelayMs = 30 * 1000;
    static const int kInitRetryDelayMs = 500;

    void setState(States state) { _state = state; }
    void startInLoop();
    void stopInLoop();
    void connect();
    void connecting(int sockfd);
    void handleWrite();
    void handleError();
    void retry(int sockfd);
    int removeAndResetChannel();
    void resetChannel();

    EventLoop* _loop;
    InetAddress _serverAddr;
    bool _connect;
    States _state;
    std::unique_ptr<Channel> _channel;
    NewConnectionCallback _newConnectionCallback;
    int _retryDelayMs;
};

#endif