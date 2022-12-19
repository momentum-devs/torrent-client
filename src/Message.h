#pragma once

#include <ostream>
#include <string>

#include "MessageType.h"

struct Message
{
    MessageType messageType;
    std::string payload;
};

inline bool operator==(const Message& lhs, const Message& rhs)
{
    return (lhs.messageType == rhs.messageType) and (lhs.payload == rhs.payload);
}

inline std::ostream& operator<<(std::ostream& os, const Message& message)
{
    return os << "messageType: " << message.messageType << " payload: " << message.payload;
}
