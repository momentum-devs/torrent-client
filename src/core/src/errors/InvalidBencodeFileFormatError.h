#pragma once

#include <stdexcept>

namespace core::errors
{
struct InvalidBencodeFileFormatError : std::runtime_error
{
    using std::runtime_error::runtime_error;
};
}
