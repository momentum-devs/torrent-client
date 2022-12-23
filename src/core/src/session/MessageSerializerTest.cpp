#include "MessageSerializer.h"

#include "gtest/gtest.h"

using namespace ::testing;

namespace
{
const auto payload = std::string{"payload"};
const auto serializedUnchokeMessage = std::string{0, 0, 0, 1, MessageId::Unchoke};
const auto serializedBitfieldMessage =
    std::string{0, 0, 0, static_cast<char>(1 + payload.size()), MessageId::Bitfield} + payload;
}

class MessageSerializerTest : public Test
{
public:
    MessageSerializer serializer;
};

TEST_F(MessageSerializerTest, serializeUnchokeMessageWithoutPayload)
{
    const auto message = ::Message{MessageId::Unchoke, ""};

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
    const auto message = ::Message{MessageId::Unchoke, payload};

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
    const auto message = ::Message{MessageId::Interested, payload};

    const auto serializedMessage = serializer.serialize(message);

    ASSERT_EQ(serializedMessage.size(), 12);
    ASSERT_EQ(static_cast<int>(serializedMessage[0]), 0);
    ASSERT_EQ(static_cast<int>(serializedMessage[1]), 0);
    ASSERT_EQ(static_cast<int>(serializedMessage[2]), 0);
    ASSERT_EQ(static_cast<int>(serializedMessage[3]), 8);
    ASSERT_EQ(static_cast<int>(serializedMessage[4]), 2);
    ASSERT_EQ(serializedMessage.substr(5), payload);
}

TEST_F(MessageSerializerTest, deserializeOutputFromSerializer)
{
    const auto message = ::Message{MessageId::Interested, payload};

    const auto serializedMessage = serializer.serialize(message);

    ASSERT_EQ(serializer.deserialize(serializedMessage), message);
}

TEST_F(MessageSerializerTest, deserializeUnchokeMessage)
{
    const auto message = serializer.deserialize(serializedUnchokeMessage);

    ASSERT_EQ(message.id, MessageId::Unchoke);
    ASSERT_EQ(message.payload, "");
}

TEST_F(MessageSerializerTest, deserializeBitfieldMessage)
{
    const auto message = serializer.deserialize(serializedBitfieldMessage);

    ASSERT_EQ(message.id, MessageId::Bitfield);
    ASSERT_EQ(message.payload, payload);
}
