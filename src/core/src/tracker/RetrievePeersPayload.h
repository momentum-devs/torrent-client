#pragma once

#include <ostream>
#include <string>
#include <vector>

#include "collection/StlOperators.h"

namespace core
{
struct RetrievePeersPayload
{
    std::string announceUrl;
    std::string infoHash;
    std::string peerId;
    std::string port;
    std::string uploaded;
    std::string downloaded;
    std::string left;
    std::string compact;
};

inline std::ostream& operator<<(std::ostream& os, const RetrievePeersPayload& retrievePeersPayload)
{
    return os << "announceUrl: " << retrievePeersPayload.announceUrl << "\ninfoHash: " << retrievePeersPayload.infoHash
              << "\npeerId: " << retrievePeersPayload.peerId << "\nport: " << retrievePeersPayload.port
              << "\nuploaded: " << retrievePeersPayload.uploaded << "\ndownloaded: " << retrievePeersPayload.downloaded
              << "\nleft: " << retrievePeersPayload.left << "\ncompact: " << retrievePeersPayload.compact;
}

inline bool operator==(const RetrievePeersPayload& lhs, const RetrievePeersPayload& rhs)
{
    auto tieStruct = [](const RetrievePeersPayload& retrievePeersPayload)
    {
        return std::tie(retrievePeersPayload.announceUrl, retrievePeersPayload.infoHash, retrievePeersPayload.peerId,
                        retrievePeersPayload.port, retrievePeersPayload.uploaded, retrievePeersPayload.downloaded,
                        retrievePeersPayload.left, retrievePeersPayload.compact);
    };
    
    return tieStruct(lhs) == tieStruct(rhs);
}
}
