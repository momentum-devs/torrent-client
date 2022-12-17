#pragma once

#include "PeerRetriever.h"

class PeerRetrieverImpl : public PeerRetriever
{
public:
    std::string retrievePeers(const RetrievePeersPayload& payload) override;
};
