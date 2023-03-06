#pragma once

#include <memory>

#include "TorrentClient.h"

namespace core
{
class TorrentClientFactory
{
public:
    static std::unique_ptr<TorrentClient> createTorrentClient();
};
}
