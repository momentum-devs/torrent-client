#include "HandshakeMessageSerializer.h"

#include "gtest/gtest.h"

#include "HexEncoder.h"
#include "PeerIdGenerator.h"

using namespace ::testing;

namespace
{
const std::string protocolIdentifier = "BitTorrent protocol";
const auto peerId = PeerIdGenerator::generate();
const std::string infoHash = "b3f4b20f822c471cfd97745abb98e69b4fe0986e";
const auto decodedInfoHash = HexEncoder::decode(infoHash);
}

class HandshakeMessageSerializerTest : public Test
{
public:
    HandshakeMessageSerializer serializer;
};

TEST_F(HandshakeMessageSerializerTest, serialize)
{
    const auto handshakeMessage = HandshakeMessage{protocolIdentifier, infoHash, peerId};

    const auto serializedHandshakeMessage = serializer.serialize(handshakeMessage);

    ASSERT_EQ(serializedHandshakeMessage.size(), 68);
    ASSERT_EQ(static_cast<int>(serializedHandshakeMessage[0]), 19);
    ASSERT_EQ(serializedHandshakeMessage.substr(1, 19), protocolIdentifier);
    ASSERT_EQ(serializedHandshakeMessage.substr(20, 8), std::string(8, '\0'));
    ASSERT_EQ(serializedHandshakeMessage.substr(28, 20), decodedInfoHash);
    ASSERT_EQ(serializedHandshakeMessage.substr(48, 20), peerId);
}
