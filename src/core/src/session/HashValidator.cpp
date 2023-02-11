#include "HashValidator.h"

#include <boost/compute/detail/sha1.hpp>

#include "encoder/HexEncoder.h"

namespace core
{
bool HashValidator::compareHashes(const std::string& hash1, const std::string& hash2)
{
    if (hash1.size() * 2 == hash2.size())
    {
        return libs::encoder::HexEncoder::encode(hash1) == hash2;
    }

    if (hash1.size() == hash2.size() * 2)
    {
        return hash1 == libs::encoder::HexEncoder::encode(hash2);
    }

    return hash1 == hash2;
}

bool HashValidator::compareHashWithData(const std::string& hash, const std::basic_string<unsigned char>& data)
{
    boost::compute::detail::sha1 dataHash;

    std::string dataString;

    for (char c : data)
    {
        dataString += c;
    }

    dataHash.process(dataString);

    return static_cast<std::string>(dataHash) == hash;
}
}
