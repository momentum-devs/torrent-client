#pragma once

#include <stdexcept>

namespace core::exceptions
{
struct InvalidJsonError : std::runtime_error
{
    using std::runtime_error::runtime_error;
};
}
