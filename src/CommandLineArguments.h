#pragma once

#include <ostream>
#include <string>

struct CommandLineArguments
{
    std::string torrentFilePath;
    std::string destinationDirectory;
};

inline std::ostream& operator<<(std::ostream& os, const CommandLineArguments& arguments)
{
    return os << "torrentFilePath: " << arguments.torrentFilePath
              << " destinationDirectory: " << arguments.destinationDirectory;
}
