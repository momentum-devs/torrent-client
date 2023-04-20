#include "fmt/core.h"
#include "gtest/gtest.h"

#include "fileSystem/FileSystemServiceMock.h"

#include "PieceRepositoryImpl.h"

using namespace ::testing;
using namespace core;

namespace
{
const unsigned int pieceSize{200};

const std::string destinationDirectory{"/destination_dir"};
const std::string torrentName{"torrent"};
const std::string file1Name{"file1.ext"};
const std::string file1Path{fmt::format("{}/{}/{}", destinationDirectory, torrentName, file1Name)};
const unsigned int file1Length{pieceSize * 50};
const NestedFileInfo nestedFileInfo1{file1Name, file1Length};
const std::string file2Name{"file2.ext"};
const std::string file2Path{fmt::format("{}/{}/{}", destinationDirectory, torrentName, file2Name)};
const unsigned int file2Length{pieceSize * 50};
const NestedFileInfo nestedFileInfo2{file2Name, file2Length};

const std::string metadataFilePath{fmt::format("{}/{}.metadata", destinationDirectory, torrentName)};
const unsigned int invalidPieceId{200};
const std::vector<unsigned int> piecesIds{10, 21, 33};
const std::vector<unsigned int> updatedPiecesIds{10, 21, 33, 15};
const std::vector<unsigned int> updatedPiecesIds2{10, 21, 33, 65};
const std::string serializedPiecesIds{"[10,21,33]"};
const std::string serializedUpdatedPiecesIds{"[10,21,33,15]"};
const std::string serializedUpdatedPiecesIds2{"[10,21,33,65]"};
const std::shared_ptr<TorrentFileInfo> torrentFileInfo = std::make_shared<TorrentFileInfo>();
}

class PieceRepositoryImplMultiFileTest_Base : public Test
{
public:
    PieceRepositoryImplMultiFileTest_Base()
    {
        torrentFileInfo->name = torrentName;
        torrentFileInfo->pieceLength = pieceSize;
        torrentFileInfo->length = file1Length + file2Length;
        torrentFileInfo->nestedFilesInfo = std::vector<NestedFileInfo>{nestedFileInfo1, nestedFileInfo2};

        EXPECT_CALL(*fileSystemService, exists(file1Path)).WillOnce(Return(false));
        EXPECT_CALL(*fileSystemService, write(file1Path, ""));
        EXPECT_CALL(*fileSystemService, exists(file2Path)).WillOnce(Return(false));
        EXPECT_CALL(*fileSystemService, write(file2Path, ""));
        EXPECT_CALL(*fileSystemService, exists(metadataFilePath)).WillOnce(Return(false));
        EXPECT_CALL(*fileSystemService, write(metadataFilePath, "[]"));
    }

    std::shared_ptr<libs::fileSystem::FileSystemServiceMock> fileSystemService =
        std::make_shared<StrictMock<libs::fileSystem::FileSystemServiceMock>>();
};

class PieceRepositoryImplMultiFileTest : public PieceRepositoryImplMultiFileTest_Base
{
public:
    PieceRepositoryImpl repository{fileSystemService, pieceSize, torrentFileInfo, destinationDirectory};
};

TEST_F(PieceRepositoryImplMultiFileTest, givenExistingPieceId_shouldNotSave)
{
    const unsigned int pieceId{10};
    const std::basic_string<unsigned char> data{reinterpret_cast<const unsigned char*>("data")};

    EXPECT_CALL(*fileSystemService, read(metadataFilePath)).WillOnce(Return(serializedPiecesIds));

    repository.save(pieceId, data);
}

TEST_F(PieceRepositoryImplMultiFileTest, givenNotExistingFromFile1PieceId_shouldSaveToFile1)
{
    const unsigned int pieceId{15};
    const std::basic_string<unsigned char> data{reinterpret_cast<const unsigned char*>("data")};

    EXPECT_CALL(*fileSystemService, read(metadataFilePath)).WillOnce(Return(serializedPiecesIds));
    EXPECT_CALL(*fileSystemService, writeAtPosition(file1Path, data, pieceId * pieceSize));
    EXPECT_CALL(*fileSystemService, write(metadataFilePath, serializedUpdatedPiecesIds));

    repository.save(pieceId, data);
}

TEST_F(PieceRepositoryImplMultiFileTest, givenNotExistingFromFile2PieceId_shouldSaveToFile2)
{
    const unsigned int pieceId{65};
    const unsigned int firstPieceFile2{50};
    const std::basic_string<unsigned char> data{reinterpret_cast<const unsigned char*>("data")};

    EXPECT_CALL(*fileSystemService, read(metadataFilePath)).WillOnce(Return(serializedPiecesIds));
    EXPECT_CALL(*fileSystemService, writeAtPosition(file2Path, data, (pieceId - firstPieceFile2) * pieceSize));
    EXPECT_CALL(*fileSystemService, write(metadataFilePath, serializedUpdatedPiecesIds2));

    repository.save(pieceId, data);
}

TEST_F(PieceRepositoryImplMultiFileTest, getDownloadedPieces)
{
    EXPECT_CALL(*fileSystemService, read(metadataFilePath)).WillOnce(Return(serializedPiecesIds));

    const auto allPiecesIds = repository.getDownloadedPieces();

    EXPECT_EQ(allPiecesIds, piecesIds);
}

TEST_F(PieceRepositoryImplMultiFileTest, givenExistingPieceId_shouldReturnTrue)
{
    EXPECT_CALL(*fileSystemService, read(metadataFilePath)).WillOnce(Return(serializedPiecesIds));

    const auto containsPieceId = repository.contains(10);

    EXPECT_TRUE(containsPieceId);
}

TEST_F(PieceRepositoryImplMultiFileTest, givenNotExistingPieceId_shouldReturnFalse)
{
    EXPECT_CALL(*fileSystemService, read(metadataFilePath)).WillOnce(Return(serializedPiecesIds));

    const auto containsPieceId = repository.contains(5);

    EXPECT_FALSE(containsPieceId);
}

TEST_F(PieceRepositoryImplMultiFileTest, givenPieceIdOutOfRange_shouldThrow)
{
    const std::basic_string<unsigned char> data{reinterpret_cast<const unsigned char*>("data")};

    EXPECT_CALL(*fileSystemService, read(metadataFilePath)).WillOnce(Return(serializedPiecesIds));
    EXPECT_THROW(repository.save(invalidPieceId, data), std::out_of_range);
}
