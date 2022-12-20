#include "Bitfield.h"

Bitfield::Bitfield(const std::basic_string<unsigned char>& dataInit) : data{initializeData(dataInit)} {}

bool Bitfield::hasBitSet(int bitIndex) const
{
    return data.at(bitIndex);
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
