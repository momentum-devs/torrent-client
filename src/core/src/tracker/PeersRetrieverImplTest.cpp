#include "PeersRetrieverImpl.h"

#include "gtest/gtest.h"

#include "AnnounceResponseDeserializerMock.h"
#include "httpClient/HttpClientMock.h"

#include "../client/PeerIdGenerator.h"
#include "encoder/HexEncoder.h"

using namespace ::testing;
using namespace core;

namespace
{
const auto announceUrl = "http://bttracker.debian.org:6969/announce";
const auto infoHash = "b3f4b20f822c471cfd97745abb98e69b4fe0986e";
const auto peerId = PeerIdGenerator::generate();
const auto port = "0";
const auto uploaded = "0";
const auto downloaded = "0";
const auto left = "399507456";
const auto compact = "1";
const auto queryParameters =
    std::map<std::string, std::string>{{"info_hash", common::encoder::HexEncoder::decode(infoHash)},
                                       {"peer_id", peerId},
                                       {"port", port},
                                       {"uploaded", uploaded},
                                       {"downloaded", downloaded},
                                       {"left", left},
                                       {"compact", compact}};
const auto httpGetPayload = common::httpClient::HttpGetRequestPayload{announceUrl, std::nullopt, queryParameters};
const auto responseBody = "body";
const auto httpGetResponse = common::httpClient::HttpResponse{200, responseBody};
const auto peersEndpoints = std::vector<PeerEndpoint>{{"address", 112}};
const auto deserializedResponse = RetrievePeersResponse{900, peersEndpoints};
}

class PeerRetrieverImplTest : public Test
{
public:
    std::unique_ptr<common::httpClient::HttpClientMock> httpClientMockInit =
        std::make_unique<common::httpClient::HttpClientMock>();
    common::httpClient::HttpClientMock* httpClientMock = httpClientMockInit.get();

    std::unique_ptr<AnnounceResponseDeserializerMock> responseDeserializerMockInit =
        std::make_unique<AnnounceResponseDeserializerMock>();
    AnnounceResponseDeserializerMock* responseDeserializerMock = responseDeserializerMockInit.get();

    PeersRetrieverImpl retriever{std::move(httpClientMockInit), std::move(responseDeserializerMockInit)};
};

TEST_F(PeerRetrieverImplTest, retrievesPeers)
{
    EXPECT_CALL(*httpClientMock, get(httpGetPayload)).WillOnce(Return(httpGetResponse));

    EXPECT_CALL(*responseDeserializerMock, deserialize(responseBody)).WillOnce(Return(deserializedResponse));

    const auto payload = RetrievePeersPayload{announceUrl, infoHash, peerId, port, uploaded, downloaded, left, compact};

    const auto data = retriever.retrievePeers(payload);

    ASSERT_EQ(data, deserializedResponse);
}
