#pragma once

#include "PiecesSerializer.h"

namespace core
{
class PiecesSerializerImpl : public PiecesSerializer
{
public:
    std::string serialize(const std::vector<unsigned int>&) const;
    std::vector<unsigned int> deserialize(const std::string&) const;
};
}
