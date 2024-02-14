#include "AsyncLogging.h"
#include "../base/Timestamp.h"

#include <stdio.h>

AsyncLogging::AsyncLogging(const std::string& basename, off_t rollsize, int flushInterval)
    : _basename(basename),
      _rollSize(rollsize),
      _flushInterval(flushInterval),
      _running(false),
      _thread(std::bind(&AsyncLogging::ThreadFunc, this), "Logging"),
      _mutex(),
      _cond(),
      _currentBuffer(new Buffer),
      _nextBuffer(new Buffer),
      _buffers()
{
    _currentBuffer->bzero();
    _nextBuffer->bzero();
    _buffers.reserve(16);
}

void AsyncLogging::append(const char* logline, int len)
{
    std::lock_guard<std::mutex> lock(_mutex);
    if (_currentBuffer->avail() > len)
    {
        _currentBuffer->append(logline, len);
    }
    else
    {
        _buffers.push_back(std::move(_currentBuffer));
        if (_nextBuffer)
        {
            _currentBuffer = std::move(_nextBuffer);
        }
        else
        {
            _currentBuffer.reset(new Buffer);
        }
        _currentBuffer->append(logline, len);
        _cond.notify_one();
    }
}

void AsyncLogging::ThreadFunc()
{
    LogFile output(_basename, _rollSize, false);
    BufferPtr newBuffer1(new Buffer);
    BufferPtr newBuffer2(new Buffer);
    newBuffer1->bzero();
    newBuffer2->bzero();
    BufferVector buffersToWrite;
    while(_running)
    {
        std::unique_lock<std::mutex> lock(_mutex);
        if (_buffers.empty())
        {
            _cond.wait_for(lock, std::chrono::seconds(_flushInterval));
        }
        _buffers.push_back(std::move(_currentBuffer));
        _currentBuffer = std::move(newBuffer1);
        buffersToWrite.swap(_buffers);
        if (!_nextBuffer)
        {
            _nextBuffer = std::move(newBuffer2);
        }

        for (const auto& buffer : buffersToWrite)
        {
            output.append(buffer->data(), buffer->length());
        }
        if (buffersToWrite.size() > 2)
        {
            buffersToWrite.resize(2);
        }
        if (!newBuffer1)
        {
            newBuffer1 = std::move(buffersToWrite.back());
            buffersToWrite.pop_back();
            newBuffer1->reset();
        }
        if (!newBuffer2)
        {
            newBuffer2 = std::move(buffersToWrite.back());
            buffersToWrite.pop_back();
            newBuffer2->reset();
        }
        buffersToWrite.clear();
        output.flush();
    }
    
    output.flush();
}
