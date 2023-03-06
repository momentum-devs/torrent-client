#pragma once

#include <ostream>
#include <string>

#include "collection/StlOperators.h"
#include "NestedFileInfo.h"

namespace core
{
struct TorrentFileInfo
{
    std::string announce;
    std::string infoHash;
    long long length;
    long long pieceLength;
    std::string name;
    std::vector<std::string> piecesHashes;
    std::optional<std::vector<NestedFileInfo>> nestedFilesInfo;
};

inline std::ostream& operator<<(std::ostream& os, const TorrentFileInfo& torrentFileInfo)
{
    return os << "name: " << torrentFileInfo.name << "\nannounce: " << torrentFileInfo.announce
              << "\ninfo hash: " << torrentFileInfo.infoHash << "\nlength: " << torrentFileInfo.length
              << "\npiece length: " << torrentFileInfo.pieceLength
              << "\nnested files info: " << torrentFileInfo.nestedFilesInfo
              << "\npieces hashes: " << torrentFileInfo.piecesHashes;
}

inline bool operator==(const TorrentFileInfo& lhs, const TorrentFileInfo& rhs)
{
    auto tieStruct = [](const TorrentFileInfo& torrentFileInfo)
    {
        return std::tie(torrentFileInfo.announce, torrentFileInfo.infoHash, torrentFileInfo.length,
                        torrentFileInfo.pieceLength, torrentFileInfo.piecesHashes, torrentFileInfo.name,
                        torrentFileInfo.nestedFilesInfo);
    };

    return tieStruct(lhs) == tieStruct(rhs);
}
}
