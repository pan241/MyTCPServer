#include "HttpServer.h"

#include "../log/Logging.h"
#include "HttpContext.h"
#include "HttpRequest.h"
#include "HttpResponse.h"

#include <memory>

void defaultHttpCallback(const HttpRequest&, HttpResponse* resp)
{
    resp->setStatusCode(HttpResponse::k404NotFound);
    resp->setStatusMessage("Not Found");
    resp->setCloseConnection(true);
}

HttpServer::HttpServer(EventLoop* loop, const InetAddress& listenAddr, const std::string& name, TcpServer::Option option = TcpServer::kNoReusePort)   
    : _server(loop, listenAddr, name, option),
      _httpCallback(defaultHttpCallback)
{
    _server.setConnectionCallback(std::bind(&HttpServer::onConection, this, std::placeholders::_1));
    _server.setMessageCallback(std::bind(&HttpServer::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

void HttpServer::onConection(const TcpConnectionPtr& conn)
{
    if (conn->connected())
    {
        LOG_INFO << "new Connection arrived";
    }
    else
    {
        LOG_INFO << "Connection closed";
    }
}

void HttpServer::onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp receiveTime)
{
    std::unique_ptr<HttpContext> context(new HttpContext);

    if (!context->parseRequest(buf, receiveTime))
    {
        conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
        conn->shutdown();
    } 
    
    if (context->gotAll())
    {
        onRequest(conn, context->request());
        context->reset();
    }
}

void HttpServer::onRequest(const TcpConnectionPtr& conn, const HttpRequest& req)
{
    const std::string& connection = req.getHeader("Connection");

    bool close = connection == "close" || (req.getVersion() == HttpRequest::kHttp10 && connection != "Keep-Alive");
    HttpResponse response(close);
    _httpCallback(req, &response);
    Buffer buf;
    response.appendToBuffer(&buf);
    conn->send(&buf);
    if (response.closeConnection())
    {
        conn->shutdown();
    }
}

void HttpServer::start()
{
    LOG_INFO << "HttpServer[" << _server.name().c_str() << "] start listening on " << _server.ipPort().c_str(); 
    _server.start();
}