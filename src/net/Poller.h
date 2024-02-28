#ifndef POLLER_H
#define POLLER_H

#include "../base/noncopyable.h"
#include "../net/Channel.h"
#include "../base/Timestamp.h"
#include "../net/EventLoop.h"

#include <unordered_map>
#include <vector>

class Poller : noncopyable
{
public:
    using ChannelList = std::vector<Channel*>;

    Poller(EventLoop* loop);
    virtual ~Poller() = default;

    virtual Timestamp poll(int timeoutMs, ChannelList* activateChannels) = 0;
    virtual void updateChannel(Channel* channel) = 0;
    virtual void removeChannel(Channel* channel) = 0;

    virtual bool hasChannel(Channel* channel) const;

    static Poller* newDefaultChannel(EventLoop* loop);

protected:
    using ChannelMap = std::unordered_map<int, Channel*>;
    ChannelMap _channels;

private:
    EventLoop *_ownerLoop;
};

#endif