#pragma once

#include <memory>

#include "HttpClient.h"

namespace libs::httpClient
{
class HttpClientFactory
{
public:
    std::unique_ptr<HttpClient> createHttpClient() const;
};
}
