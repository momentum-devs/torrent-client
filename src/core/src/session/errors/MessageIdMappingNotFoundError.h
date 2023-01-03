#pragma once

#include <stdexcept>

namespace core::errors
{
struct MessageIdMappingNotFoundNotError : std::runtime_error
{
    using std::runtime_error::runtime_error;
};
}
