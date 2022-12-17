#include "TorrentFileDeserializerImpl.h"

#include <boost/compute/detail/sha1.hpp>

#include "BencodeHelper.h"
#include "HexEncoder.h"
#include "TorrentFileInfo.h"

namespace
{
constexpr auto announceFieldName = "announce";
constexpr auto infoFieldName = "info";
constexpr auto nameFieldName = "name";
constexpr auto piecesFieldName = "pieces";
constexpr auto pieceLengthFieldName = "piece length";
constexpr auto lengthFieldName = "length";
}

TorrentFileInfo TorrentFileDeserializerImpl::deserialize(const std::string& torrentFileContent)
{
    auto bencodeData = parseBencode(torrentFileContent);

    auto bencodeDictionary = getDictionary(bencodeData);

    auto announce = getFieldValue<bencode::string>(bencodeDictionary, announceFieldName);

    auto infoHash = getInfoHash(bencodeDictionary);

    auto infoDictionary = getFieldValue<bencode::dict>(bencodeDictionary, infoFieldName);

    auto torrentSize = getFieldValue<bencode::integer>(infoDictionary, lengthFieldName);

    auto piecesLength = getFieldValue<bencode::integer>(infoDictionary, pieceLengthFieldName);

    auto fileName = getFieldValue<bencode::string>(infoDictionary, nameFieldName);

    auto piecesHashes = getPiecesHashes(infoDictionary);

    return {announce, infoHash, torrentSize, piecesLength, fileName, piecesHashes};
}

std::string TorrentFileDeserializerImpl::getInfoHash(bencode::dict& bencodeDictionary)
{
    auto info = getFieldValue<bencode::data>(bencodeDictionary, infoFieldName);

    auto infoText = bencode::encode(info);

    boost::compute::detail::sha1 infoHash;

    infoHash.process(infoText);

    return infoHash;
}

std::vector<std::string> TorrentFileDeserializerImpl::getPiecesHashes(bencode::dict& infoDictionary)
{
    auto piecesFieldValue = getFieldValue<bencode::string>(infoDictionary, piecesFieldName);

    std::vector<std::string> piecesHashesVector;

    for (size_t i = 0; i < piecesFieldValue.size(); i += 20)
    {
        std::string pieceHashBytes{piecesFieldValue.begin() + i, piecesFieldValue.begin() + i + 20};
        std::string pieceHashHex = HexEncoder::encode(pieceHashBytes);
        piecesHashesVector.push_back(pieceHashHex);
    }

    return piecesHashesVector;
}
