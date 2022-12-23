#pragma once

#include "HandshakeMessage.h"

class HandshakeMessageSerializer
{
public:
    std::string serialize(const HandshakeMessage& message) const;
};
