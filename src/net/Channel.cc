#include "Channel.h"
#include "EventLoop.h"


const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = EPOLLIN | EPOLLPRI;
const int Channel::kWriteEvent = EPOLLOUT;

Channel::Channel(Eventloop* loop, int fd) 
    : _loop(loop),
      _fd(fd),
      _events(0),
      _revents(0),
      _index(-1),
      _tied(false)
{
}

Channel::~Channel()
{
}

void Channel::tie(const std::shared_ptr<void>& obj)
{
    _tie = obj;
    _tied = true;
}

void Channel::update()
{
    _loop->updateChannel(this);
}

void Channel::handleEvent(Timestamp receiveTime)
{
    if (_tied)
    {
        std::shared_ptr<void> guard = _tie.lock();
        if (guard)
        {
            handleEventWithGuard(receiveTime);
        }
        else
        {
            handleEventWithGuard(receiveTime);
        }
    }
}

void Channel::handleEventWithGuard(Timestamp receiveTime)
{
    if ((_revents & EPOLLHUP) && (_revents & EPOLLIN))
    {
        if (_closeCallback)
        {
            _closeCallback();
        }
    }

    if (_revents & EPOLLERR)
    {
        LOG_ERROR << "the fd = " << this->_fd;
        if (_errorCallback)
        {
            _errorCallback();
        }
    }

    if (_revents & (EPOLLIN | EPOLLPRI))
    {
        //LOG_DEBUG << "channel has read events, fd is " << this->_fd;
        if (_readCallback)
        {
            _readCallback(receiveTime);
        }
    }

    if (_revents & EPOLLOUT)
    {
        if (_writeCallback)
        {
            _writeCallback();
        }
    }

}

void Channel::remove()
{
    _loop->removeChannel(this);
}