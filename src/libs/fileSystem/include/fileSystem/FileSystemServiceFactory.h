#pragma once

#include <memory>

#include "FileSystemService.h"

namespace libs::fileSystem
{
class FileSystemServiceFactory
{
public:
    std::unique_ptr<FileSystemService> createFileSystemService() const;
};
}
