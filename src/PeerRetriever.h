#pragma once

#include <string>

#include "RetrievePeersPayload.h"
#include "RetrievePeersResponse.h"

class PeerRetriever
{
public:
    virtual ~PeerRetriever() = default;

    virtual RetrievePeersResponse retrievePeers(const RetrievePeersPayload& payload) = 0;
};
