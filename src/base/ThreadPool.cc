#include "ThreadPool.h"
#include "Logger.h"

ThreadPool::ThreadPool(const std::string& name)
    : _mutex(), 
      _cond(),
      _name(name),
      _running(false)
{
}

ThreadPool::~ThreadPool()
{
    if (_running)
    {
        stop();
    }
}

void ThreadPool::start()
{
    _running = true;
    _threads.reserve(_threadSize);
    for (int i = 0; i < _threadSize; i++)
    {
        char id[32];
        snprintf(id, sizeof(id), "%d", i + 1);
        _threads.emplace_back(new Thread(
            std::bind(&ThreadPool::runInThread, this), _name + id));
        _threads[i]->start();
    }
    if (_threadSize == 0 && _threadInitCallback)
    {
        _threadInitCallback();
    }
}

void ThreadPool::stop()
{
    std::lock_guard<std::mutex> lock(_mutex);
    _running = false;
    _cond.notify_all();

    for (const auto& t : _threads)
    {
        t->join();
    }
}

size_t ThreadPool::queueSize() const
{
    std::lock_guard<std::mutex> lock(_mutex);
    return _queue.size();
}

void ThreadPool::add(ThreadFunc f)
{
    if (_threads.empty())
    {
        f();
    }
    else
    {
        std::unique_lock<std::mutex> lock(_mutex);
        _queue.push_back(std::move(f));
        _cond.notify_one();
    }
}

bool  ThreadPool::isFull() const
{
    return _threadSize > 0 && _queue.size() >= _threadSize;
}

void ThreadPool::runInThread()
{
    try
    {
        if (_threadInitCallback)
        {
            _threadInitCallback();
        }
        ThreadFunc task;
        while (true)
        {
            std::unique_lock<std::mutex> lock(_mutex);
            while (_queue.empty())
            {
                if (!_running)
                {
                    return;
                }
                _cond.wait(lock);
            }
            task = _queue.front();
            _queue.pop_front();
        }
        if (task)
        {
            task();
        }
    }
    catch(...)
    {
        LOG_WARN << "runInThread throw exception";
    }
}