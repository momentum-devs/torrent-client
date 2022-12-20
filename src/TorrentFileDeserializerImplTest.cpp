#include "TorrentFileDeserializerImpl.h"

#include "gtest/gtest.h"

#include "errors/InvalidBencodeFileFormatError.h"
#include "errors/MissingTorrentInfo.h"
#include "fileSystem/FileSystemServiceFactory.h"
#include "fileSystem/GetProjectPath.h"

using namespace ::testing;

namespace
{
std::unique_ptr<common::fileSystem::FileSystemService> fileSystemService =
    common::fileSystem::FileSystemServiceFactory().createFileSystemService();
const auto projectPath = common::fileSystem::getProjectPath("torrent-client");
const auto testFileDirectoryPath = projectPath + "src/testFiles/";
const auto validTorrentFilePath = testFileDirectoryPath + "valid.torrent";
const auto torrentFileWithoutAnnounceFieldPath = testFileDirectoryPath + "fileWithoutAnnounceField.torrent";
const auto torrentFileWithoutInfoFieldPath = testFileDirectoryPath + "fileWithoutInfoField.torrent";
const auto torrentFileWithoutLengthFieldPath = testFileDirectoryPath + "fileWithoutLengthField.torrent";
const auto torrentFileWithoutNameFieldPath = testFileDirectoryPath + "fileWithoutNameField.torrent";
const auto torrentFileWithoutPieceLengthFieldPath = testFileDirectoryPath + "fileWithoutPieceLengthField.torrent";
const auto torrentFileWithoutPiecesFieldPath = testFileDirectoryPath + "fileWithoutPiecesField.torrent";
const auto invalidBencodeFilePath = testFileDirectoryPath + "invalidBencodeFile.torrent";

const auto validTorrentFileContent = fileSystemService->read(validTorrentFilePath);
const auto torrentFileWithoutAnnounceFieldContent = fileSystemService->read(torrentFileWithoutAnnounceFieldPath);
const auto torrentFileWithoutInfoFieldContent = fileSystemService->read(torrentFileWithoutInfoFieldPath);
const auto torrentFileWithoutLengthFieldContent = fileSystemService->read(torrentFileWithoutLengthFieldPath);
const auto torrentFileWithoutNameFieldContent = fileSystemService->read(torrentFileWithoutNameFieldPath);
const auto torrentFileWithoutPieceLengthFieldContent = fileSystemService->read(torrentFileWithoutPieceLengthFieldPath);
const auto torrentFileWithoutPiecesFieldContent = fileSystemService->read(torrentFileWithoutPiecesFieldPath);
const auto invalidBencodeFileContent = fileSystemService->read(invalidBencodeFilePath);
}

class TorrentFileDeserializerImplTest : public Test
{
public:
    TorrentFileDeserializerImpl torrentFileDeserializer;
};

TEST_F(TorrentFileDeserializerImplTest, deserializeTorrentFile)
{
    const auto torrentFileInfo = torrentFileDeserializer.deserialize(validTorrentFileContent);

    ASSERT_EQ(torrentFileInfo.announce, "http://bttracker.debian.org:6969/announce");
    ASSERT_EQ(torrentFileInfo.infoHash, "b3f4b20f822c471cfd97745abb98e69b4fe0986e");
    ASSERT_EQ(torrentFileInfo.length, 399507456);
    ASSERT_EQ(torrentFileInfo.pieceLength, 262144);
    ASSERT_EQ(torrentFileInfo.piecesHashes[0], "3fbdf5be0c348bb673cca7eab8eb44f7a2c0f409");
    ASSERT_EQ(torrentFileInfo.piecesHashes.size(), 1524);
}

TEST_F(TorrentFileDeserializerImplTest, givenTorrentFileWithoutAnnounceField_throwsAnError)
{
    ASSERT_THROW(torrentFileDeserializer.deserialize(torrentFileWithoutAnnounceFieldContent),
                 errors::MissingTorrentInfo);
}

TEST_F(TorrentFileDeserializerImplTest, givenTorrentFileWithoutInfoField_throwsAnError)
{
    ASSERT_THROW(torrentFileDeserializer.deserialize(torrentFileWithoutInfoFieldContent), errors::MissingTorrentInfo);
}

TEST_F(TorrentFileDeserializerImplTest, givenTorrentFileWithoutLengthField_throwsAnError)
{
    ASSERT_THROW(torrentFileDeserializer.deserialize(torrentFileWithoutLengthFieldContent), errors::MissingTorrentInfo);
}

TEST_F(TorrentFileDeserializerImplTest, givenTorrentFileWithoutNameField_throwsAnError)
{
    ASSERT_THROW(torrentFileDeserializer.deserialize(torrentFileWithoutNameFieldContent), errors::MissingTorrentInfo);
}

TEST_F(TorrentFileDeserializerImplTest, givenTorrentFileWithoutPieceLengthField_throwsAnError)
{
    ASSERT_THROW(torrentFileDeserializer.deserialize(torrentFileWithoutPieceLengthFieldContent),
                 errors::MissingTorrentInfo);
}

TEST_F(TorrentFileDeserializerImplTest, givenTorrentFileWithoutPiecesField_throwsAnError)
{
    ASSERT_THROW(torrentFileDeserializer.deserialize(torrentFileWithoutPiecesFieldContent), errors::MissingTorrentInfo);
}

TEST_F(TorrentFileDeserializerImplTest, givenInvalidBencodeFile_throwsAnError)
{
    ASSERT_THROW(torrentFileDeserializer.deserialize(invalidBencodeFileContent), errors::InvalidBencodeFileFormatError);
}
