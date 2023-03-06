#pragma once

#include "bencode.hpp"
#include "TorrentFileDeserializer.h"

namespace core
{
class TorrentFileDeserializerImpl : public TorrentFileDeserializer
{
public:
    TorrentFileInfo deserialize(const std::string&) const override;

private:
    std::string getInfoHash(const bencode::dict&) const;
    std::vector<std::string> getPiecesHashes(const bencode::dict&) const;
};
}
