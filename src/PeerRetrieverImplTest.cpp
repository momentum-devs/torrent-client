#include "PeerRetrieverImpl.h"

#include "gtest/gtest.h"

using namespace ::testing;

class PeerRetrieverImplTest : public Test
{
public:
    PeerRetrieverImpl retriever;
};

TEST_F(PeerRetrieverImplTest, retrievesPeers)
{
    const auto data = retriever.retrievePeers("udp://tracker.leechers-paradise.org:6969");

    std::cout << data<< std::endl;
}
