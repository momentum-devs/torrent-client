#pragma once

#include <ostream>
#include <string>

namespace core
{
struct HandshakeMessage
{
    std::string protocolIdentifier;
    std::string infoHash;
    std::string peerId;
};

inline bool operator==(const HandshakeMessage& lhs, const HandshakeMessage& rhs)
{
    return (lhs.protocolIdentifier == rhs.protocolIdentifier) and (lhs.infoHash == rhs.infoHash) and
           (lhs.peerId == rhs.peerId);
}

inline std::ostream& operator<<(std::ostream& os, const HandshakeMessage& handshakeMessage)
{
    return os << "protocolIdentifier: " << handshakeMessage.protocolIdentifier
              << " infoHash: " << handshakeMessage.infoHash << " peerId: " << handshakeMessage.peerId;
}
}
