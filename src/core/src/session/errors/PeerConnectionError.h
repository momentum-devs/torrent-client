#pragma once

#include <stdexcept>

namespace core::errors
{
struct PeerConnectionError : std::runtime_error
{
    using std::runtime_error::runtime_error;
};
}
