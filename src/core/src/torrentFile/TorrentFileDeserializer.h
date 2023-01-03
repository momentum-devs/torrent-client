#pragma once

#include <string>

#include "TorrentFileInfo.h"

namespace core
{
class TorrentFileDeserializer
{
public:
    virtual ~TorrentFileDeserializer() = default;

    virtual TorrentFileInfo deserialize(const std::string&) = 0;
};
}
