#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include <atomic>
#include <functional>
#include <vector>
#include <mutex>
#include <memory>

#include "../base/noncopyable.h"
#include "../base/Timestamp.h"
#include "../base/CurrentThread.h"
#include "../timer/TimerQueue.h"

class Channel;
class Poller;

class EventLoop : noncopyable
{
public:
    using Functor = std::function<void()>;

    EventLoop();
    ~EventLoop();

    void loop();
    void quit();

    void runInLoop(Functor cb);
    void queueInLoop(Functor cb);

    void wakeup();

    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);
    bool hasChannel(Channel* channel);

    bool isInLoopThread() const { return _threadId == CurrentThread::tid(); }

    void runAt(Timestamp timestamp, Functor&& cb)
    {
        _timerQueue->addTimer(std::move(cb), timestamp, 0.0);
    }

    void runAfter(double waitTime, Functor&& cb)
    {
        Timestamp time(addTime(Timestamp::now(), waitTime));
        runAt(time, std::move(cb));
    }

    void runEvery(double interval, Functor&& cb)
    {
        Timestamp time(addTime(Timestamp::now(), interval));
        _timerQueue->addTimer(std::move(cb), time, interval);
    }

    Timestamp pollReturnTime() const { return _pollReturnTime; }

private:
    void handleRead();
    void doPendingFunctors();

    using ChannelList = std::vector<Channel*>;

    std::atomic_bool _looping;
    std::atomic_bool _quit;
    std::atomic_bool _callingPendingFunctors;

    const pid_t _threadId; 
    Timestamp _pollReturnTime;
    std::unique_ptr<Poller> _poller;
    std::unique_ptr<TimerQueue> _timerQueue;

    int _wakeupFd;
    std::unique_ptr<Channel> _wakeupChannel;

    ChannelList _activeChannels;
    Channel* _currentActiveChannel;

    std::mutex _mutex;
    std::vector<Functor> _pendingFunctors;
};


#endif