#include "TorrentFileDeserializerImpl.h"

#include <boost/compute/detail/sha1.hpp>

#include "fmt/format.h"

#include "encoder/HexEncoder.h"
#include "errors/InvalidBencodeFormatError.h"
#include "errors/MissingBencodeDictionary.h"
#include "errors/MissingBencodeFieldValue.h"
#include "TorrentFileInfo.h"

namespace core
{
namespace
{
bencode::data parseBencode(const std::string& bencodeText);
bencode::dict getDictionary(const bencode::data& data);
template <typename T>
T getFieldValue(const bencode::dict& bencodeDictionary, const std::string& fieldName);
template <>
bencode::data getFieldValue<bencode::data>(const bencode::dict& bencodeDictionary, const std::string& fieldName);

constexpr auto announceFieldName = "announce";
constexpr auto infoFieldName = "info";
constexpr auto nameFieldName = "name";
constexpr auto piecesFieldName = "pieces";
constexpr auto pieceLengthFieldName = "piece length";
constexpr auto lengthFieldName = "length";
}

TorrentFileInfo TorrentFileDeserializerImpl::deserialize(const std::string& torrentFileContent)
{
    const auto bencodeData = parseBencode(torrentFileContent);

    auto bencodeDictionary = getDictionary(bencodeData);

    const auto announce = getFieldValue<bencode::string>(bencodeDictionary, announceFieldName);

    const auto infoHash = getInfoHash(bencodeDictionary);

    auto infoDictionary = getFieldValue<bencode::dict>(bencodeDictionary, infoFieldName);

    const auto torrentSize = getFieldValue<bencode::integer>(infoDictionary, lengthFieldName);

    const auto piecesLength = getFieldValue<bencode::integer>(infoDictionary, pieceLengthFieldName);

    const auto fileName = getFieldValue<bencode::string>(infoDictionary, nameFieldName);

    const auto piecesHashes = getPiecesHashes(infoDictionary);

    return {announce, infoHash, torrentSize, piecesLength, fileName, piecesHashes};
}

std::string TorrentFileDeserializerImpl::getInfoHash(bencode::dict& bencodeDictionary)
{
    const auto info = getFieldValue<bencode::data>(bencodeDictionary, infoFieldName);

    const auto infoText = bencode::encode(info);

    boost::compute::detail::sha1 infoHash;

    infoHash.process(infoText);

    return infoHash;
}

std::vector<std::string> TorrentFileDeserializerImpl::getPiecesHashes(bencode::dict& infoDictionary)
{
    const auto piecesFieldValue = getFieldValue<bencode::string>(infoDictionary, piecesFieldName);

    std::vector<std::string> piecesHashesHexEncoded;

    for (size_t i = 0; i < piecesFieldValue.size(); i += 20)
    {
        std::string pieceHashBytes{piecesFieldValue.begin() + i, piecesFieldValue.begin() + i + 20};

        std::string pieceHashHexEncoded = libs::encoder::HexEncoder::encode(pieceHashBytes);

        piecesHashesHexEncoded.push_back(pieceHashHexEncoded);
    }

    return piecesHashesHexEncoded;
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

template <>
bencode::data getFieldValue<bencode::data>(const bencode::dict& bencodeDictionary, const std::string& fieldName)
{
    try
    {
        return bencodeDictionary.at(fieldName);
    }
    catch (const std::exception& e)
    {
        throw errors::MissingBencodeFieldValue{fmt::format("Missing {} field.", fieldName)};
    }
}
}
}
