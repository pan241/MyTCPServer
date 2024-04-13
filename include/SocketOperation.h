#pragma once

#include <arpa/inet.h>

namespace sockets
{
int createNonblockingOrDie(sa_family_t family);

int connect(int sockfd, const struct sockaddr* addr);

void close(int sockfd);

int getSocketError(int sockfd);




struct sockaddr_in6 getLocalAddr(int sockfd);
struct sockaddr_in6 getPeerAddr(int sockfd);
bool isSelfConnect(int sockfd);
};
