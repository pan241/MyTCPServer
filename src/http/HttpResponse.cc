#include "HttpResponse.h"
#include "../net/Buffer.h"

#include <stdio.h>
#include <string.h>

void HttpResponse::appendToBuffer(Buffer* output) const
{
    char buf[32];
    memset(buf, '\0', sizeof(buf));
    snprintf(buf, sizeof(buf), "HTTP/1.1 %d", _statusCode);
    output->append(buf);
    output->append(_statusMessage);
    output->append("\r\n");

    if (_closeConnection)
    {
        output->append("Connection: close\r\n");
    }
    else
    {
        snprintf(buf, sizeof(buf), "Content-Length: %zd\r\n", _body.size());
        output->append(buf);
        output->append("Connection: Keep-Alive\r\n");
    }

    for (const auto& header : _headers)
    {
        output->append(header.first);
        output->append(": ");
        output->append(header.second);
        output->append("\r\n");
    }

    output->append("\r\n");
    output->append(_body);
}