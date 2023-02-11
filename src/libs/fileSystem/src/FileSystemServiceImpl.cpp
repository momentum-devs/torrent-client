#include "FileSystemServiceImpl.h"

#include <filesystem>
#include <fstream>
#include <iostream>

#include "fmt/core.h"

#include "errors/FileNotFound.h"

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

void FileSystemServiceImpl::writeAtPosition(const std::string& absolutePath,
                                            const std::basic_string<unsigned char>& data, unsigned int position) const
{
    std::ofstream fileStream{absolutePath, std::ios::binary | std::ios::in | std::ios::out};

    if (!fileStream.is_open())
    {
        throw errors::FileNotFound(fmt::format("file not found: {}", absolutePath));
    }

    fileStream.seekp(position, std::ios::beg);

    fileStream.write(reinterpret_cast<const char*>(data.c_str()), static_cast<long>(data.size()));
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

bool FileSystemServiceImpl::exists(const std::string& absolutePath) const
{
    return std::filesystem::exists(absolutePath);
}

std::string FileSystemServiceImpl::getParentDirectory(const std::string& absolutePath) const
{
    return std::filesystem::path{absolutePath}.parent_path();
}

std::string FileSystemServiceImpl::getFileName(const std::string& absolutePath) const
{
    return std::filesystem::path{absolutePath}.filename();
}

void FileSystemServiceImpl::remove(const std::string& absolutePath) const
{
    std::filesystem::remove_all(absolutePath);
}

}
