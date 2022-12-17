#pragma once

#include "AnnounceResponseDeserializer.h"

class AnnounceResponseDeserializerImpl : public AnnounceResponseDeserializer
{
public:
    RetrievePeersResponse deserialize(const std::string&) const override;

private:
    std::vector<PeerEndpoint> getPeersEndpoints(const std::string& peers) const;
};
