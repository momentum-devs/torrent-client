#include "PeerRetrieverImpl.h"

#include "gtest/gtest.h"

#include "PeerIdGenerator.h"

using namespace ::testing;

class PeerRetrieverImplTest : public Test
{
public:
    PeerRetrieverImpl retriever;
};

TEST_F(PeerRetrieverImplTest, retrievesPeers)
{
    const auto announceUrl = "http://bttracker.debian.org:6969/announce";
    const auto hashInfo = "b3f4b20f822c471cfd97745abb98e69b4fe0986e";
    const auto peerId = PeerIdGenerator::generate();
    const auto port = "0";
    const auto uploaded = "0";
    const auto downloaded = "0";
    const auto left = "399507456";
    const auto compact = "1";

    const auto payload = RetrievePeersPayload{announceUrl, hashInfo, peerId, port, uploaded, downloaded, left, compact};

    const auto data = retriever.retrievePeers(payload);

    std::cout << data << std::endl;
}
