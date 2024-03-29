#pragma once

#include "gmock/gmock.h"

#include "HttpClient.h"

namespace libs::httpClient
{
class HttpClientMock : public HttpClient
{
public:
    MOCK_CONST_METHOD1(get, HttpResponse(const HttpGetRequestPayload&));
};
}
