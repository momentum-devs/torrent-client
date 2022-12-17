#pragma once

#include "HttpGetRequestPayload.h"
#include "HttpResponse.h"

class HttpClient
{
public:
    virtual ~HttpClient() = default;

    virtual HttpResponse get(const HttpGetRequestPayload&) const = 0;
};
