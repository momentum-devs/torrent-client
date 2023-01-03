#pragma once

#include <gmock/gmock.h>

#include "PeersRetriever.h"

namespace core
{
class PeerRetrieverMock : public PeerRetriever
{
public:
    MOCK_METHOD(std::string, retrievePeers, (const std::string& announceUrl), ());
};
}
