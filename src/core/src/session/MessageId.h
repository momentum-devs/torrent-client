#pragma once

#include <ostream>
#include <string>
#include <unordered_map>

namespace core
{
enum MessageId
{
    Choke,
    Unchoke,
    Interested,
    NotInterested,
    Have,
    Bitfield,
    Request,
    Piece,
    Cancel,
};

inline std::string toString(MessageId messageId)
{
    const std::unordered_map<MessageId, std::string> messageIdsToStringMapping{
        {MessageId::Choke, "Choke"},
        {MessageId::Unchoke, "Unchoke"},
        {MessageId::Interested, "Interested"},
        {MessageId::NotInterested, "NotInterested"},
        {MessageId::Have, "Have"},
        {MessageId::Bitfield, "Bitfield"},
        {MessageId::Request, "Request"},
        {MessageId::Piece, "Piece"},
        {MessageId::Cancel, "Cancel"},
    };

    try
    {
        return messageIdsToStringMapping.at(messageId);
    }
    catch (const std::out_of_range& e)
    {
        return "Undefined";
    }
}

inline std::ostream& operator<<(std::ostream& os, MessageId messageId)
{
    return os << "MessageId::" << toString(messageId) << ", value: " << static_cast<unsigned int>(messageId);
}
}
