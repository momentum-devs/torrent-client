#pragma once

#include <string>
#include <vector>

namespace core
{
class PiecesSerializer
{
public:
    virtual ~PiecesSerializer() = default;

    virtual std::string serialize(const std::vector<unsigned int>&) const = 0;
    virtual std::vector<unsigned int> deserialize(const std::string&) const = 0;
};
}
