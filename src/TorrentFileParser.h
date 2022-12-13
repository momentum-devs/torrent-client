#pragma once

#include <string>

#include "TorrentFileInfo.h"

class TorrentFileParser{
public:
    virtual ~TorrentFileParser() = default;
    virtual TorrentFileInfo parse(std::string) = 0;
};