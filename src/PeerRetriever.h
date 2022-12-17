#pragma once

#include <string>

#include "RetrievePeersPayload.h"

class PeerRetriever
{
public:
    virtual ~PeerRetriever() = default;

    virtual std::string retrievePeers(const RetrievePeersPayload& payload) = 0;
};
