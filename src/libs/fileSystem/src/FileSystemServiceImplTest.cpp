#include "FileSystemServiceImpl.h"

#include "fmt/format.h"
#include "gtest/gtest.h"

#include "errors/FileNotFound.h"
#include "GetProjectPath.h"

using namespace ::testing;
using namespace libs::fileSystem;

namespace
{
const std::string testFilesDirectory{
    fmt::format("{}src/libs/fileSystem/src/testFiles/", getProjectPath("torrent-client"))};
const std::string textToWrite{"write method"};
const std::string textToWriteAtPosition{"position data"};
const std::string textToAppend{"append method"};
const std::string textAfterWriteAndAppend{textToWrite + textToAppend};
const std::string filenameForReading = "testReading.txt";
const std::string filenameForWriting = "testWriting.txt";
const std::string filenameForWritingAtPosition = "testWritingAtPosition.txt";
const std::string pathForReading{testFilesDirectory + filenameForReading};
const std::string pathForWriting{testFilesDirectory + filenameForWriting};
const std::string pathForWritingAtPosition{testFilesDirectory + filenameForWritingAtPosition};
const std::string exampleContent{"example data\n"};
const std::string incorrectPath = "433\\UTzxxxx/fi123xtF";
}

class FileSystemServiceImplTest : public Test
{
public:
    FileSystemServiceImpl fileSystemService;
};

TEST_F(FileSystemServiceImplTest, givenCorrectPath_shouldWriteToFile)
{
    fileSystemService.write(pathForWriting, textToWrite);

    const auto actualFileContent = fileSystemService.read(pathForWriting);

    ASSERT_EQ(actualFileContent, textToWrite);
}

TEST_F(FileSystemServiceImplTest, givenIncorrectPath_shouldThrowFileNotFoundForWritingAppendingAndReading)
{
    ASSERT_THROW(fileSystemService.write(incorrectPath, textToWrite), errors::FileNotFound);
    ASSERT_THROW(fileSystemService.writeAtPosition(incorrectPath, textToWrite, 0), errors::FileNotFound);
    ASSERT_THROW(fileSystemService.append(incorrectPath, textToWrite), errors::FileNotFound);
    ASSERT_THROW(fileSystemService.read(incorrectPath), errors::FileNotFound);
}

TEST_F(FileSystemServiceImplTest, givenCorrectPath_shouldAppendToFile)
{
    fileSystemService.write(pathForWriting, textToWrite);
    fileSystemService.append(pathForWriting, textToAppend);

    const auto actualFileContent = fileSystemService.read(pathForWriting);

    ASSERT_EQ(actualFileContent, textAfterWriteAndAppend);
}

TEST_F(FileSystemServiceImplTest, givenCorrectPath_shouldReturnContentOfFile)
{
    const auto actualFileContent = fileSystemService.read(pathForReading);

    ASSERT_EQ(actualFileContent, exampleContent);
}

TEST_F(FileSystemServiceImplTest, givenCorrectPath_shouldWriteToFileAtPosition)
{
    fileSystemService.writeAtPosition(pathForWritingAtPosition, textToWriteAtPosition, 5);
    fileSystemService.writeAtPosition(pathForWritingAtPosition, textToWriteAtPosition, 30);

    const auto actualFileContent = fileSystemService.read(pathForWritingAtPosition);

    ASSERT_EQ(actualFileContent.size(), 43);
    ASSERT_EQ(actualFileContent.substr(5, textToWriteAtPosition.size()), textToWriteAtPosition);
    ASSERT_EQ(actualFileContent.substr(30, textToWriteAtPosition.size()), textToWriteAtPosition);
}
