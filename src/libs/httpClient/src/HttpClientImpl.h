#pragma once

#include "HttpClient.h"

namespace libs::httpClient
{
class HttpClientImpl : public HttpClient
{
public:
    HttpResponse get(const HttpGetRequestPayload&) const override;
};
}
