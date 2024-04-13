#ifndef HTTPRESPONSE_H
#define HTTPRESPONSE_H

#include <unordered_map>
#include <string>

#include "HttpRequest.h"
 
class Buffer;

class HttpResponse
{
public:
    enum HttpStatusCode
    {
        kUnknown,
        k2000k = 200,
        k301MovedPermanently = 301,
        k400BadRequest = 400,
        k404NotFound = 404
    };
    enum Version { kUnknown, kHttp10, kHttp11 };

    explicit HttpResponse(bool close)
        : _statusCode(kUnknown),
          _closeConnection(close)
    {
    }

    void setStatusCode(HttpStatusCode code) { _statusCode = code; }

    void setStatusMessage(const std::string& message) { _statusMessage = message; }
    
    void setCloseConnection(bool on) { _closeConnection = on; }
    bool closeConnection() const { return _closeConnection; }

    void setContentType(const std::string& type) { addHeader("Content-Type", type); }
    void addHeader(const std::string& key, const std::string& value) { _headers[key] = value; }
     
    void setBody(const std::string& body) { _body = body; }

    void appendToBuffer(Buffer* output) const;

private:
    std::unordered_map<std::string, std::string> _headers;
    HttpStatusCode _statusCode;
    Version _version;
    std::string _statusMessage;
    bool _closeConnection;
    std::string _body;
};

#endif