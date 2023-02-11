#pragma once

#include <string>

namespace core
{
class HashValidator
{
public:
    static bool compareHashes(const std::string& hash1, const std::string& hash2);
    static bool compareHashWithData(const std::string& hash, const std::basic_string<unsigned char>& data);
};
}
