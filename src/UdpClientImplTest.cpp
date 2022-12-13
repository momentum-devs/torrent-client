#include "UdpClientImpl.h"

#include "gtest/gtest.h"

using namespace ::testing;

class UdpClientImplTest : public Test
{
public:
    UdpClientImpl client;
};

TEST_F(UdpClientImplTest, sendsUdpRequest)
{
    const auto data = client.receiveData("udp://tracker.leechers-paradise.org", 6969);

    std::cout << data<< std::endl;
}
