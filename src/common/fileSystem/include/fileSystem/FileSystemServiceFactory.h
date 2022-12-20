#pragma once

#include <memory>

#include "FileSystemService.h"

namespace common::fileSystem
{
class FileSystemServiceFactory
{
public:
    std::unique_ptr<FileSystemService> createFileSystemService() const;
};
}
