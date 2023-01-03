#include "HandshakeMessageSerializer.h"

#include "encoder/HexEncoder.h"

namespace core
{
std::string HandshakeMessageSerializer::serialize(const HandshakeMessage& message) const
{
    std::string serializedMessage;

    serializedMessage += static_cast<char>(message.protocolIdentifier.size());
    serializedMessage += message.protocolIdentifier;
    serializedMessage += std::string(8, '\0');
    serializedMessage += common::encoder::HexEncoder::decode(message.infoHash);
    serializedMessage += message.peerId;

    return serializedMessage;
}
}
