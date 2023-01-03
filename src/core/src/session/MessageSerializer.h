#pragma once

#include "Message.h"

namespace core
{
class MessageSerializer
{
public:
    std::string serialize(const Message& message) const;
    Message deserialize(const std::string& message) const;
};
}
