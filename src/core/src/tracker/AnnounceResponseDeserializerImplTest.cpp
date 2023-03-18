#include "AnnounceResponseDeserializerImpl.h"

#include "gtest/gtest.h"

#include "fileSystem/FileSystemServiceFactory.h"
#include "fileSystem/GetProjectPath.h"

using namespace ::testing;
using namespace libs::fileSystem;
using namespace core;

namespace
{
std::unique_ptr<libs::fileSystem::FileSystemService> fileSystemService =
    FileSystemServiceFactory().createFileSystemService();
const auto projectPath = getProjectPath("torrent-client");
const auto testFileDirectoryPath = projectPath + "src/core/src/torrentFile/testFiles/";
const auto validAnnounceResponseFilePath = testFileDirectoryPath + "validAnnounceResponse.txt";
const auto validAnnounceResponseFileContent = fileSystemService->read(validAnnounceResponseFilePath);
PeerEndpoint peerEndpoint{"169.1.40.40", 51414};
}

class AnnounceResponseDeserializerImplTest : public Test
{
public:
    AnnounceResponseDeserializerImpl deserializer;
};

TEST_F(AnnounceResponseDeserializerImplTest, deserializeBencode)
{
    auto deserializedResponse = deserializer.deserializeBencode(validAnnounceResponseFileContent);

    std::cout << deserializedResponse;

    ASSERT_EQ(deserializedResponse.interval, 900);
    ASSERT_EQ(deserializedResponse.peersEndpoints.size(), 50);
    ASSERT_EQ(deserializedResponse.peersEndpoints[0], peerEndpoint);
}
