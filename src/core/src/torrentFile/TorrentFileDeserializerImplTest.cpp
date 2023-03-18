#include "TorrentFileDeserializerImpl.h"

#include "gtest/gtest.h"

#include "errors/InvalidBencodeFormatError.h"
#include "errors/MissingBencodeFieldValue.h"
#include "fileSystem/FileSystemServiceFactory.h"
#include "fileSystem/GetProjectPath.h"

using namespace ::testing;
using namespace core;

namespace
{
std::unique_ptr<libs::fileSystem::FileSystemService> fileSystemService =
    libs::fileSystem::FileSystemServiceFactory().createFileSystemService();
const auto projectPath = libs::fileSystem::getProjectPath("torrent-client");
const auto testFileDirectoryPath = projectPath + "src/core/src/torrentFile/testFiles/";
const auto validTorrentFilePath = testFileDirectoryPath + "valid.torrent";
const auto validMultiFilesTorrentFilePath = testFileDirectoryPath + "validMultiFiles.torrent";
const auto torrentFileWithoutAnnounceFieldPath = testFileDirectoryPath + "fileWithoutAnnounceField.torrent";
const auto torrentFileWithoutInfoFieldPath = testFileDirectoryPath + "fileWithoutInfoField.torrent";
const auto torrentFileWithoutLengthFieldPath = testFileDirectoryPath + "fileWithoutLengthField.torrent";
const auto torrentFileWithoutNameFieldPath = testFileDirectoryPath + "fileWithoutNameField.torrent";
const auto torrentFileWithoutPieceLengthFieldPath = testFileDirectoryPath + "fileWithoutPieceLengthField.torrent";
const auto torrentFileWithoutPiecesFieldPath = testFileDirectoryPath + "fileWithoutPiecesField.torrent";
const auto invalidBencodeFilePath = testFileDirectoryPath + "invalidBencodeFile.torrent";

const auto validTorrentFileContent = fileSystemService->read(validTorrentFilePath);
const auto validMultiFilesTorrentFileContent = fileSystemService->read(validMultiFilesTorrentFilePath);
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

    ASSERT_EQ(torrentFileInfo.announceList[0], "http://bttracker.debian.org:6969/announce");
    ASSERT_EQ(torrentFileInfo.infoHash, "b3f4b20f822c471cfd97745abb98e69b4fe0986e");
    ASSERT_EQ(torrentFileInfo.length, 399507456);
    ASSERT_EQ(torrentFileInfo.pieceLength, 262144);
    ASSERT_EQ(torrentFileInfo.piecesHashes[0], "3fbdf5be0c348bb673cca7eab8eb44f7a2c0f409");
    ASSERT_EQ(torrentFileInfo.piecesHashes.size(), 1524);
    ASSERT_EQ(torrentFileInfo.nestedFilesInfo, std::nullopt);
}

TEST_F(TorrentFileDeserializerImplTest, deserializeMultiFileTorrentFile)
{
    const auto torrentFileInfo = torrentFileDeserializer.deserialize(validMultiFilesTorrentFileContent);

    ASSERT_EQ(torrentFileInfo.announceList[0], "udp://tracker.leechers-paradise.org:6969/announce");
    ASSERT_EQ(torrentFileInfo.infoHash, "9e8cbe0fc04f2c6eced5896173bdedbeca846f4d");
    ASSERT_EQ(torrentFileInfo.length, 10562409472);
    ASSERT_EQ(torrentFileInfo.pieceLength, 1048576);
    ASSERT_EQ(torrentFileInfo.piecesHashes[0], "0d61225476b4483667bb47cb9a631558cfe64ac0");
    ASSERT_EQ(torrentFileInfo.piecesHashes.size(), 10074);
    ASSERT_EQ(torrentFileInfo.nestedFilesInfo->size(), 13);
    ASSERT_EQ(torrentFileInfo.nestedFilesInfo->at(0).name, "The Sopranos S01E09 Boca.mkv");
    ASSERT_EQ(torrentFileInfo.nestedFilesInfo->at(0).length, 1031293069);
}

TEST_F(TorrentFileDeserializerImplTest, givenTorrentFileWithoutAnnounceField_throwsAnError)
{
    ASSERT_THROW(torrentFileDeserializer.deserialize(torrentFileWithoutAnnounceFieldContent),
                 errors::MissingBencodeFieldValue);
}

TEST_F(TorrentFileDeserializerImplTest, givenTorrentFileWithoutInfoField_throwsAnError)
{
    ASSERT_THROW(torrentFileDeserializer.deserialize(torrentFileWithoutInfoFieldContent),
                 errors::MissingBencodeFieldValue);
}

TEST_F(TorrentFileDeserializerImplTest, givenTorrentFileWithoutLengthField_throwsAnError)
{
    ASSERT_THROW(torrentFileDeserializer.deserialize(torrentFileWithoutLengthFieldContent),
                 errors::MissingBencodeFieldValue);
}

TEST_F(TorrentFileDeserializerImplTest, givenTorrentFileWithoutNameField_throwsAnError)
{
    ASSERT_THROW(torrentFileDeserializer.deserialize(torrentFileWithoutNameFieldContent),
                 errors::MissingBencodeFieldValue);
}

TEST_F(TorrentFileDeserializerImplTest, givenTorrentFileWithoutPieceLengthField_throwsAnError)
{
    ASSERT_THROW(torrentFileDeserializer.deserialize(torrentFileWithoutPieceLengthFieldContent),
                 errors::MissingBencodeFieldValue);
}

TEST_F(TorrentFileDeserializerImplTest, givenTorrentFileWithoutPiecesField_throwsAnError)
{
    ASSERT_THROW(torrentFileDeserializer.deserialize(torrentFileWithoutPiecesFieldContent),
                 errors::MissingBencodeFieldValue);
}

TEST_F(TorrentFileDeserializerImplTest, givenInvalidBencodeFile_throwsAnError)
{
    ASSERT_THROW(torrentFileDeserializer.deserialize(invalidBencodeFileContent), errors::InvalidBencodeFormatError);
}
