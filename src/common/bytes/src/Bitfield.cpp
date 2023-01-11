#include "Bitfield.h"

namespace common::bytes
{
Bitfield::Bitfield(const std::basic_string<unsigned char>& dataInit) : data{initializeData(dataInit)} {}

bool Bitfield::hasBitSet(int bitIndex) const
{
    return data.at(bitIndex);
}

std::string Bitfield::toString() const
{
    std::string dataAsString;

    for (const auto& bit : data)
    {
        dataAsString += std::to_string(static_cast<int>(bit));
    }

    return dataAsString;
}

std::vector<bool> Bitfield::initializeData(const std::basic_string<unsigned char>& dataInit) const
{
    std::vector<bool> bits;

    for (const auto& byte : dataInit)
    {
        for (int i = 7; i >= 0; i--)
        {
            const auto bitValue = byte & (1 << i);

            bits.push_back(static_cast<bool>(bitValue));
        }
    }

    return bits;
}

}
