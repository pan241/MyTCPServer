#ifndef LOGFILE_H
#define LOGFILE_H

#include <string>
#include <memory>
#include <mutex>
#include "FileUtil.h"

class LogFile
{
public:
    LogFile(const std::string& basename, 
            off_t rollSize, 
            bool threadSafe, 
            int flushInterval, 
            int checkEveryN);
    ~LogFile();

    void append(const char* data, int len);
    void flush();
    bool rollFile();

private:
    static std::string getLogFileName(const std::string& basename, time_t* now);
    void append_unlocked(const char* date, int len);

    const std::string _basename;
    const off_t _rollsize;
    const int _flushInterval;
    const int _checkEveryN;

    int _count;

    std::unique_ptr<std::mutex> _mutex;
    time_t _startOfPeriod;
    time_t _lastRoll;
    time_t _lastFlush;
    std::unique_ptr<AppendFile> _file;

    const static int _kRollPerSeconds = 60 * 60 * 24;
};

#endif