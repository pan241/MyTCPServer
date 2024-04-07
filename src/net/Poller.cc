#include "Poller.h"
#include "PollPoller.h"
#include "EpollPoller.h"
#include "Channel.h"
#include "EventLoop.h"

Poller::Poller(EventLoop* loop)
    : _ownerLoop(loop)
{
}

bool Poller::hasChannel(Channel* channel) const
{
    auto it = _channels.find(channel->fd());
    return it != _channels.end() && it->second == channel;
}
