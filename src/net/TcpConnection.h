#ifndef TCPCONNECTION_H
#define TCPCONNECTION_H

#include <memory>
#include <string>
#include <atomic>

#include "Callback.h"
#include "noncopyable.h"
#include "Timestamp.h"
#include "InetAddress.h"
#include "Buffer.h"

class Channel;
class EventLoop;
class Socket;

class TcpConnection : noncopyable,
                    public std::enable_shared_from_this<TcpConnection>
{
public:
    TcpConnection(EventLoop* loop,
                const std::string& name,
                int sockfd,
                const InetAddress& localAddr,
                const InetAddress& peerAddr);
    ~TcpConnection();

    EventLoop* getLoop() const { return _loop; }
    const std::string name() const { return _name; }
    const InetAddress& localAddress() const { return _localAddr; }
    const InetAddress& peerAddress() const { return _peerAddr; }

    bool connected() const { return _state == kConnected; }
    bool disconnected() const { return _state == kDisconnected; }

    void send(const void* message, int len);
    void send(const std::string& buf);
    void send(Buffer* buf);

    void shutdown();

    void forceClose();

    void setConnectionCallback(const ConnectionCallback& cb) { _connectionCallback = cb; }
    void setMessageCallback(const MessageCallback& cb) { _messageCallback = cb; }
    void setCloseCallback(const CloseCallback& cb) { _closeCallback = cb; }
    void setWriteCompleteCallback(const WriteCompleteCallback& cb) { _writeCompleteCallback = cb;}
    void setHighWaterMarkCallback(const HighWaterMarkCallback& cb, size_t highWaterMark) 
    { _highWaterMarkCallback = cb; _highWaterMark = highWaterMark; }

    Buffer* inputBuffer() { return &_inputBuffer; }
    Buffer* outputBuffer() {return &_outputBuffer; }

    void connectEstablished();
    void connectDestroyed();

private:
    enum StateE
    {
        kDisconnected,
        kConnecting,
        kConnected,
        kDisconnecting
    };
    void setState(StateE state) { _state = state; }

    void handleRead(Timestamp receiveTime);
    void handleWrite();
    void handleClose();
    void handleError();

    void sendInLoop(const void* message, size_t len);
    void sendInLoop(const std::string& message);
    void shutdownInLoop();
    void forceCloseInLoop();

    EventLoop* _loop;
    const std::string _name;
    std::atomic_int _state;
    bool _reading;

    std::unique_ptr<Socket> _socket;
    std::unique_ptr<Channel> _channel;

    const InetAddress _localAddr;
    const InetAddress _peerAddr;

    ConnectionCallback _connectionCallback;
    MessageCallback _messageCallback;
    CloseCallback _closeCallback;
    WriteCompleteCallback _writeCompleteCallback;
    HighWaterMarkCallback _highWaterMarkCallback;
    size_t _highWaterMark;

    Buffer _inputBuffer;
    Buffer _outputBuffer;
};

#endif