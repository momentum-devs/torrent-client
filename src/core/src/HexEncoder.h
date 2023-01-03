#pragma once

#include <string>

namespace core
{
class HexEncoder
{
public:
    static std::string encode(const std::string& data);
    static std::string decode(const std::string& data);
};
}
