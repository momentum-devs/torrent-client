#include "AnnounceResponseDeserializerImpl.h"

#include "gtest/gtest.h"

#include "FileSystemServiceImpl.h"
#include "GetProjectPath.h"

using namespace ::testing;

namespace
{
std::unique_ptr<FileSystemService> fileSystemService = std::make_unique<FileSystemServiceImpl>();
const auto projectPath = getProjectPath("torrent-client");
const auto testFileDirectoryPath = projectPath + "src/testFiles/";
const auto validAnnounceResponseFilePath = testFileDirectoryPath + "validAnnounceResponse.txt";
const auto validAnnounceResponseFileContent = fileSystemService->read(validAnnounceResponseFilePath);
PeerEndpoint peerEndpoint{"169.1.40.40", 51414};
}

class AnnounceResponseDeserializerImplTest : public Test
{
public:
    AnnounceResponseDeserializerImpl deserializer;
};

TEST_F(AnnounceResponseDeserializerImplTest, deserialize)
{
    auto deserializedResponse = deserializer.deserialize(validAnnounceResponseFileContent);

    std::cout << deserializedResponse;

    ASSERT_EQ(deserializedResponse.interval, 900);
    ASSERT_EQ(deserializedResponse.peersEndpoints.size(), 50);
    ASSERT_EQ(deserializedResponse.peersEndpoints[0], peerEndpoint);
}
