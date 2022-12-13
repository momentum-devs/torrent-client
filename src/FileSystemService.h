#pragma once

#include <string>

class FileSystemService
{
public:
    virtual ~FileSystemService() = default;

    virtual void write(const std::string& absolutePath, const std::string& data) const = 0;
    virtual void append(const std::string& absolutePath, const std::string& data) const = 0;
    virtual std::string read(const std::string& absolutePath) const = 0;
};
