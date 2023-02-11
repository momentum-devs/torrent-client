#include "FileSystemServiceImpl.h"

#include <fstream>
#include <sstream>

#include "errors/FileNotFound.h"
#include "fmt/core.h"

namespace libs::fileSystem
{
void FileSystemServiceImpl::write(const std::string& absolutePath, const std::string& content) const
{
    std::ofstream fileStream{absolutePath};

    if (!fileStream.is_open())
    {
        throw errors::FileNotFound(fmt::format("file not found: {}", absolutePath));
    }

    fileStream << content;
}

void FileSystemServiceImpl::append(const std::string& absolutePath, const std::string& content) const
{
    std::ofstream fileStream{absolutePath, std::ofstream::app};

    if (!fileStream.is_open())
    {
        throw errors::FileNotFound(fmt::format("file not found: {}", absolutePath));
    }

    fileStream << content;
}

std::string FileSystemServiceImpl::read(const std::string& absolutePath) const
{
    std::ifstream fileStream{absolutePath};

    std::stringstream buffer;

    if (!fileStream.is_open())
    {
        throw errors::FileNotFound(fmt::format("file not found: {}", absolutePath));
    }

    buffer << fileStream.rdbuf();

    return buffer.str();
}
}
