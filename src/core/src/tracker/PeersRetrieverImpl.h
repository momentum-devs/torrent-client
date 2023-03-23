#pragma once

#include <memory>

#include "AnnounceResponseDeserializer.h"
#include "httpClient/HttpClient.h"
#include "PeersRetriever.h"

namespace core
{
class PeersRetrieverImpl : public PeersRetriever
{
public:
    PeersRetrieverImpl(std::unique_ptr<libs::httpClient::HttpClient>, std::unique_ptr<AnnounceResponseDeserializer>);

    void retrievePeers(const RetrievePeersPayload& payload,
                       std::function<void(const std::vector<PeerEndpoint>&)> processPeersHandler) override;

private:
    std::unique_ptr<libs::httpClient::HttpClient> httpClient;
    std::unique_ptr<AnnounceResponseDeserializer> responseDeserializer;
};
}
