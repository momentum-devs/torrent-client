#pragma once

#include <string>

class PeerToPeerSession
{
public:
    virtual ~PeerToPeerSession() = default;
    
    virtual void startSession(const std::string& hashInfo) = 0;
};
