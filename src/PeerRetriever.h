#pragma once

#include <string>

class PeerRetriever
{
public:
    virtual ~PeerRetriever() = default;

    virtual std::string retrievePeers(const std::string& announceUrl) = 0;
};
