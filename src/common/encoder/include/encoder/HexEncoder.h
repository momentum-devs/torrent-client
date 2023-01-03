#pragma once

#include <string>

namespace common::encoder
{
class HexEncoder
{
public:
    static std::string encode(const std::string& data);
    static std::string decode(const std::string& data);
};
}
