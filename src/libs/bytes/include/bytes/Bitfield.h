#pragma once

#include <string>
#include <vector>

namespace libs::bytes
{
class Bitfield
{
public:
    explicit Bitfield(const std::basic_string<unsigned char>& data);

    bool hasBitSet(unsigned int bitIndex) const;
    std::string toString() const;
    std::size_t size() const;
    void setBit(unsigned int bitIndex);

private:
    std::vector<bool> initializeData(const std::basic_string<unsigned char>& data) const;

    std::vector<bool> data;
};

}
