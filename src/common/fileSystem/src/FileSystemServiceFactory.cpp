#include "FileSystemServiceFactory.h"

#include "FileSystemService.h"
#include "FileSystemServiceImpl.h"

namespace common::fileSystem
{
std::unique_ptr<FileSystemService> FileSystemServiceFactory::createFileSystemService() const
{
    return std::make_unique<FileSystemServiceImpl>();
}
}
