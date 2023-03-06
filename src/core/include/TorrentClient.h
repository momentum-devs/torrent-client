#pragma once

#include <string>

namespace core
{
class TorrentClient
{
public:
    virtual ~TorrentClient() = default;

    virtual void download(const std::string& torrentFilePath, const std::string& destinationDirectory) = 0;
};
}
