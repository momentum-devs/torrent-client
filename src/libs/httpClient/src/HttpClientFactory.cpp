#include "HttpClientFactory.h"

#include "HttpClientImpl.h"

namespace libs::httpClient
{
std::unique_ptr<HttpClient> HttpClientFactory::createHttpClient() const
{
    return std::make_unique<HttpClientImpl>();
}
}
