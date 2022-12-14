#pragma once

#include <stdexcept>

struct BencodeParseError : std::runtime_error
{
    using std::runtime_error::runtime_error;
};
