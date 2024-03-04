#ifndef CHANNEL_H
#define CHANNEL_H

#include <functional>
#include <memory>
#include <sys/epoll.h>

#include "../base/noncopyable.h"
#include "../base/Timestamp.h"
#include "../log/Logging.h"

class EventLoop;

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

    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;

    EventLoop* _loop;
    const int _fd;
    int _events;
    int _revents;
    int _index;

    std::weak_ptr<void> _tie;
    bool _tied;

    ReadEventCallback _readCallback;
    EventCallback _writeCallback;
    EventCallback _closeCallback;
    EventCallback _errorCallback;
};


#endif