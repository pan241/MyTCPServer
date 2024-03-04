#ifndef CONNECTOR_H
#define CONNECTOR_H

#include <functional>
#include <memory>

#include "../base/noncopyable.h"
#include "InetAddress.h"

class Channel;
class EventLoop;

class Connector : noncopyable,
            public std::enable_shared_from_this<Connector>
{
public:


private:
    enum States
    {
        kDisconnected,
        kConnecting,
        kConnected
    };
    static const int kMaxRetryDelayMs = 30 * 1000;
    

};

#endif
