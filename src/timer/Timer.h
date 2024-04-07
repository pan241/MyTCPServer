#ifndef TIMER_H
#define TIMER_H

#include <functional>

#include "noncopyable.h"
#include "Timestamp.h"

class Timer : noncopyable
{
public:
    using TimerCallback = std::function<void()>;

    Timer(TimerCallback cb, Timestamp when, double interval)
        : _callback(std::move(cb)),
         _expiration(when),
         _interval(interval),
         _repeat(interval > 0.0)
    {
    }

    void run() const
    {
        _callback();
    }

    Timestamp expiration() const { return _expiration; }
    bool repeat() const { return _repeat; }

    void restart(Timestamp now);

private: 
    const TimerCallback _callback;
    Timestamp _expiration;
    const double _interval;
    const bool _repeat;   
};

#endif