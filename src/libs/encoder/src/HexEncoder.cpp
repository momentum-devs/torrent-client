#include "HexEncoder.h"

namespace libs::encoder
{
std::string HexEncoder::encode(const std::string& data)
{
    static const char hexDigits[] = "0123456789abcdef";

    std::string output;

    output.reserve(data.length() * 2);

    for (unsigned char c : data)
    {
        output.push_back(hexDigits[c >> 4]);
        output.push_back(hexDigits[c & 15]);
    }

    return output;
}

std::string HexEncoder::decode(const std::string& data)
{
    int hashLength = data.length();

    std::string decodedHexString;

    for (int i = 0; i < hashLength; i += 2)
    {
        std::string byte = data.substr(i, 2);

        char c = (char)(int)strtol(byte.c_str(), nullptr, 16);

        decodedHexString.push_back(c);
    }

    return decodedHexString;
}
}
