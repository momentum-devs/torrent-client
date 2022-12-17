#pragma once

#include <string>
#include <ostream>

#include "StlOperators.h"

struct TorrentFileInfo
{
    std::string announce;
    std::string infoHash;
    long long length;
    long long pieceLength;
    std::string fileName;
    std::vector<std::string> piecesHashes;
};

inline std::ostream& operator<<(std::ostream& os, const TorrentFileInfo& torrentFileInfo)
{
    return os << "name: " << torrentFileInfo.fileName
              << "\nannounce: " << torrentFileInfo.announce
              << "\ninfo hash: " << torrentFileInfo.infoHash
              << "\nlength: " << torrentFileInfo.length
              << "\npiece length: " << torrentFileInfo.pieceLength
              << "\npieces hashes: " << torrentFileInfo.piecesHashes;

}

inline bool operator==(const TorrentFileInfo& lhs, const TorrentFileInfo& rhs)
{
    auto tieStruct = [](const TorrentFileInfo& torrentFileInfo)
    { return std::tie(torrentFileInfo.announce, torrentFileInfo.infoHash, torrentFileInfo.length,
                      torrentFileInfo.pieceLength, torrentFileInfo.piecesHashes, torrentFileInfo.fileName); };
    return tieStruct(lhs) == tieStruct(rhs);
}
