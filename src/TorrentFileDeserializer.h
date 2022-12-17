#pragma once

#include <string>

#include "TorrentFileInfo.h"

class TorrentFileDeserializer{
public:
    virtual ~TorrentFileDeserializer() = default;

    virtual TorrentFileInfo deserialize(const std::string&) = 0;
};
