#include "Bitfield.h"

namespace libs::bytes
{
Bitfield::Bitfield(const std::basic_string<unsigned char>& dataInit) : data{initializeData(dataInit)} {}

bool Bitfield::hasBitSet(unsigned int bitIndex) const
{
    if (bitIndex >= data.size())
    {
        return false;
    }

    try
    {
        return data.at(bitIndex);
    }
    catch (const std::exception& error)
    {
        return false;
    }
}

std::size_t Bitfield::size() const
{
    return data.size();
}

void Bitfield::setBit(unsigned int bitIndex)
{
    if (bitIndex < static_cast<unsigned int>(data.size()))
    {
        data[bitIndex] = true;
    }
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
