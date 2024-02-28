#include "EventLoop.h"
#include "../log/Logging.h"
#include "Poller.h"

#include <unistd.h>
#include <sys/eventfd.h>


__thread EventLoop *t_loopInThisThread = nullptr;

const int kPollTimeMs = 10000;

int createEventfd()
{
    int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evtfd < 0)
    {
        LOG_FATAL << "eventfd error: " << errno;
    }
    return evtfd;
}

EventLoop::EventLoop()
    : _looping(false),
      _quit(false)
{}


EventLoop::~EventLoop()
{

}

void EventLoop::loop()
{
    _looping = true;

}

    
void EventLoop::quit()
{

}

void EventLoop::runInLoop(Functor cb)
{
    std::unique_lock<std::mutex> lock(_mutex);
    _pendingFunctors.push_back(std::move(cb));

    if (!isInLoopThread() || _callingPendingFunctors)
    {
        wakeup();
    }
}
    

void EventLoop::updateChannel(Channel* channel)
{
    _poller->updateChannel(channel);
}

void EventLoop::removeChannel(Channel* channel)
{
    _poller->removeChannel(channel);
}

bool EventLoop::hasChannel(Channel* channel)
{
    return _poller->hasChannel(channel);
}

void EventLoop::wakeup()
{
    uint64_t one = 1;
    ssize_t n = ::write(_wakeupFd, &one, sizeof(one));
    if (n != sizeof(one))
    {
        LOG_ERROR << "EventLoop::wakeup() write " << n << " bytes instead of 8";
    }
}

void EventLoop::handleRead()
{
    uint64_t one = 1;
    ssize_t n = ::read(_wakeupFd, &one, sizeof(one));
    if (n != sizeof(one))
    {
        LOG_ERROR << "EventLoop::handleRead() reads " << n << " bytes instead of 8";
    }
}

void EventLoop::doPendingFunctors()
{
    std::vector<Functor> functors;
    _callingPendingFunctors = true;
    
    {
        std::unique_lock<std::mutex> lock(_mutex);
        functors.swap(_pendingFunctors);
    }

    for (const Functor& functor : functors)
    {
        functor();
    }

    _callingPendingFunctors = false;
}
