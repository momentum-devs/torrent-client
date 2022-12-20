#pragma once

#include <memory>

#include "AnnounceResponseDeserializer.h"
#include "HttpClient.h"
#include "PeersRetriever.h"

class PeersRetrieverImpl : public PeersRetriever
{
public:
    PeersRetrieverImpl(std::unique_ptr<HttpClient>, std::unique_ptr<AnnounceResponseDeserializer>);

    RetrievePeersResponse retrievePeers(const RetrievePeersPayload& payload) override;

private:
    std::unique_ptr<HttpClient> httpClient;
    std::unique_ptr<AnnounceResponseDeserializer> responseDeserializer;
};
