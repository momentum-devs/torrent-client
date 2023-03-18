#include "MessageSerializer.h"

#include "gtest/gtest.h"

using namespace ::testing;
using namespace core;

namespace
{
const auto payload = std::basic_string<unsigned char>(reinterpret_cast<const unsigned char*>("payload"));
const auto serializedUnchokeMessage = std::basic_string<unsigned char>{MessageId::Unchoke};
const auto serializedBitfieldMessage = std::basic_string<unsigned char>{MessageId::Bitfield} + payload;
}

class MessageSerializerTest : public Test
{
public:
    MessageSerializer serializer;
};

TEST_F(MessageSerializerTest, serializeUnchokeMessageWithoutPayload)
{
    const auto message = core::Message{MessageId::Unchoke, std::basic_string<unsigned char>{}};

    const auto serializedMessage = serializer.serialize(message);

    ASSERT_EQ(serializedMessage.size(), 5);
    ASSERT_EQ(static_cast<int>(serializedMessage[0]), 0);
    ASSERT_EQ(static_cast<int>(serializedMessage[1]), 0);
    ASSERT_EQ(static_cast<int>(serializedMessage[2]), 0);
    ASSERT_EQ(static_cast<int>(serializedMessage[3]), 1);
    ASSERT_EQ(static_cast<int>(serializedMessage[4]), 1);
}

TEST_F(MessageSerializerTest, serializeUnchokeMessage)
{
    const auto message = core::Message{MessageId::Unchoke, payload};

    const auto serializedMessage = serializer.serialize(message);

    ASSERT_EQ(serializedMessage.size(), 12);
    ASSERT_EQ(static_cast<int>(serializedMessage[0]), 0);
    ASSERT_EQ(static_cast<int>(serializedMessage[1]), 0);
    ASSERT_EQ(static_cast<int>(serializedMessage[2]), 0);
    ASSERT_EQ(static_cast<int>(serializedMessage[3]), 8);
    ASSERT_EQ(static_cast<int>(serializedMessage[4]), 1);
    ASSERT_EQ(serializedMessage.substr(5), payload);
}

TEST_F(MessageSerializerTest, serializeInterestedMessage)
{
    const auto message = core::Message{MessageId::Interested, payload};

    const auto serializedMessage = serializer.serialize(message);

    ASSERT_EQ(serializedMessage.size(), 12);
    ASSERT_EQ(static_cast<int>(serializedMessage[0]), 0);
    ASSERT_EQ(static_cast<int>(serializedMessage[1]), 0);
    ASSERT_EQ(static_cast<int>(serializedMessage[2]), 0);
    ASSERT_EQ(static_cast<int>(serializedMessage[3]), 8);
    ASSERT_EQ(static_cast<int>(serializedMessage[4]), 2);
    ASSERT_EQ(serializedMessage.substr(5), payload);
}

TEST_F(MessageSerializerTest, deserializeUnchokeMessage)
{
    const auto message = serializer.deserialize(serializedUnchokeMessage);

    ASSERT_EQ(message.id, MessageId::Unchoke);
    ASSERT_TRUE(message.payload.empty());
}

TEST_F(MessageSerializerTest, deserializeBitfieldMessage)
{
    const auto message = serializer.deserialize(serializedBitfieldMessage);

    ASSERT_EQ(message.id, MessageId::Bitfield);
    ASSERT_EQ(message.payload, payload);
}
