#pragma once

#include "RetrievePeersResponse.h"

namespace core
{
class AnnounceResponseDeserializer
{
public:
    virtual ~AnnounceResponseDeserializer() = default;

    virtual RetrievePeersResponse deserializeBencode(const std::string&) const = 0;
    virtual std::vector<PeerEndpoint> deserializePeersEndpoints(const std::string&) const = 0;
};
}
