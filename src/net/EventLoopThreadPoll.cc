#include "EventLoopThreadPoll.h"
#include "EventLoop.h"
#include "EventLoopThread.h"

EventLoopThreadPoll::EventLoopThreadPoll(EventLoop* baseLoop, const std::string& name)
    : _baseLoop(baseLoop),
      _name(name),
      _started(false),
      _numThreads(0),
      _next(0)
{
}

EventLoopThreadPoll:: ~EventLoopThreadPoll()
{   
}

void EventLoopThreadPoll::start(const ThreadInitCallback& cb = ThreadInitCallback())
{
    _started = true;

    for (int i = 0; i < _numThreads; i++)
    {
        char buf[_name.size() + 32];
        snprintf(buf, sizeof(buf), "%s%d", _name.c_str(), i);
        EventLoopThread* t = new EventLoopThread(cb, buf);
        _threads.push_back(std::unique_ptr<EventLoopThread>(t));
        _loops.push_back(t->startLoop());
    }

    if (_numThreads == 0 && cb)
    {
        cb(_baseLoop);
    }
}

EventLoop* EventLoopThreadPoll::getNextLoop()
{
    EventLoop* loop = _baseLoop;
    if (!_loops.empty())
    {
        loop = _loops[_next];
        _next = _next;
        if (_next >= _loops.size())
        {
            _next = 0;
        }
    }
    return loop;
}

std::vector<EventLoop*> EventLoopThreadPoll::getAllLoops()
{
    if (_loops.empty())
    {
        return std::vector<EventLoop*>(1, _baseLoop);
    }
    else
    {
        return _loops;
    }
}