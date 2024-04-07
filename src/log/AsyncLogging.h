#ifndef ASYNCLOGGING_H
#define ASYNCLOGGING_H

#include "Thread.h"
#include "LogStream.h"
#include "LogFile.h"
#include <atomic>
#include <vector>
#include <mutex>
#include <condition_variable>

class AsyncLogging : noncopyable
{
public:
    AsyncLogging(const std::string& basename, off_t rollsize, int flushInterval = 3);
    ~AsyncLogging()
    {
        if (_running)
        {
            stop();
        }
    }

    void append(const char* logline, int len);

    void start() {
        _running = true;
        _thread.start();
    }

    void stop() {
        _running = false;
        _cond.notify_one();
        _thread.join();
    }

private:
    using Buffer = FixedBuffer<kLargeBuffer>;
    using BufferVector = std::vector<std::unique_ptr<Buffer>>;
    using BufferPtr = BufferVector::value_type;

    void ThreadFunc();

    const int _flushInterval;
    std::atomic<bool> _running;
    const std::string _basename;
    const off_t _rollSize;
    Thread _thread;
    std::mutex _mutex;
    std::condition_variable _cond;
    
    BufferPtr _currentBuffer;
    BufferPtr _nextBuffer;
    BufferVector _buffers;
};

#endif