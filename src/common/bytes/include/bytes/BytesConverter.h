#pragma once

#include <string>

namespace common::bytes
{
class BytesConverter
{
public:
    static std::basic_string<unsigned char> intToBytes(unsigned int data);
    static unsigned int bytesToInt(const std::basic_string<unsigned char>& data);
};
}
