#include "Connector.h"

#include "errno.h"

#include "../log/Logging.h"


const int Connector::kMaxRetryDelayMs;

Connector::Connector(EventLoop* loop, const InetAddress& serverAddr)

{}

Connector::~Connector()
{

}

void Connector::start()
{

}

void Connector::restart()
{

}

void Connector::stop()
{

}

void Connector::startInLoop()
{

}

void Connector::stopInLoop()
{

}

void Connector::connect()
{

}

void Connector::connecting(int sockfd)
{

}

void Connector::handleWrite()
{

}

void Connector::handleError()
{

}

void Connector::retry(int sockfd)
{

}

int Connector::removeAndResetChannel()
{

}

void Connector::resetChannel()
{
    
}