#include "fmt/core.h"
#include "gtest/gtest.h"

#include "fileSystem/FileSystemServiceMock.h"
#include "PiecesSerializerMock.h"

#include "PieceRepositoryImpl.h"

using namespace ::testing;
using namespace core;

namespace
{
const unsigned int pieceSize{200};
const unsigned int fileLength{200 * 50};
const std::string destinationDirectory{"/destination_dir"};
const std::string fileName{"file.ext"};
const std::string filePath{fmt::format("{}/{}", destinationDirectory, fileName)};
const std::string metadataFilePath{fmt::format("{}/{}.metadata", destinationDirectory, fileName)};

const unsigned int invalidPieceId{200};
const std::vector<unsigned int> piecesIds{10, 21, 33};
const std::vector<unsigned int> updatedPiecesIds{10, 21, 33, 15};
const std::string serializedPiecesIds{"[10,21,33]"};
const std::string serializedUpdatedPiecesIds{"[10,21,33, 15]"};
const std::shared_ptr<TorrentFileInfo> torrentFileInfo = std::make_shared<TorrentFileInfo>();
}

class PieceRepositoryImplSingleFileTest_Base : public Test
{
public:
    PieceRepositoryImplSingleFileTest_Base()
    {
        torrentFileInfo->name = fileName;
        torrentFileInfo->pieceLength = pieceSize;
        torrentFileInfo->length = fileLength;
        EXPECT_CALL(*fileSystemService, exists(filePath)).WillOnce(Return(false));
        EXPECT_CALL(*fileSystemService, write(filePath, ""));
        EXPECT_CALL(*fileSystemService, exists(metadataFilePath)).WillOnce(Return(false));
        EXPECT_CALL(*fileSystemService, write(metadataFilePath, "[]"));
    }

    std::shared_ptr<libs::fileSystem::FileSystemServiceMock> fileSystemService =
        std::make_shared<StrictMock<libs::fileSystem::FileSystemServiceMock>>();
    std::shared_ptr<PiecesSerializerMock> piecesSerializer = std::make_shared<StrictMock<PiecesSerializerMock>>();
};

class PieceRepositoryImplSingleFileTest : public PieceRepositoryImplSingleFileTest_Base
{
public:
    PieceRepositoryImpl repository{fileSystemService, piecesSerializer, pieceSize, torrentFileInfo,
                                   destinationDirectory};
};

TEST_F(PieceRepositoryImplSingleFileTest, givenExistingPieceId_shouldNotSave)
{
    const unsigned int pieceId{10};
    const std::basic_string<unsigned char> data{reinterpret_cast<const unsigned char*>("data")};

    EXPECT_CALL(*fileSystemService, read(metadataFilePath)).WillOnce(Return(serializedPiecesIds));
    EXPECT_CALL(*piecesSerializer, deserialize(serializedPiecesIds)).WillOnce(Return(piecesIds));

    repository.save(pieceId, data);
}

TEST_F(PieceRepositoryImplSingleFileTest, givenNotExistingPieceId_shouldSave)
{
    const unsigned int pieceId{15};
    const std::basic_string<unsigned char> data{reinterpret_cast<const unsigned char*>("data")};

    EXPECT_CALL(*fileSystemService, read(metadataFilePath)).WillOnce(Return(serializedPiecesIds));
    EXPECT_CALL(*piecesSerializer, deserialize(serializedPiecesIds)).WillOnce(Return(piecesIds));
    EXPECT_CALL(*fileSystemService, writeAtPosition(filePath, data, pieceId * pieceSize));
    EXPECT_CALL(*piecesSerializer, serialize(updatedPiecesIds)).WillOnce(Return(serializedUpdatedPiecesIds));
    EXPECT_CALL(*fileSystemService, write(metadataFilePath, serializedUpdatedPiecesIds));

    repository.save(pieceId, data);
}

TEST_F(PieceRepositoryImplSingleFileTest, getDownloadedPieces)
{
    EXPECT_CALL(*fileSystemService, read(metadataFilePath)).WillOnce(Return(serializedPiecesIds));
    EXPECT_CALL(*piecesSerializer, deserialize(serializedPiecesIds)).WillOnce(Return(piecesIds));

    const auto allPiecesIds = repository.getDownloadedPieces();

    EXPECT_EQ(allPiecesIds, piecesIds);
}

TEST_F(PieceRepositoryImplSingleFileTest, givenExistingPieceId_shouldReturnTrue)
{
    EXPECT_CALL(*fileSystemService, read(metadataFilePath)).WillOnce(Return(serializedPiecesIds));
    EXPECT_CALL(*piecesSerializer, deserialize(serializedPiecesIds)).WillOnce(Return(piecesIds));

    const auto containsPieceId = repository.contains(10);

    EXPECT_TRUE(containsPieceId);
}

TEST_F(PieceRepositoryImplSingleFileTest, givenNotExistingPieceId_shouldReturnFalse)
{
    EXPECT_CALL(*fileSystemService, read(metadataFilePath)).WillOnce(Return(serializedPiecesIds));
    EXPECT_CALL(*piecesSerializer, deserialize(serializedPiecesIds)).WillOnce(Return(piecesIds));

    const auto containsPieceId = repository.contains(5);

    EXPECT_FALSE(containsPieceId);
}

TEST_F(PieceRepositoryImplSingleFileTest, givenPieceIdOutOfRange_shouldThrow)
{
    const std::basic_string<unsigned char> data{reinterpret_cast<const unsigned char*>("data")};

    EXPECT_CALL(*fileSystemService, read(metadataFilePath)).WillOnce(Return(serializedPiecesIds));
    EXPECT_CALL(*piecesSerializer, deserialize(serializedPiecesIds)).WillOnce(Return(piecesIds));
    EXPECT_THROW(repository.save(invalidPieceId, data), std::out_of_range);
}
