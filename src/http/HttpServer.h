#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include "TcpServer.h"

class HttpRequest;
class HttpResponse;

class HttpServer : noncopyable
{
public:
    using HttpCallback = std::function<void(const HttpRequest&, HttpResponse*)>;

    HttpServer(EventLoop* loop, const InetAddress& listenAddr, const std::string& name, TcpServer::Option option = TcpServer::kNoReusePort); 

    EventLoop* getLoop() const { return _server.getLoop(); }

    void setHttpCallback(const HttpCallback& cb) { _httpCallback = cb; }
    void setThreadNum(int num) { _server.setThreadNum(num); }

    void start();

private:
    void onConection(const TcpConnectionPtr& conn);
    void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp receiveTime);
    void onRequest(const TcpConnectionPtr& conn, const HttpRequest&);

    TcpServer _server;
    HttpCallback _httpCallback;
};

#endif