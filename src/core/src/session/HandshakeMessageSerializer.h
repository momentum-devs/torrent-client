#pragma once

#include "HandshakeMessage.h"

namespace core
{
class HandshakeMessageSerializer
{
public:
    std::string serialize(const HandshakeMessage& message) const;
};
}
