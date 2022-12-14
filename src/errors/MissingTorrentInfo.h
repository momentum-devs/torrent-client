#pragma once

#include <stdexcept>

struct MissingTorrentInfo : std::runtime_error
{
    using std::runtime_error::runtime_error;
};
