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

}

void EpollPoller::updateChannel(Channel* channel)
{

}

void EpollPoller::removeChannel(Channel* channel)
{

}

void EpollPoller::fillActiveChannels(int num, ChannelList* activeChannels) const
{

}

void EpollPoller::update(int operation, Channel* channel)
{
    struct epoll_event event;
    ::memset(&event, 0, sizeof(event));

    event.events = channel->events();
    event.data.ptr = channel;
    int fd = channel->fd();
}
