#pragma once

#include <ostream>
#include <string>

#include "MessageType.h"

struct Message
{
    MessageId id;
    std::string payload;
};

inline bool operator==(const Message& lhs, const Message& rhs)
{
    return (lhs.id == rhs.id) and (lhs.payload == rhs.payload);
}

inline std::ostream& operator<<(std::ostream& os, const Message& message)
{
    return os << "id: " << message.id << " payload: " << message.payload;
}
