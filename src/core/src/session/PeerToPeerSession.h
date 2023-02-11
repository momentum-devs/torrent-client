#pragma once

#include <string>

namespace core
{
class PeerToPeerSession
{
public:
    virtual ~PeerToPeerSession() = default;

    virtual void startSession() = 0;
};
}
