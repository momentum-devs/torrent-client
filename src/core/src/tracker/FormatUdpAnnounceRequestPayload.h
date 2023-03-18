#pragma once

#include <ostream>
#include <string>
#include <vector>

#include "collection/StlOperators.h"

namespace core
{
struct FormatUdpAnnounceRequestPayload
{
    std::string infoHash;
    std::string peerId;
    std::string port;
    std::string uploaded;
    std::string downloaded;
    std::string left;
    std::basic_string<unsigned char> connectionId;
};

inline std::ostream& operator<<(std::ostream& os, const FormatUdpAnnounceRequestPayload& payload)
{
    return os << "\ninfoHash: " << payload.infoHash << "\npeerId: " << payload.peerId << "\nport: " << payload.port
              << "\nuploaded: " << payload.uploaded << "\ndownloaded: " << payload.downloaded
              << "\nleft: " << payload.left << "\nconnectionId: " << payload.connectionId.c_str();
}

inline bool operator==(const FormatUdpAnnounceRequestPayload& lhs, const FormatUdpAnnounceRequestPayload& rhs)
{
    const auto tieStruct = [](const FormatUdpAnnounceRequestPayload& payload)
    {
        return std::tie(payload.infoHash, payload.peerId, payload.port, payload.uploaded, payload.downloaded,
                        payload.left, payload.connectionId);
    };

    return tieStruct(lhs) == tieStruct(rhs);
}
}
