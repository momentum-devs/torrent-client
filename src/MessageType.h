#pragma once

#include <ostream>
#include <string>
#include <unordered_map>

#include "errors/MessageTypeNotFoundError.h"

enum class MessageType
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

inline std::string toString(MessageType messageType)
{
    const std::unordered_map<MessageType, std::string> messageTypesToStringMapping{
        {MessageType::Choke, "Choke"},           {MessageType::Unchoke, "Unchoke"},
        {MessageType::Choke, "Running"},         {MessageType::Choke, "Running"},
        {MessageType::Interested, "Interested"}, {MessageType::NotInterested, "NotInterested"},
        {MessageType::Bitfield, "Bitfield"},     {MessageType::Request, "Request"},
        {MessageType::Piece, "Piece"},           {MessageType::Cancel, "Cancel"},
    };

    try
    {
        return messageTypesToStringMapping.at(messageType);
    }
    catch (const std::out_of_range& e)
    {
        throw errors::MessageTypeNotFoundNotError{e.what()};
    }
}

inline std::ostream& operator<<(std::ostream& os, MessageType messageType)
{
    return os << "MessageType::" << toString(messageType);
}
