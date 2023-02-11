#include "PiecesSerializerImpl.h"

#include "gtest/gtest.h"

#include "errors/InvalidJsonError.h"

using namespace ::testing;
using namespace core;

namespace
{
const std::string invalidJson{"{."};
const std::vector<unsigned int> piecesIds{1, 4, 10};
const std::string serializedPiecesIds{R"([1,4,10])"};
}

class PiecesSerializerImplTest : public Test
{
public:
    PiecesSerializerImpl serializer;
};

TEST_F(PiecesSerializerImplTest, givenNoPiecesIds_shouldReturnEmptyJson)
{
    const auto actualSerializedPiecesIds = serializer.serialize({});

    EXPECT_EQ(actualSerializedPiecesIds, "[]");
}

TEST_F(PiecesSerializerImplTest, givenPiecesIds_shouldReturnSerializedPiecesIds)
{
    const auto actualSerializedPiecesIds = serializer.serialize(piecesIds);

    EXPECT_EQ(actualSerializedPiecesIds, serializedPiecesIds);
}

TEST_F(PiecesSerializerImplTest, givenInvalidJson_shouldThrow)
{
    ASSERT_THROW(serializer.deserialize(invalidJson), exceptions::InvalidJsonError);
}

TEST_F(PiecesSerializerImplTest, givenSerializedPiecesIdsJson_shouldReturnPiecesIds)
{
    const auto actualPiecesIds = serializer.deserialize(serializedPiecesIds);

    EXPECT_EQ(actualPiecesIds, piecesIds);
}

TEST_F(PiecesSerializerImplTest, givenEmptyJsonshouldReturnEmptyPiecesIds)
{
    const auto actualStatistics = serializer.deserialize("[]");

    EXPECT_TRUE(actualStatistics.empty());
}
