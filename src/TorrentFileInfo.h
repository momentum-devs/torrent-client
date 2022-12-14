#pragma once

#include <string>
#include <ostream>

struct TorrentFileInfo{
    std::string announce;
    std::string infoHash;
    long long size;
};

inline std::ostream& operator<<(std::ostream& os, const TorrentFileInfo& torrentFileInfo)
{
    return os << "announce: " << torrentFileInfo.announce << " info hash: " << torrentFileInfo.infoHash << " size: " << torrentFileInfo.size;
}

inline bool operator==(const TorrentFileInfo& lhs, const TorrentFileInfo& rhs)
{
    return (lhs.infoHash == rhs.infoHash);
}

