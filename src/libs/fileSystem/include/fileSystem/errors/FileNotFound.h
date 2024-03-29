#pragma once

#include <stdexcept>

namespace libs::fileSystem::errors
{
struct FileNotFound : std::runtime_error
{
    using std::runtime_error::runtime_error;
};
}
