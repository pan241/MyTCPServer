#ifndef POLLPOLLER_H
#define POLLPOLLER_H

#include "../Poller.h"
#include "../../log/Logging.h"

#include <vector>

struct pollfd;

class PollPoller : public Poller
{
public:
    PollPoller(EventLoop* eventLoop);
    ~PollPoller() override;

    Timestamp poll(int timeoutMs, ChannelList* activateChannels) override;
    void updateChannel(Channel* channel) override;
    void removeChannel(Channel* channel) override;

private:
    void fillActiveChannels(int numEvents, ChannelList* activeChannels) const;
    using PollFdList = std::vector<struct pollfd>;
    PollFdList _pollfds;
};

#endif