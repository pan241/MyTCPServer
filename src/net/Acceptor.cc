#include "Acceptor.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include "../log/Logging.h"

#include
#include <sys/socket.h>
#include <unistd.h>


static int createNonblocking()
{
    int sockfd = ::socket(AF_INET)
}


 Acceptor::Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport)
    : _loop(loop),
      _acceptSocket()


 Acceptor::~Acceptor()
 {}