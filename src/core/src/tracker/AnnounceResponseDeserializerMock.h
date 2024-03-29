#pragma once

#include "gmock/gmock.h"

#include "AnnounceResponseDeserializer.h"

namespace core
{
class AnnounceResponseDeserializerMock : public AnnounceResponseDeserializer
{
public:
    MOCK_CONST_METHOD1(deserializeBencode, RetrievePeersResponse(const std::string&));
    MOCK_CONST_METHOD1(deserializePeersEndpoints, std::vector<PeerEndpoint>(const std::string&));

};
}
