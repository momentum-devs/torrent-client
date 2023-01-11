#include "Bitfield.h"

#include "gtest/gtest.h"

using namespace ::testing;
using namespace common::bytes;

namespace
{
const auto dataSize = 4;
const auto dataWithZeros = std::basic_string<unsigned char>(dataSize, static_cast<unsigned char>(0x00));
const auto dataWithOnes = std::basic_string<unsigned char>(dataSize, static_cast<unsigned char>(0xff));
const auto data = std::basic_string<unsigned char>(1, static_cast<unsigned char>(0x7f));
}

class BitfieldTest : public Test
{
public:
    Bitfield bitfieldWithAllZeros{dataWithZeros};
    Bitfield bitfieldWithAllOnes{dataWithOnes};
    Bitfield bitfield{data};
};

TEST_F(BitfieldTest, givenBitfieldWithZeros_shouldReturnAlwaysFalse)
{
    for (auto i = 0; i < dataSize; i++)
    {
        ASSERT_FALSE(bitfieldWithAllZeros.hasBitSet(i));
    }
}

TEST_F(BitfieldTest, givenBitfieldWithOnes_shouldReturnAlwaysTrue)
{
    for (auto i = 0; i < dataSize; i++)
    {
        ASSERT_TRUE(bitfieldWithAllOnes.hasBitSet(i));
    }
}

TEST_F(BitfieldTest, givenBitfieldWithOneZerosAndOtherOnes_shouldReturnCorrectBitsAsSet)
{
    ASSERT_FALSE(bitfield.hasBitSet(0));
    ASSERT_TRUE(bitfield.hasBitSet(1));
    ASSERT_TRUE(bitfield.hasBitSet(2));
    ASSERT_TRUE(bitfield.hasBitSet(3));
    ASSERT_TRUE(bitfield.hasBitSet(4));
    ASSERT_TRUE(bitfield.hasBitSet(5));
    ASSERT_TRUE(bitfield.hasBitSet(6));
    ASSERT_TRUE(bitfield.hasBitSet(7));
}
