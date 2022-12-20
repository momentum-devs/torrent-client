#pragma once

#include "Message.h"

class MessageSerializer
{
public:
    std::string serialize(const Message& message) const;
    Message deserialize(const std::string& message) const;
};
