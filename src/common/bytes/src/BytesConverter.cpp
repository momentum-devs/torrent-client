#include "BytesConverter.h"

#include <iostream>
#include <stdexcept>

#include "fmt/format.h"

namespace common::bytes
{
namespace
{
const size_t bytesInInteger = 4;
}

std::basic_string<unsigned char> BytesConverter::intToBytes(unsigned int data)
{
    std::basic_string<unsigned char> bytes;

    bytes += static_cast<char>(data >> 24);
    bytes += static_cast<char>(data >> 16);
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

    const auto byte1 = bytes[0] << 24;
    const auto byte2 = bytes[1] << 16;
    const auto byte3 = bytes[2] << 8;
    const auto byte4 = bytes[3];

    auto data = byte1 + byte2 + byte3 + byte4;

    std::cout << fmt::format("Bytes {},{},{},{} converted to integer {}", bytes[0], bytes[1], bytes[2], bytes[3], data)
              << std::endl;

    return data;
}
}
