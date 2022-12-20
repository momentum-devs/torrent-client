#pragma once

#include <string>

class PeerToPeerSession
{
public:
    virtual void startSession(const std::string& hashInfo) = 0;
};