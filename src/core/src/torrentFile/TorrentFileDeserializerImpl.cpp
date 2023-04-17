#include "TorrentFileDeserializerImpl.h"

#include <boost/compute/detail/sha1.hpp>

#include "fmt/format.h"

#include "encoder/HexEncoder.h"
#include "errors/InvalidBencodeFormatError.h"
#include "errors/MissingBencodeDictionary.h"
#include "errors/MissingBencodeFieldValue.h"
#include "loguru.hpp"
#include "TorrentFileInfo.h"

namespace core
{
namespace
{
bencode::data parseBencode(const std::string& bencodeText);
bencode::dict getDictionary(const bencode::data& data);
template <typename T>
T getFieldValue(const bencode::dict& bencodeDictionary, const std::string& fieldName);
template <typename T>
std::optional<T> getFieldValueAsOptional(const bencode::dict& bencodeDictionary, const std::string& fieldName);
template <>
bencode::data getFieldValue<bencode::data>(const bencode::dict& bencodeDictionary, const std::string& fieldName);

constexpr auto announceFieldName = "announce";
constexpr auto announceListFieldName = "announce-list";
constexpr auto infoFieldName = "info";
constexpr auto nameFieldName = "name";
constexpr auto nestedFilesFieldName = "files";
constexpr auto nestedFileFieldName = "path";
constexpr auto piecesFieldName = "pieces";
constexpr auto pieceLengthFieldName = "piece length";
constexpr auto lengthFieldName = "length";
}

TorrentFileInfo TorrentFileDeserializerImpl::deserialize(const std::string& torrentFileContent) const
{
    const auto bencodeData = parseBencode(torrentFileContent);

    const auto bencodeDictionary = getDictionary(bencodeData);

    const auto announceFieldValue = getFieldValue<bencode::string>(bencodeDictionary, announceFieldName);

    const auto announceList = getFieldValueAsOptional<bencode::list>(bencodeDictionary, announceListFieldName);

    std::set<std::string> announceFullList{announceFieldValue};

    if (announceList)
    {
        for (const auto& optionalAnnounceList : *announceList)
        {
            for (const auto& optionalAnnounce : std::get<bencode::list>(optionalAnnounceList))
            {
                std::string announce = std::get<std::string>(optionalAnnounce);

                if (announce.find("ipv6") != std::string::npos)
                {
                    continue;
                }

                announceFullList.insert(announce);
            }
        }
    }

    const auto infoHash = getInfoHash(bencodeDictionary);

    const auto infoDictionary = getFieldValue<bencode::dict>(bencodeDictionary, infoFieldName);

    const auto piecesLength = getFieldValue<bencode::integer>(infoDictionary, pieceLengthFieldName);

    const auto fileName = getFieldValue<bencode::string>(infoDictionary, nameFieldName);

    const auto nestedFilesList = getFieldValueAsOptional<bencode::list>(infoDictionary, nestedFilesFieldName);

    std::vector<NestedFileInfo> nestedFilesInfo;

    long long torrentSize = 0;

    if (nestedFilesList && !nestedFilesList->empty())
    {
        for (const auto& nestedFileListEntry : *nestedFilesList)
        {
            const auto nestedFileDictionary = getDictionary(nestedFileListEntry);

            const auto nestedFileNameList = getFieldValue<bencode::list>(nestedFileDictionary, nestedFileFieldName);

            std::string nestedFileName;

            for (const auto& nestedFileNameListEntry : nestedFileNameList)
            {
                nestedFileName += std::get<bencode::string>(nestedFileNameListEntry);
            }

            const auto nestedFileSize = getFieldValue<bencode::integer>(nestedFileDictionary, lengthFieldName);

            nestedFilesInfo.push_back(NestedFileInfo{nestedFileName, nestedFileSize});

            torrentSize += nestedFileSize;
        }
    }
    else
    {
        torrentSize = getFieldValue<bencode::integer>(infoDictionary, lengthFieldName);
    }

    const auto piecesHashes = getPiecesHashes(infoDictionary);

    return {std::vector<std::string>{announceFullList.begin(), announceFullList.end()},
            infoHash,
            torrentSize,
            piecesLength,
            fileName,
            piecesHashes,
            nestedFilesInfo.empty() ? std::nullopt : std::optional<std::vector<NestedFileInfo>>(nestedFilesInfo)};
}

std::string TorrentFileDeserializerImpl::getInfoHash(const bencode::dict& bencodeDictionary) const
{
    const auto info = getFieldValue<bencode::data>(bencodeDictionary, infoFieldName);

    const auto infoText = bencode::encode(info);

    boost::compute::detail::sha1 infoHash;

    infoHash.process(infoText);

    return infoHash;
}

std::vector<std::string> TorrentFileDeserializerImpl::getPiecesHashes(const bencode::dict& infoDictionary) const
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
        LOG_S(ERROR) << e.what();

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
        LOG_S(ERROR) << e.what();

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
        LOG_S(ERROR) << e.what();

        throw errors::MissingBencodeFieldValue{fmt::format("Missing {} field.", fieldName)};
    }
}

template <typename T>
std::optional<T> getFieldValueAsOptional(const bencode::dict& bencodeDictionary, const std::string& fieldName)
{
    try
    {
        return std::get<T>(bencodeDictionary.at(fieldName));
    }
    catch (const std::exception& e)
    {
        return std::nullopt;
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
        LOG_S(ERROR) << e.what();

        throw errors::MissingBencodeFieldValue{fmt::format("Missing {} field.", fieldName)};
    }
}
}
}
