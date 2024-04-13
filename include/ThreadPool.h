#ifndef THREADPOOL_H
#define THREADPOOL_H

#include "noncopyable.h"
#include "Thread.h"

#include <mutex>
#include <condition_variable>
#include <deque>
#include <vector>

class ThreadPool : noncopyable
{
public:
    using ThreadFunc = std::function<void()>;

    explicit ThreadPool(const std::string& name = std::string("ThreadPool"));
    ~ThreadPool();

    void setThreadSize(const int& num) { _threadSize = num; }
    void setThreadInitFunc(const ThreadFunc& cb) { _threadInitCallback = cb; }
    void start();
    void stop();

    const std::string& name() const { return _name; }
    size_t queueSize() const;

    void add(ThreadFunc f);
private:
    bool isFull() const;
    void runInThread();

    mutable std::mutex _mutex;
    std::condition_variable _cond;
    std::string _name;
    ThreadFunc _threadInitCallback;
    std::vector<std::unique_ptr<Thread>> _threads;
    std::deque<ThreadFunc> _queue;
    size_t _threadSize;
    bool _running;

};


#endif