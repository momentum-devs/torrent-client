#include "HashValidator.h"

#include <boost/compute/detail/sha1.hpp>

#include "gtest/gtest.h"

#include "encoder/HexEncoder.h"

using namespace ::testing;
using namespace core;

TEST(HashValidatorTest, compareTwoHashes_nonEncoded)
{
    boost::compute::detail::sha1 data1Hash;

    std::string data1{"abc"};

    data1Hash.process(data1);

    boost::compute::detail::sha1 data2Hash;

    std::string data2{"abc"};

    data2Hash.process(data2);

    ASSERT_TRUE(HashValidator::compareHashes(static_cast<std::string>(data1Hash), static_cast<std::string>(data2Hash)));
}

TEST(HashValidatorTest, compareTwoHashes_leftEncoded)
{
    boost::compute::detail::sha1 data1Hash;

    std::string data1{"abc"};

    data1Hash.process(data1);

    boost::compute::detail::sha1 data2Hash;

    std::string data2{"abc"};

    data2Hash.process(data2);

    ASSERT_TRUE(HashValidator::compareHashes(common::encoder::HexEncoder::encode(static_cast<std::string>(data1Hash)),
                                             static_cast<std::string>(data2Hash)));
}

TEST(HashValidatorTest, compareTwoHashes_rightEncoded)
{
    boost::compute::detail::sha1 data1Hash;

    std::string data1{"abc"};

    data1Hash.process(data1);

    boost::compute::detail::sha1 data2Hash;

    std::string data2{"abc"};

    data2Hash.process(data2);

    ASSERT_TRUE(HashValidator::compareHashes(static_cast<std::string>(data1Hash),
                                             common::encoder::HexEncoder::encode(static_cast<std::string>(data2Hash))));
}

TEST(HashValidatorTest, compareTwoHashes_bothEncoded)
{
    boost::compute::detail::sha1 data1Hash;

    std::string data1{"abc"};

    data1Hash.process(data1);

    boost::compute::detail::sha1 data2Hash;

    std::string data2{"abc"};

    data2Hash.process(data2);

    ASSERT_TRUE(HashValidator::compareHashes(common::encoder::HexEncoder::encode(static_cast<std::string>(data1Hash)),
                                             common::encoder::HexEncoder::encode(static_cast<std::string>(data2Hash))));
}

TEST(HashValidatorTest, compareTwoDifferentHashes_nonEncoded)
{
    boost::compute::detail::sha1 data1Hash;

    std::string data1{"abc"};

    data1Hash.process(data1);

    boost::compute::detail::sha1 data2Hash;

    std::string data2{"abd"};

    data2Hash.process(data2);

    ASSERT_FALSE(
        HashValidator::compareHashes(static_cast<std::string>(data1Hash), static_cast<std::string>(data2Hash)));
}

TEST(HashValidatorTest, compareTwoDifferentHashes_leftEncoded)
{
    boost::compute::detail::sha1 data1Hash;

    std::string data1{"abc"};

    data1Hash.process(data1);

    boost::compute::detail::sha1 data2Hash;

    std::string data2{"abd"};

    data2Hash.process(data2);

    ASSERT_FALSE(HashValidator::compareHashes(common::encoder::HexEncoder::encode(static_cast<std::string>(data1Hash)),
                                              static_cast<std::string>(data2Hash)));
}

TEST(HashValidatorTest, compareTwoDifferentHashes_rightEncoded)
{
    boost::compute::detail::sha1 data1Hash;

    std::string data1{"abc"};

    data1Hash.process(data1);

    boost::compute::detail::sha1 data2Hash;

    std::string data2{"abd"};

    data2Hash.process(data2);

    ASSERT_FALSE(HashValidator::compareHashes(
        static_cast<std::string>(data1Hash), common::encoder::HexEncoder::encode(static_cast<std::string>(data2Hash))));
}

TEST(HashValidatorTest, compareTwoDifferentHashes_bothEncoded)
{
    boost::compute::detail::sha1 data1Hash;

    std::string data1{"abc"};

    data1Hash.process(data1);

    boost::compute::detail::sha1 data2Hash;

    std::string data2{"abd"};

    data2Hash.process(data2);

    ASSERT_FALSE(
        HashValidator::compareHashes(common::encoder::HexEncoder::encode(static_cast<std::string>(data1Hash)),
                                     common::encoder::HexEncoder::encode(static_cast<std::string>(data2Hash))));
}

TEST(HashValidatorTest, compareHashWithData)
{
    boost::compute::detail::sha1 dataHash;

    std::string data{"abc"};

    dataHash.process(data);

    std::basic_string<unsigned char> bytesData{'a', 'b', 'c'};

    ASSERT_TRUE(HashValidator::compareHashWithData(static_cast<std::string>(dataHash), bytesData));
}

TEST(HashValidatorTest, compareHashWithWrongData)
{
    boost::compute::detail::sha1 dataHash;

    std::string data{"abc"};

    dataHash.process(data);

    std::basic_string<unsigned char> bytesData{'a', 'b', 'd'};

    ASSERT_FALSE(HashValidator::compareHashWithData(static_cast<std::string>(dataHash), bytesData));
}
