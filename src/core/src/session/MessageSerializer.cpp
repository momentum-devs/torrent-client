#include "MessageSerializer.h"

namespace
{
std::string intToBytes(unsigned int value)
{
    std::string bytes;

    bytes += static_cast<char>(value >> 24);
    bytes += static_cast<char>(value >> 16);
    bytes += static_cast<char>(value >> 8);
    bytes += static_cast<char>(value);

    return bytes;
}
}

namespace core
{
std::string MessageSerializer::serialize(const Message& message) const
{
    std::string serializedMessage;

    serializedMessage += intToBytes(message.payload.size() + 1);
    serializedMessage += static_cast<char>(static_cast<unsigned int>(message.id));
    serializedMessage += message.payload;

    return serializedMessage;
}

Message MessageSerializer::deserialize(const std::string& message) const
{
    auto messageId = MessageId(static_cast<unsigned int>(static_cast<unsigned char>(message[4])));

    auto messagePayload = message.substr(5, message.size() - 5);

    return Message{messageId, messagePayload};
}
}
