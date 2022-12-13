#include <fstream>
#include <sstream>

#include "errors/FileNotFound.h"
#include "FileSystemServiceImpl.h"
#include "fmt/core.h"

void FileSystemServiceImpl::write(const std::string& absolutePath, const std::string& content) const
{
    std::ofstream fileStream{absolutePath};

    if (!fileStream.is_open())
    {
        throw FileNotFound(fmt::format("file not found", absolutePath));
    }

    fileStream << content;
}

void FileSystemServiceImpl::append(const std::string& absolutePath, const std::string& content) const
{
    std::ofstream fileStream{absolutePath, std::ofstream::app};

    if (!fileStream.is_open())
    {
        throw FileNotFound(fmt::format("file not found", absolutePath));
    }

    fileStream << content;
}

std::string FileSystemServiceImpl::read(const std::string& absolutePath) const
{
    std::ifstream fileStream{absolutePath};

    std::stringstream buffer;

    if (!fileStream.is_open())
    {
        throw FileNotFound(fmt::format("file not found", absolutePath));
    }

    buffer << fileStream.rdbuf();

    return buffer.str();
}
