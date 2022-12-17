#include "TorrentFileDeserializerImpl.h"

#include <boost/compute/detail/sha1.hpp>

#include "bencode.hpp"
#include "errors/BencodeParseError.h"
#include "errors/MissingTorrentInfo.h"
#include "TorrentFileInfo.h"

TorrentFileInfo TorrentFileDeserializerImpl::deserialize(const std::string& torrentFileContent)
{
    bencode::data bencodeData;

    try
    {
        bencodeData = bencode::decode(torrentFileContent);
    }
    catch (const std::exception& e)
    {
        throw errors::BencodeParseError(e.what());
    }

    bencode::dict torrentDict;

    try
    {
        torrentDict = std::get<bencode::dict>(bencodeData);
    }
    catch (const std::exception& e)
    {
        throw errors::MissingTorrentInfo(e.what());
    }

    auto announce = getAnnounce(torrentDict);

    auto infoHash = getInfoHash(torrentDict);

    auto torrentSize = getTorrentSize(torrentDict);

    return {announce, infoHash, torrentSize};
}
std::string TorrentFileDeserializerImpl::getAnnounce(bencode::dict& dict)
{
    std::string announce;
    try
    {
        announce = std::get<bencode::string>(dict.at("announce"));
    }
    catch (const std::exception& e)
    {
        throw errors::MissingTorrentInfo(e.what());
    }

    return announce;
}

std::string TorrentFileDeserializerImpl::getInfoHash(bencode::dict& dict)
{
    bencode::data info;

    try
    {
        info = dict.at("info");
    }
    catch (const std::exception& e)
    {
        throw errors::MissingTorrentInfo(e.what());
    }

    auto infoText = bencode::encode(info);

    boost::compute::detail::sha1 sha1;

    sha1.process(infoText);

    std::string infoHash = sha1;

    return infoHash;
}

long long TorrentFileDeserializerImpl::getTorrentSize(bencode::dict& dict)
{
    bencode::data info;

    try
    {
        info = dict.at("info");
    }
    catch (const std::exception& e)
    {
        throw errors::MissingTorrentInfo(e.what());
    }

    long long fileSizeSum = 0;

    auto infoDict = std::get<bencode::dict>(info);

    bencode::list files;

    try
    {
        files = std::get<bencode::list>(infoDict.at("files"));
    }
    catch (const std::exception& e)
    {
        throw errors::MissingTorrentInfo(e.what());
    }

    for(const auto& file : files)
    {
        auto fileDict = std::get<bencode::dict>(file);

        long long fileSize;

        try
        {
            fileSize = std::get<bencode::integer>(fileDict.at("length"));
        }
        catch (const std::exception& e)
        {
            throw errors::MissingTorrentInfo(e.what());
        }

        fileSizeSum += fileSize;
    }

    return fileSizeSum;
}
