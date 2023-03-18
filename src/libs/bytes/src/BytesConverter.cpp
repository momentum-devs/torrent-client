#include "BytesConverter.h"

#include <stdexcept>

#include "fmt/format.h"

namespace libs::bytes
{
namespace
{
const size_t bytesInInteger = 4;
}

std::basic_string<unsigned char> BytesConverter::int64ToBytes(long data)
{
    std::basic_string<unsigned char> bytes;

    bytes += static_cast<char>(data >> 56);
    bytes += static_cast<char>(data >> 48);
    bytes += static_cast<char>(data >> 40);
    bytes += static_cast<char>(data >> 32);
    bytes += static_cast<char>(data >> 24);
    bytes += static_cast<char>(data >> 16);
    bytes += static_cast<char>(data >> 8);
    bytes += static_cast<char>(data);

    return bytes;
}

std::basic_string<unsigned char> BytesConverter::int32ToBytes(unsigned int data)
{
    std::basic_string<unsigned char> bytes;

    bytes += static_cast<char>(data >> 24);
    bytes += static_cast<char>(data >> 16);
    bytes += static_cast<char>(data >> 8);
    bytes += static_cast<char>(data);

    return bytes;
}

std::basic_string<unsigned char> BytesConverter::int16ToBytes(unsigned int data)
{
    std::basic_string<unsigned char> bytes;

    bytes += static_cast<char>(data >> 8);
    bytes += static_cast<char>(data);

    return bytes;
}

unsigned int BytesConverter::bytesToInt(const std::basic_string<unsigned char>& bytes)
{
    if (bytes.size() != bytesInInteger)
    {
        throw std::invalid_argument{fmt::format(
            "Invalid number of bytes in integer, provided {} bytes, expected {} bytes", bytes.size(), bytesInInteger)};
    }

    const auto byte1 = static_cast<unsigned int>(bytes[0]) << 24;
    const auto byte2 = static_cast<unsigned int>(bytes[1]) << 16;
    const auto byte3 = static_cast<unsigned int>(bytes[2]) << 8;
    const auto byte4 = static_cast<unsigned int>(bytes[3]);

    auto data = byte1 + byte2 + byte3 + byte4;

    return data;
}
}
