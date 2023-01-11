#pragma once

#include "Message.h"

namespace core
{
class MessageSerializer
{
public:
    std::basic_string<unsigned char> serialize(const Message& message) const;
    Message deserialize(const std::basic_string<unsigned char>& message) const;
};
}
