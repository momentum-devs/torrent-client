#include "PieceRepositoryImpl.h"

#include "gtest/gtest.h"

#include "fileSystem/FileSystemServiceMock.h"
#include "PiecesSerializerMock.h"

using namespace ::testing;
using namespace core;

namespace
{
const unsigned int pieceSize{200};
const std::string dataFilePath{"/data"};
const std::string metaDataFilePath{"/metadata"};
const std::vector<unsigned int> piecesIds{10, 21, 33};
const std::vector<unsigned int> updatedPiecesIds{10, 21, 33, 15};
const std::string serializedPiecesIds{"[10,21,33]"};
const std::string serializedUpdatedPiecesIds{"[10,21,33, 15]"};
}

class PieceRepositoryImplTest : public Test
{
public:
    std::shared_ptr<libs::fileSystem::FileSystemServiceMock> fileSystemService =
        std::make_shared<StrictMock<libs::fileSystem::FileSystemServiceMock>>();
    std::shared_ptr<PiecesSerializerMock> serializer = std::make_shared<StrictMock<PiecesSerializerMock>>();

    PieceRepositoryImpl repository{fileSystemService, serializer, pieceSize, dataFilePath, metaDataFilePath};
};

TEST_F(PieceRepositoryImplTest, givenExistingPieceId_shouldNotSave)
{
    const unsigned int pieceId{10};
    const std::basic_string<unsigned char> data{reinterpret_cast<const unsigned char*>("data")};

    EXPECT_CALL(*fileSystemService, read(metaDataFilePath)).WillOnce(Return(serializedPiecesIds));
    EXPECT_CALL(*serializer, deserialize(serializedPiecesIds)).WillOnce(Return(piecesIds));

    repository.save(pieceId, data);
}

TEST_F(PieceRepositoryImplTest, givenNotExistingPieceId_shouldSave)
{
    const unsigned int pieceId{15};
    const std::basic_string<unsigned char> data{reinterpret_cast<const unsigned char*>("data")};

    EXPECT_CALL(*fileSystemService, read(metaDataFilePath)).WillOnce(Return(serializedPiecesIds));
    EXPECT_CALL(*serializer, deserialize(serializedPiecesIds)).WillOnce(Return(piecesIds));
    EXPECT_CALL(*fileSystemService, writeAtPosition(dataFilePath, data, pieceId * pieceSize));
    EXPECT_CALL(*serializer, serialize(updatedPiecesIds)).WillOnce(Return(serializedUpdatedPiecesIds));
    EXPECT_CALL(*fileSystemService, write(metaDataFilePath, serializedUpdatedPiecesIds));

    repository.save(pieceId, data);
}

TEST_F(PieceRepositoryImplTest, findAllPiecesIds)
{
    EXPECT_CALL(*fileSystemService, read(metaDataFilePath)).WillOnce(Return(serializedPiecesIds));
    EXPECT_CALL(*serializer, deserialize(serializedPiecesIds)).WillOnce(Return(piecesIds));

    const auto allPiecesIds = repository.findAllPiecesIds();

    EXPECT_EQ(allPiecesIds, piecesIds);
}

TEST_F(PieceRepositoryImplTest, givenExistingPieceId_shouldReturnTrue)
{
    EXPECT_CALL(*fileSystemService, read(metaDataFilePath)).WillOnce(Return(serializedPiecesIds));
    EXPECT_CALL(*serializer, deserialize(serializedPiecesIds)).WillOnce(Return(piecesIds));

    const auto containsPieceId = repository.contains(10);

    EXPECT_TRUE(containsPieceId);
}

TEST_F(PieceRepositoryImplTest, givenNotExistingPieceId_shouldReturnFalse)
{
    EXPECT_CALL(*fileSystemService, read(metaDataFilePath)).WillOnce(Return(serializedPiecesIds));
    EXPECT_CALL(*serializer, deserialize(serializedPiecesIds)).WillOnce(Return(piecesIds));

    const auto containsPieceId = repository.contains(5);

    EXPECT_FALSE(containsPieceId);
}
