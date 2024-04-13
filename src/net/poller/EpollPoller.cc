
#include "EpollPoller.h"

#include <errno.h>
#include <strings.h>
#include <assert.h>
#include <poll.h>

#include "Channel.h"
#include "Logger.h"

// channel未添加到poller中
const int kNew = -1;
// channel添加到poller中
const int kAdded = 1;
// channel从poller中删除
const int kDeleted = 2;

EpollPoller::EpollPoller(EventLoop* loop)
    : Poller(loop),
      _epollfd(::epoll_create1(EPOLL_CLOEXEC)),
      _events(kInitEventListSize)
{
    if (_epollfd < 0)
    {
        LOG_FATAL << "epoll_create() error:" << errno;
    }
}

EpollPoller::~EpollPoller()
{
    ::close(_epollfd);
}

void EpollPoller::updateChannel(Channel* channel)
{
    const int index = channel->index();
    //LOG_INFO << "fd= " << channel->fd() << " events= " << channel->events() << " status= " << channel->index();
    if (index == kNew || index == kDeleted)
    {
        int fd = channel->fd();
        if (index == kNew)
        {
            _channels[fd] = channel;
        }
        else
        {
        }
        channel->set_index(kAdded);
        update(EPOLL_CTL_ADD, channel);
    } 
    else // channel is in epoll
    {
        if (channel->isNoneEvent())
        {
            update(EPOLL_CTL_DEL, channel);
            channel->set_index(kDeleted);
        }
        else
        {
            update(EPOLL_CTL_MOD, channel);
        }
    }
}

void EpollPoller::update(int operation, Channel* channel)
{
    epoll_event event;
    bzero(&event, sizeof(event));
    
    int fd = channel->fd();

    event.events = channel->events();
    event.data.fd = fd;
    event.data.ptr = channel;

    if (::epoll_ctl(_epollfd, operation, fd, &event) < 0)
    {
        if (operation == EPOLL_CTL_DEL)
        {
            LOG_ERROR << "epoll_ctl() del error" << " fd =" << fd << " errno =" << errno;
        }
        else
        {
            LOG_FATAL << "epoll_ctl() add/mod error" << " fd =" << fd << " errno =" << errno;
        }
    }
}

void EpollPoller::removeChannel(Channel* channel)
{
    int fd = channel->fd();
    LOG_INFO << "remove fd = " <<fd;
    size_t n = _channels.erase(fd);
    
    if (channel->index() == kAdded)
    {
        update(EPOLL_CTL_DEL, channel);
    }
    channel->set_index(kNew);
}

Timestamp EpollPoller::poll(int timeoutMs, ChannelList* activateChannels)
{
    // LOG_INFO << "poll with " << _channels.size() << " fd";
    
    int numEvents = ::epoll_wait(_epollfd, &*_events.begin(), static_cast<int>(_events.size()), timeoutMs);
    int savedErrno = errno;
    Timestamp now(Timestamp::now());

    if (numEvents > 0)
    {
        // LOG_INFO << numEvents << " events happen";
        fillActiveChannels(numEvents, activateChannels);
        if (numEvents == _events.size())
        {
            _events.resize(_events.size() * 2);
        }
    }
    else if (numEvents == 0)
    {
        LOG_DEBUG << "time out!";
    }
    else
    {
        if (savedErrno != EINTR)
        {
            errno = savedErrno;
            LOG_ERROR << "EPollPoller::Poll()";
        }
    }
    return now;
}

void EpollPoller::fillActiveChannels(int numEvents, ChannelList* activeChannels) const
{
    for (int i = 0; i < numEvents; i++)
    {
        Channel* channel = static_cast<Channel*>(_events[i].data.ptr);
        channel->set_revents(_events[i].events);
        activeChannels->push_back(channel);
    }
}
