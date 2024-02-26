#include "Timer.h"

void Timer::restart(Timestamp now)
{
    if (_repeat)
    {
        _expiration = addTime(now, _interval);
    }
    else
    {
        _expiration = Timestamp();
    }
}