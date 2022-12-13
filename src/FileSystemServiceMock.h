#pragma once

#include <gmock/gmock.h>

#include "FileSystemService.h"

class FileSystemServiceMock : public FileSystemService
{
public:
    MOCK_CONST_METHOD2(write, void(const std::string& absolutePath, const std::string& data));
    MOCK_CONST_METHOD2(append, void(const std::string& absolutePath, const std::string& data));
    MOCK_CONST_METHOD1(read, std::string(const std::string& absolutePath));
};
