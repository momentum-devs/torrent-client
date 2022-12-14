#pragma once

#include <gmock/gmock.h>

#include "PeerRetriever.h"

class PeerRetrieverMock : public PeerRetriever
{
public:
    MOCK_METHOD(std::string, retrievePeers, (const std::string& announceUrl), ());
};
