#pragma once

#include <string>

namespace libs::fileSystem
{
class FileSystemService
{
public:
    virtual ~FileSystemService() = default;

    virtual void write(const std::string& absolutePath, const std::string& data) const = 0;
    virtual void writeAtPosition(const std::string& absolutePath, const std::string& data,
                                 unsigned int position) const = 0;
    virtual void append(const std::string& absolutePath, const std::string& data) const = 0;
    virtual std::string read(const std::string& absolutePath) const = 0;
    virtual bool exists(const std::string& absolutePath) const = 0;
};
}
