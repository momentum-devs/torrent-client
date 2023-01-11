#pragma once

#include <ostream>
#include <string>

#include "MessageId.h"

namespace core
{
struct Message
{
    MessageId id;
    std::basic_string<unsigned char> payload;
};

inline bool operator==(const Message& lhs, const Message& rhs)
{
    return (lhs.id == rhs.id) and (lhs.payload == rhs.payload);
}

inline std::ostream& operator<<(std::ostream& os, const Message& message)
{
    return os << "id: " << message.id << " payload: " << message.payload.data();
}
}
