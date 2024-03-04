#include "EpollPoller.h"

#include <errno.h>
#include <string.h>
#include <assert.h>
#include <poll.h>

#include "../Channel.h"

static_assert(EPOLLIN == POLLIN, "epoll uses same flag values as poll");
static_assert(EPOLLPRI == POLLPRI, "epoll uses same flag values as poll");
static_assert(EPOLLOUT == POLLOUT, "epoll uses same flag values as poll");
static_assert(EPOLLRDHUP == POLLRDHUP, "epoll uses same flag values as poll");
static_assert(EPOLLERR == POLLERR, "epoll uses same flag values as poll");
static_assert(EPOLLHUP == POLLHUP, "epoll uses same flag values as poll");

const int kNew = -1;
const int kAdded = 1;
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

Timestamp EpollPoller::poll(int timeoutMs, ChannelList* activateChannels)
{
    int numEvents = ::epoll_wait(_epollfd, &*_events.begin(), static_cast<int>(_events.size()), timeoutMs);
    int savedErrno = errno;
    Timestamp now(Timestamp::now());

    if (numEvents > 0)
    {
        fillActiveChannels(numEvents, activateChannels);
        if (numEvents == _events.size())
        {
            _events.resize(_events.size() * 2);
        }
    }
    else if (numEvents == 0)
    {
        LOG_DEBUG << "nothing happened";
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

void EpollPoller::updateChannel(Channel* channel)
{
    const int index = channel->index();

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
    else
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

void EpollPoller::removeChannel(Channel* channel)
{
    int fd = channel->fd();
    LOG_INFO << "fd = " <<fd;
    int index = channel->index();
    size_t n = _channels.erase(fd);
    if (index == kAdded)
    {
        update(EPOLL_CTL_DEL, channel);
    }
    channel->set_index(kNew);
}

void EpollPoller::fillActiveChannels(int num, ChannelList* activeChannels) const
{
    for (int i = 0; i < num; i++)
    {
        Channel* channel = static_cast<Channel*>(_events[i].data.ptr);
        channel->set_revents(_events[i].events);
        activeChannels->push_back(channel);
    }
}

void EpollPoller::update(int operation, Channel* channel)
{
    struct epoll_event event;
    ::memset(&event, 0, sizeof(event));

    event.events = channel->events();
    event.data.ptr = channel;
    int fd = channel->fd();

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
