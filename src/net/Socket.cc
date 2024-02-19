#include <unistd.h>
#include <sys/types.h>
#include <




#include "InetAddress.h"
#include "Socket.h"

Socket::~Socket()
{
    ::close(_sockfd);
}