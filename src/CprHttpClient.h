#pragma once

#include "HttpClient.h"

class CprHttpClient : public HttpClient
{
public:
    HttpResponse get(const HttpGetRequestPayload&) const override;
};
