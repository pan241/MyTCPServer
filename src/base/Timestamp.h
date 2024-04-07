#pragma once

#include <string>
#include <sys/time.h>

class Timestamp
{
public:
    Timestamp() :
        _microSecondsSinceEpoch(0) {}

    explicit Timestamp(int64_t microSecondsSinceEpoch) :
        _microSecondsSinceEpoch(microSecondsSinceEpoch) {}

    void swap(Timestamp& that)
    {
        std::swap(_microSecondsSinceEpoch, that._microSecondsSinceEpoch);
    }
    
    std::string toString() const;
    std::string toFormattedString(bool showMicroseconds = false) const;

    bool valid() const { return _microSecondsSinceEpoch > 0; }

    int64_t microSecondSinceEpoch() const { return _microSecondsSinceEpoch; }
    time_t secondSinceEpoch() const
    {
        return static_cast<time_t>(_microSecondsSinceEpoch / kMicroSecondsPerSecond);
    }

    static Timestamp now();
    static Timestamp invalid()
    {
        return Timestamp();
    }


    static const int kMicroSecondsPerSecond = 1000 * 1000;
private:
    int64_t _microSecondsSinceEpoch;
};

inline bool operator<(Timestamp lhs, Timestamp rhs)
{
    return lhs.microSecondSinceEpoch() < rhs.microSecondSinceEpoch();
}

inline bool operator==(Timestamp lhs, Timestamp rhs)
{
    return lhs.microSecondSinceEpoch() == rhs.microSecondSinceEpoch();
}

inline Timestamp addTime(Timestamp timestamp, double seconds)
{
    int64_t delta = static_cast<int64_t>(seconds * Timestamp::kMicroSecondsPerSecond);
    return Timestamp(timestamp.microSecondSinceEpoch() + delta);
}
