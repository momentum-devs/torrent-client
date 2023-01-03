#pragma once

#include <stdexcept>

namespace core::errors
{
struct InvalidBencodeFormatError : std::runtime_error
{
    using std::runtime_error::runtime_error;
};
}
