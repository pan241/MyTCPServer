#ifndef TIMERQUEUE_H
#define TIMERQUEUE_H

#include <vector>
#include <set>

#include "Timestamp.h"
#include "Channel.h"

class EventLoop;
class Timer;

class TimerQueue
{
public:
    using TimerCallback = std::function<void()>;

    explicit TimerQueue(EventLoop* loop);
    ~TimerQueue();

    void addTimer(TimerCallback cb, Timestamp when, double interval);
private:
    using Entry = std::pair<Timestamp, Timer*>;
    using TimerList = std::set<Entry>;

    void addTimerInLoop(Timer* timer);

    void resetTimerfd(int _timerfd, Timestamp expiration);

    std::vector<Entry> getExpired(Timestamp now);
    void reset(const std::vector<Entry>& expired, Timestamp now);

    void handleRead();

    bool insert(Timer* timer); 

    EventLoop* _loop;
    const int _timerfd;
    Channel _timerfdChannel;
    TimerList _timers;
    bool _callingExpiredTimers;
};

#endif