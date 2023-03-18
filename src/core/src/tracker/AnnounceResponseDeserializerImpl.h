#pragma once

#include "AnnounceResponseDeserializer.h"

namespace core
{
class AnnounceResponseDeserializerImpl : public AnnounceResponseDeserializer
{
public:
    RetrievePeersResponse deserializeBencode(const std::string&) const override;
    std::vector<PeerEndpoint> deserializePeersEndpoints(const std::string& peers) const override;
};
}
