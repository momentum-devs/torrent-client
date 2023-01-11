#pragma once

#include <string>
#include <vector>

namespace common::bytes
{
class Bitfield
{
public:
    explicit Bitfield(const std::basic_string<unsigned char>& data);

    bool hasBitSet(int bitIndex) const;
    std::string toString() const;

private:
    std::vector<bool> initializeData(const std::basic_string<unsigned char>& data) const;

    const std::vector<bool> data;
};

}
