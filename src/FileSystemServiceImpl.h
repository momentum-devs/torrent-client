#pragma once

#include "FileSystemService.h"

class FileSystemServiceImpl : public FileSystemService
{
public:
    void write(const std::string& absolutePath, const std::string& content) const override;
    void append(const std::string& absolutePath, const std::string& content) const override;
    std::string read(const std::string& absolutePath) const override;
};
