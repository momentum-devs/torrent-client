#pragma once

#include <memory>

#include "../../../common/httpClient/include/httpClient/HttpClient.h"
#include "AnnounceResponseDeserializer.h"
#include "PeersRetriever.h"

class PeersRetrieverImpl : public PeersRetriever
{
public:
    PeersRetrieverImpl(std::unique_ptr<common::httpClient::HttpClient>, std::unique_ptr<AnnounceResponseDeserializer>);

    RetrievePeersResponse retrievePeers(const RetrievePeersPayload& payload) override;

private:
    std::unique_ptr<common::httpClient::HttpClient> httpClient;
    std::unique_ptr<AnnounceResponseDeserializer> responseDeserializer;
};
