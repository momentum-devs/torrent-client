#include "MessageSerializer.h"

#include "bytes/BytesConverter.h"

namespace core
{
std::basic_string<unsigned char> MessageSerializer::serialize(const Message& message) const
{
    std::basic_string<unsigned char> serializedMessage;

    serializedMessage += common::bytes::BytesConverter::intToBytes(message.payload.size() + 1);
    serializedMessage += static_cast<unsigned char>(static_cast<unsigned int>(message.id));
    serializedMessage += message.payload;

    return serializedMessage;
}

Message MessageSerializer::deserialize(const std::basic_string<unsigned char>& message) const
{
    const auto messageId = MessageId(static_cast<unsigned int>(static_cast<unsigned char>(message[4])));

    const auto messagePayload = message.substr(5, message.size() - 5);

    return Message{messageId, messagePayload};
}
}
