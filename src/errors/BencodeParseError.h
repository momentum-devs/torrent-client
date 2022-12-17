#pragma once

#include <stdexcept>

namespace errors
{
struct BencodeParseError : std::runtime_error
{
    using std::runtime_error::runtime_error;
};
}
