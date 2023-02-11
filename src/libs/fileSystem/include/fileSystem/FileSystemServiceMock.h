#pragma once

#include <gmock/gmock.h>

#include "FileSystemService.h"

namespace libs::fileSystem
{
class FileSystemServiceMock : public FileSystemService
{
public:
    MOCK_METHOD(void, write, (const std::string& absolutePath, const std::string& data), ());
    MOCK_METHOD(void, append, (const std::string& absolutePath, const std::string& data), ());
    MOCK_METHOD(std::string, read, (const std::string& absolutePath), ());
};
}
