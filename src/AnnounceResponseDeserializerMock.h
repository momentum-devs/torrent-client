#pragma once

#include "gmock/gmock.h"

#include "AnnounceResponseDeserializer.h"

class AnnounceResponseDeserializerMock : public AnnounceResponseDeserializer
{
public:
    MOCK_CONST_METHOD1(deserialize, RetrievePeersResponse(const std::string&));
};
