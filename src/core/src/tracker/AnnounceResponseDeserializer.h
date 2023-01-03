#pragma once

#include "RetrievePeersResponse.h"

namespace core
{
class AnnounceResponseDeserializer
{
public:
    virtual ~AnnounceResponseDeserializer() = default;

    virtual RetrievePeersResponse deserialize(const std::string&) const = 0;
};
}
