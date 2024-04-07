#include "LogFile.h"

LogFile::LogFile(const std::string& basename, 
                 off_t rollSize, 
                 bool threadSafe = true, 
                 int flushInterval = 3, 
                 int checkEveryN = 1024) :
        _basename(basename),
        _rollsize(rollSize),
        _flushInterval(flushInterval),
        _checkEveryN(checkEveryN),
        _count(0),
        _mutex(threadSafe ? new std::mutex : NULL),
        _startOfPeriod(0),
        _lastRoll(0),
        _lastFlush(0)
{
    rollFile();
}

LogFile::~LogFile() = default;

void LogFile::append(const char* data, int len)
{
    if (_mutex)
    {
        std::lock_guard<std::mutex> lock(*_mutex);
        append_unlocked(data, len);
    }
    else
    {
        append_unlocked(data, len);
    }
}

void LogFile::flush()
{
    if (_mutex)
    {
        std::lock_guard<std::mutex> lock(*_mutex);
        _file->flush();
    }
}

void LogFile::append_unlocked(const char* data, int len)
{
    _file->append(data, len);
}

bool LogFile::rollFile()
{
    time_t now = 0;
    std::string filename = getLogFileName(_basename, &now);

    time_t start = now / _kRollPerSeconds * _kRollPerSeconds;

    if (now > _lastRoll)
    {
        _lastRoll = now;
        _lastFlush = now;
        _startOfPeriod = start;
        _file.reset(new AppendFile(filename));
        return true;
    }
    return false;
}

std::string LogFile::getLogFileName(const std::string& basename, time_t* now)
{
    std::string filename;
    filename.reserve(basename.size() + 64);
    filename = basename;

    char timebuf[32];
    struct tm tm;
    *now = time(NULL);
    gmtime_r(now, &tm);
    strftime(timebuf, sizeof(timebuf), ".%Y%m%d-%H%M%S", &tm);
    filename += timebuf;
    
    filename += ".log";

    return filename;
}