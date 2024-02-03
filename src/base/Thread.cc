#include "Thread.h"
#include "CurrentThread.h"
#include <assert.h>
#include <semaphore.h>

std::atomic_int32_t Thread::_numCreated(0);

Thread::Thread(ThreadFunc func, const std::string &name = std::string()) :
    _started(false),
    _joined(false),
    _tid(0),
    _func(std::move(func)),
    _name(name)
{
    setDefaultName();
}

Thread::~Thread()
{
    if (_started && !_joined)
    {
       _thread->detach(); 
    }
}

void Thread::setDefaultName()
{
    int num = ++_numCreated;
    if (_name.empty())
    {
        char buf[32] = {0};
        snprintf(buf, sizeof buf, "Thread%d", num);
        _name = buf;
    }
}

void Thread::start()
{
    assert(!_started);
    _started = true;
    sem_t sem;
    sem_init(&sem, false, 0);
    _thread = std::shared_ptr<std::thread>(new std::thread([&](){
        _tid = CurrentThread::tid();
        sem_post(&sem);
        _func();
    }));
    sem_wait(&sem);
}

void Thread::join()
{
    assert(_started);
    assert(!_joined);
    _joined = true;
    _thread->join();
}

bool CurrentThread::isMainThread()
{
    return tid() == ::gettid();
}