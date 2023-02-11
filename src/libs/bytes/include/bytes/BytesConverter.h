#pragma once

#include <string>

namespace libs::bytes
{
class BytesConverter
{
public:
    static std::basic_string<unsigned char> intToBytes(unsigned int data);
    static unsigned int bytesToInt(const std::basic_string<unsigned char>& data);
};
}
