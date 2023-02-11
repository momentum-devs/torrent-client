#pragma once

#include "FileSystemService.h"

namespace libs::fileSystem
{
class FileSystemServiceImpl : public FileSystemService
{
public:
    void write(const std::string& absolutePath, const std::string& content) const override;
    void writeAtPosition(const std::string& absolutePath, const std::string& data,
                         unsigned int position) const override;
    void append(const std::string& absolutePath, const std::string& content) const override;
    std::string read(const std::string& absolutePath) const override;
    bool exists(const std::string& absolutePath) const override;
};
}
