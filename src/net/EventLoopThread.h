#pragma once

#include <mutex>
#include <condition_variable>
#include "Thread.h"

class EventLoop;

class EventLoopThread : noncopyable
{
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;

    EventLoopThread(const ThreadInitCallback& cb = ThreadInitCallback(), const std::string& name = std::string());
    ~EventLoopThread();

    EventLoop* startLoop();

private:
    void threadFunc();

    EventLoop* _loop;
    bool _existing;
    Thread _thread;
    std::mutex _mutex;
    std::condition_variable _cond;
    ThreadInitCallback _callback;
};
