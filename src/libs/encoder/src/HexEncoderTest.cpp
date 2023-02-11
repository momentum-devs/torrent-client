#include "HexEncoder.h"

#include "gtest/gtest.h"

using namespace ::testing;
using namespace libs::encoder;

TEST(HexEncoderTest, encode)
{
    const auto encoded = HexEncoder::encode("hello");

    ASSERT_EQ(encoded, "68656c6c6f");
}

TEST(HexEncoderTest, decode)
{
    const auto encoded = HexEncoder::decode("627965");

    ASSERT_EQ(encoded, "bye");
}
