#include "MessageSerializer.h"

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
