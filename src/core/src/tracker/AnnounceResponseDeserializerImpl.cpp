#include "AnnounceResponseDeserializerImpl.h"

#include "fmt/format.h"

#include "bencode.hpp"
#include "errors/InvalidBencodeFormatError.h"
#include "errors/MissingBencodeDictionary.h"
#include "errors/MissingBencodeFieldValue.h"

namespace core
{
namespace
{
bencode::data parseBencode(const std::string& bencodeText);
bencode::dict getDictionary(const bencode::data& data);
template <typename T>
T getFieldValue(const bencode::dict& bencodeDictionary, const std::string& fieldName);

constexpr auto intervalFieldName = "interval";
constexpr auto peersFieldName = "peers";
}

RetrievePeersResponse AnnounceResponseDeserializerImpl::deserialize(const std::string& response) const
{
    auto bencodeData = parseBencode(response);

    auto bencodeDictionary = getDictionary(bencodeData);

    auto interval = getFieldValue<bencode::integer>(bencodeDictionary, intervalFieldName);

    auto peers = getFieldValue<bencode::string>(bencodeDictionary, peersFieldName);

    auto peersEndpoints = getPeersEndpoints(peers);

    return {interval, peersEndpoints};
}

std::vector<PeerEndpoint> AnnounceResponseDeserializerImpl::getPeersEndpoints(const std::string& peers) const
{
    std::vector<PeerEndpoint> peersEndpoints;

    for (size_t i = 0; i < peers.length(); i += 6)
    {
        auto addressOctet1 = static_cast<unsigned>(static_cast<unsigned char>(peers[i]));
        auto addressOctet2 = static_cast<unsigned>(static_cast<unsigned char>(peers[i + 1]));
        auto addressOctet3 = static_cast<unsigned>(static_cast<unsigned char>(peers[i + 2]));
        auto addressOctet4 = static_cast<unsigned>(static_cast<unsigned char>(peers[i + 3]));

        auto portOctet1 = static_cast<unsigned>(static_cast<unsigned char>(peers[i + 4]));
        auto portOctet2 = static_cast<unsigned>(static_cast<unsigned char>(peers[i + 5]));

        unsigned port = (portOctet1 << 8) + portOctet2;

        peersEndpoints.push_back(
            {fmt::format("{}.{}.{}.{}", addressOctet1, addressOctet2, addressOctet3, addressOctet4), port});
    }

    return peersEndpoints;
}

namespace
{
bencode::data parseBencode(const std::string& bencodeText)
{
    try
    {
        return bencode::decode(bencodeText);
    }
    catch (const std::exception& e)
    {
        throw errors::InvalidBencodeFormatError(e.what());
    }
}

bencode::dict getDictionary(const bencode::data& data)
{
    try
    {
        return std::get<bencode::dict>(data);
    }
    catch (const std::exception& e)
    {
        throw errors::MissingBencodeDictionary{"Missing dictionary."};
    }
}

template <typename T>
T getFieldValue(const bencode::dict& bencodeDictionary, const std::string& fieldName)
{
    try
    {
        return std::get<T>(bencodeDictionary.at(fieldName));
    }
    catch (const std::exception& e)
    {
        throw errors::MissingBencodeFieldValue{fmt::format("Missing {} field.", fieldName)};
    }
}
}
}
