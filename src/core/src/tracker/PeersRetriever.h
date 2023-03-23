#pragma once

#include <string>

#include "RetrievePeersPayload.h"
#include "RetrievePeersResponse.h"

namespace core
{
class PeersRetriever
{
public:
    virtual ~PeersRetriever() = default;

    virtual void retrievePeers(const RetrievePeersPayload& payload,
                               std::function<void(const std::vector<PeerEndpoint>&)> processPeersHandler) = 0;
};
}
