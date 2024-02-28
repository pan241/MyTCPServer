#include "EventLoopThread.h"
#include "EventLoop.h"

EventLoopThread::EventLoopThread(const ThreadInitCallback& cb = ThreadInitCallback(), const std::string& name = std::string())
    : _loop(nullptr),
      _existing(false),
      _thread(std::bind(&EventLoopThread::threadFunc, this), name),
      _mutex(),
      _cond(),
      _callback(cb)
{
}

EventLoopThread::~EventLoopThread()
{
    _existing = true;
    if (_loop != nullptr)
    {
        _loop->quit();
        _thread.join();
    }
}

EventLoop* EventLoopThread::startLoop()
{
    _thread.start();

    EventLoop* loop = nullptr;
    {
        std::unique_lock<std::mutex> lock(_mutex);
        while (_loop == nullptr)
        {
            _cond.wait(lock);
        }
        loop = _loop;
    }

    return loop;
}

void EventLoopThread::threadFunc()
{
    EventLoop loop;

    if (_callback)
    {
        _callback(&loop);
    }

    {
        std::unique_lock<std::mutex> lock(_mutex);
        _loop = &loop;
        _cond.notify_one();
    }

    _loop->loop();
    std::unique_lock<std::mutex> lock(_mutex);
    _loop = nullptr;
}