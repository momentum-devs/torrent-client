#include "HandshakeMessageSerializer.h"

#include "../HexEncoder.h"

std::string HandshakeMessageSerializer::serialize(const HandshakeMessage& message) const
{
    std::string serializedMessage;

    serializedMessage += static_cast<char>(message.protocolIdentifier.size());
    serializedMessage += message.protocolIdentifier;
    serializedMessage += std::string(8, '\0');
    serializedMessage += HexEncoder::decode(message.infoHash);
    serializedMessage += message.peerId;

    return serializedMessage;
}
