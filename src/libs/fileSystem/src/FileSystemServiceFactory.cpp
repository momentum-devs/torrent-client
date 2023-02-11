#include "FileSystemServiceFactory.h"

#include "FileSystemService.h"
#include "FileSystemServiceImpl.h"

namespace libs::fileSystem
{
std::unique_ptr<FileSystemService> FileSystemServiceFactory::createFileSystemService() const
{
    return std::make_unique<FileSystemServiceImpl>();
}
}
