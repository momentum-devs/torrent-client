#pragma once

#include <memory>

#include "AnnounceResponseDeserializer.h"
#include "HttpClient.h"
#include "PeerRetriever.h"

class PeerRetrieverImpl : public PeerRetriever
{
public:
    PeerRetrieverImpl(std::unique_ptr<HttpClient>, std::unique_ptr<AnnounceResponseDeserializer>);

    RetrievePeersResponse retrievePeers(const RetrievePeersPayload& payload) override;

private:
    std::unique_ptr<HttpClient> httpClient;
    std::unique_ptr<AnnounceResponseDeserializer> responseDeserializer;
};
