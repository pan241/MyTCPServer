#pragma once

#include <thread>
#include <memory>
#include <functional>
#include <string>
#include <atomic>

#include "noncopyable.h"

class Thread : noncopyable
{
public:
    using ThreadFunc = std::function<void()>;

    explicit Thread(ThreadFunc, const std::string &name);
    ~Thread();

    void start();
    void join();

    bool started() const { return _started;}
    pid_t tid() const { return _tid; }
    const std::string& name() const {return _name;}

    static int numCreated() { return _numCreated; }

private:
    void setDefaultName();

    bool _started;
    bool _joined;
    std::shared_ptr<std::thread> _thread;
    pid_t _tid;
    ThreadFunc _func;
    std::string _name;

    static std::atomic_int32_t _numCreated;
};
