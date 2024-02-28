#ifndef EPOLLER_H
#define EPOLLER_H

#include <vector>
#include <sys/epoll.h>
#include <unistd.h>

#include "../base/Timestamp.h"
#include "../log/Logging.h"
#include "../Poller.h"

class Epoller : public Poller
{
    using EventList = std::vector<epoll_event>;

public:
    Epoller(EventLoop* loop);
    ~Epoller() override;

    Timestamp poll(int timeoutMs, ChannelList* activateChannels) override;
    void updateChannel(Channel* channel) override;
    void removeChannel(Channel* channel) override;

private:
    static const int kInitEventListSize = 16;

    void fillActiveChannels(int num, ChannelList* activeChannels) const;

    void update(int operation, Channel* Channel);

    int _epollfd;
    EventList _events;
};

#endif