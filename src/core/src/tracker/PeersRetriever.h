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

    virtual RetrievePeersResponse retrievePeers(const RetrievePeersPayload& payload) = 0;
};
}
