#ifndef CHANNEL_H
#define CHANNEL_H

#include <functional>
#include <memory>

#include "noncopyable.h"
#include "Logger.h"
#include "Timestamp.h"

class EventLoop;

// include socket and event such as EPOLLIN and event from poller

class Channel : noncopyable
{
public:
    using EventCallback = std::function<void()>;
    using ReadEventCallback = std::function<void(Timestamp)>;

    Channel(EventLoop* loop, int fd);
    ~Channel();

    void handleEvent(Timestamp receiveTime);

    void setReadCallback(ReadEventCallback cb) { _readCallback = std::move(cb); }
    void setWriteCallback(EventCallback cb) { _writeCallback = std::move(cb); }
    void setCloseCallback(EventCallback cb) { _closeCallback = std::move(cb); }
    void setErrorCallback(EventCallback cb) { _errorCallback = std::move(cb); }

    void enableReading() { _events |= kReadEvent; update(); }
    void disableReading() { _events &= ~kReadEvent; update(); }
    void enableWriting() { _events |= kWriteEvent; update(); }
    void disableWriting() { _events &= ~kWriteEvent; update(); }
    void disableAll() { _events &= kNoneEvent; update(); }

    bool isNoneEvent() const { return _events == kNoneEvent; }
    bool isReading() const { return _events & kReadEvent; }
    bool isWriting() const {return _events & kWriteEvent; }

    // 防止channel被手动remove
    void tie(const std::shared_ptr<void>&);

    int fd() const { return _fd; }
    int events() const { return _events; }
    void set_revents(int revt) { _revents = revt; }

    int index() { return _index; }
    void set_index(int idx) { _index = idx; }
    
    EventLoop* ownerLoop() { return _loop; }
    void remove();

private:
    void update();
    void handleEventWithGuard(Timestamp receiveTime);

    // 感兴趣的状态
    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;

    EventLoop* _loop;
    const int _fd; // 监听对象
    int _events; // 注册fd的事件
    int _revents; // poller返回的事件
    int _index;

    std::weak_ptr<void> _tie;
    bool _tied;

    ReadEventCallback _readCallback;
    EventCallback _writeCallback;
    EventCallback _closeCallback;
    EventCallback _errorCallback;
};


#endif