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
      _quit(false),
      _callingPendingFunctors(false),
      _threadId(CurrentThread::tid()),
      _poller(Poller::newDefaultPoller(this)),
      _timerQueue(new TimerQueue(this)),
      _wakeupFd(createEventfd()),
      _wakeupChannel(new Channel(this, _wakeupFd)),
      _currentActiveChannel(nullptr)
{


    _wakeupChannel->disableAll();
}


EventLoop::~EventLoop()
{
    _wakeupChannel->disableAll();
    _wakeupChannel->remove();
    ::close(_wakeupFd);
    t_loopInThisThread = nullptr;
}

void EventLoop::loop()
{
    _looping = true;
    _quit = false;

    LOG_INFO << "EventLoop " << this << " start looping";

    while (!_quit)
    {
        _activeChannels.clear();
        _pollReturnTime = _poller->poll(kPollTimeMs, &_activeChannels);
        for (Channel* channel : _activeChannels)
        {
            _currentActiveChannel = channel;
            channel->handleEvent(_pollReturnTime);
        }
        _currentActiveChannel = nullptr;
        doPendingFunctors();
    }
    _looping = false;
}

    
void EventLoop::quit()
{
    _quit = true;

    if (!isInLoopThread())
    {
        wakeup();
    }
}

void EventLoop::runInLoop(Functor cb)
{
    if (isInLoopThread())
    {
        cb();
    }
    else
    {
        queueInLoop(cb);
    }
}

void EventLoop::queueInLoop(Functor cb)
{
    {
        std::unique_lock<std::mutex> lock(_mutex);
        _pendingFunctors.emplace_back(cb);
    }

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
