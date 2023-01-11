#include "BytesConverter.h"

#include "gtest/gtest.h"

using namespace ::testing;
using namespace common::bytes;

TEST(BytesConverterTest, convertsIntegerIntoBytes)
{
    const auto result = BytesConverter::intToBytes(19);

    ASSERT_EQ(result[0], '\0');
    ASSERT_EQ(result[1], '\0');
    ASSERT_EQ(result[2], '\0');
    ASSERT_EQ(result[3], '\x13');
}

TEST(BytesConverterTest, convertsBytesIntoInteger)
{
    const auto bytes = std::basic_string<unsigned char>(3, '\0') + static_cast<unsigned char>(0x7f);

    const auto result = BytesConverter::bytesToInt(bytes);

    ASSERT_EQ(result, 127);
}

TEST(BytesConverterTest, givenBytesWithInvalidNumberOfBytes_shouldThrow)
{
    const auto bytes = std::basic_string<unsigned char>(2, '\0') + static_cast<unsigned char>(0x7f);

    ASSERT_THROW(BytesConverter::bytesToInt(bytes), std::invalid_argument);
}
