#pragma once

#include <vector>
#include <sys/epoll.h>
#include <unistd.h>

#include "Timestamp.h"
#include "Logger.h"
#include "Poller.h"

/*
 * epoll_create
 * epoll_ctl
 * epoll_wait 
 */

class Channel;

class EpollPoller : public Poller
{
    using EventList = std::vector<epoll_event>;

public:
    EpollPoller(EventLoop* loop);
    ~EpollPoller() override;

    Timestamp poll(int timeoutMs, ChannelList* activateChannels) override;
    void updateChannel(Channel* channel) override;
    void removeChannel(Channel* channel) override;

private:
    static const int kInitEventListSize = 16;

    void fillActiveChannels(int num, ChannelList* activeChannels) const;

    void update(int operation, Channel* channel);

    int _epollfd;
    EventList _events;
};
