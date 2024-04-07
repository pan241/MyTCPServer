#include "PollPoller.h"

#include <poll.h>
#include <errno.h>

#include "Channel.h"

PollPoller::PollPoller(EventLoop* loop)
    : Poller(loop)
{
}

PollPoller::~PollPoller() = default;

Timestamp PollPoller::poll(int timeoutMs, ChannelList* activateChannels)
{
    int numEvents = ::poll(&*_pollfds.begin(), _pollfds.size(), timeoutMs);
    int savedErrno = errno;
    Timestamp now(Timestamp::now());
    if (numEvents > 0)
    {
        fillActiveChannels(numEvents, activateChannels);
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
            LOG_ERROR << "PollPoller::Poll()";
        }
    }
    return now;
}

void PollPoller::updateChannel(Channel* channel)
{
    if (channel->index() < 0)
    {
        struct pollfd pfd;
        pfd.fd = channel->fd();
        pfd.events = static_cast<short>(channel->events());
        pfd.revents = 0;
        _pollfds.push_back(pfd);
        int idx = static_cast<int>(_pollfds.size()) - 1;
        channel->set_index(idx);
        _channels[pfd.fd] = channel;
    }
    else
    {
        int idx = channel->index();
        struct pollfd& pfd = _pollfds[idx];
        pfd.fd = channel->fd();
        pfd.events = static_cast<short>(channel->events());
        pfd.revents = 0;
        if (channel->isNoneEvent())
        {
            pfd.fd = -channel->fd() - 1;
        }
    }
}

void PollPoller::removeChannel(Channel* channel)
{
    int idx = channel->index();
    const struct pollfd& pfd = _pollfds[idx];
    (void)pfd;
    size_t n = _channels.erase(channel->fd());
    if (idx == _pollfds.size() - 1)
    {
        _pollfds.pop_back();
    }
    else
    {
        int channelAtEnd = _pollfds.back().fd;
        iter_swap(_pollfds.begin() + idx, _pollfds.end() - 1);
        if (channelAtEnd < 0)
        {
            channelAtEnd = -channelAtEnd - 1;
        }
        _channels[channelAtEnd]->set_index(idx);
        _pollfds.pop_back();
    }
}

void PollPoller::fillActiveChannels(int numEvents, ChannelList* activeChannels) const
{
    for (PollFdList::const_iterator pfd = _pollfds.begin();
        pfd != _pollfds.end() && numEvents > 0; ++pfd)
    {
        if (pfd->revents > 0)
        {
            --numEvents;
            ChannelMap::const_iterator ch = _channels.find(pfd->fd);
            Channel* channel = ch->second;
            channel->set_revents(pfd->revents);
            activeChannels->push_back(channel);
        }
    }
}