#pragma once

#include <memory>

#include "AnnounceResponseDeserializer.h"
#include "httpClient/HttpClient.h"
#include "PeerRetriever.h"

class PeerRetrieverImpl : public PeerRetriever
{
public:
    PeerRetrieverImpl(std::unique_ptr<common::httpClient::HttpClient>, std::unique_ptr<AnnounceResponseDeserializer>);

    RetrievePeersResponse retrievePeers(const RetrievePeersPayload& payload) override;

private:
    std::unique_ptr<common::httpClient::HttpClient> httpClient;
    std::unique_ptr<AnnounceResponseDeserializer> responseDeserializer;
};
