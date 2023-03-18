#pragma once

#include <string>

namespace libs::bytes
{
class BytesConverter
{
public:
    static std::basic_string<unsigned char> int64ToBytes(long data);
    static std::basic_string<unsigned char> int32ToBytes(unsigned int data);
    static std::basic_string<unsigned char> int16ToBytes(unsigned int data);
    static unsigned int bytesToInt(const std::basic_string<unsigned char>& data);
};
}
