#pragma once

#include <stdexcept>

struct FileNotFound : std::runtime_error
{
    using std::runtime_error::runtime_error;
};
