#include "TimerQueue.h"
#include "Timer.h"
#include "Channel.h"
#include "Logger.h"
#include "EventLoop.h"

#include <unistd.h>
#include <sys/timerfd.h>
#include <string.h>
#include <algorithm>

int createTimerfd()
{
    int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);

    if (timerfd < 0)
    {
        LOG_ERROR << "Failed in timerfd_create";
    }
    return timerfd;
}

TimerQueue::TimerQueue(EventLoop* loop)
    : _loop(loop),
      _timerfd(createTimerfd()),
      _timerfdChannel(loop, _timerfd),
      _timers()
{
    _timerfdChannel.setReadCallback(std::bind(&TimerQueue::handleRead, this));
    _timerfdChannel.enableReading();
}
   
TimerQueue::~TimerQueue()
{
    _timerfdChannel.disableAll();
    _timerfdChannel.remove();
    ::close(_timerfd);
    for (const Entry& timer : _timers)
    {
        delete timer.second;
    }
}

void TimerQueue::addTimer(TimerCallback cb, Timestamp when, double interval)
{
    Timer* timer = new Timer(std::move(cb), when, interval);
    _loop->runInLoop(std::bind(&TimerQueue::addTimerInLoop, this, timer));
}

void TimerQueue::addTimerInLoop(Timer* timer)
{
    bool earliestChanged = insert(timer);

    if (earliestChanged)
    {
        resetTimerfd(_timerfd, timer->expiration());
    }
}

void readTimerfd(int timerfd)
{
    uint64_t read_byte;
    ssize_t n = ::read(timerfd, &read_byte, sizeof(read_byte));

    if (n != sizeof(read_byte))
    {
        LOG_ERROR << "TimerQueue::handleRead() reads " << n << " bytes";
    }
}

void  TimerQueue::resetTimerfd(int _timerfd, Timestamp expiration)
{
    struct itimerspec newValue;
    struct itimerspec oldValue;
    memset(&newValue, '\0', sizeof(newValue));

    int64_t microSecond = expiration.microSecondSinceEpoch() - Timestamp::now().microSecondSinceEpoch();
    microSecond = std::max(int64_t(100), microSecond);
    
    struct timespec ts;
    ts.tv_sec = static_cast<time_t>(microSecond / Timestamp::kMicroSecondsPerSecond);
    ts.tv_nsec = static_cast<time_t>(microSecond % Timestamp::kMicroSecondsPerSecond) * 1000;
    newValue.it_value = ts;
    if (::timerfd_settime(_timerfd, 0, &newValue, &oldValue))
    {
        LOG_ERROR << "timerfd_settime failed";
    }
}

void TimerQueue::reset(const std::vector<Entry>& expired, Timestamp now)
{
    Timestamp nextExpire;

    for (const Entry& it : expired)
    {
        if (it.second->repeat())
        {
            auto timer = it.second;
            timer->restart(Timestamp::now());
            insert(timer);
        }
        else
        {
            delete it.second;
        }
    }

    if (!_timers.empty())
    {
        resetTimerfd(_timerfd, (_timers.begin()->second)->expiration());
    }
}

void TimerQueue::handleRead()
{
    Timestamp now = Timestamp::now();
    readTimerfd(_timerfd);

    std::vector<Entry> expired = getExpired(now);

    _callingExpiredTimers = true;
    for (const Entry& it : expired)
    {
        it.second->run();
    }
    _callingExpiredTimers = false;
    
    reset(expired, now);
}

std::vector<TimerQueue::Entry> TimerQueue::getExpired(Timestamp now)
{
    std::vector<Entry> expired;
    Entry sentry(now, reinterpret_cast<Timer*>(UINTPTR_MAX));
    TimerList::iterator end = _timers.lower_bound(sentry);
    std::copy(_timers.begin(), end, back_inserter(expired));
    _timers.erase(_timers.begin(), end);

    return expired;
}

bool TimerQueue::insert(Timer* timer)
{
    bool earliestChanged = false;
    Timestamp when = timer->expiration();
    TimerList::iterator it = _timers.begin();
    if (it == _timers.end() || when < it->first)
    {
        earliestChanged = true;
    }
    _timers.insert(Entry(when, timer));

    return earliestChanged;
}
