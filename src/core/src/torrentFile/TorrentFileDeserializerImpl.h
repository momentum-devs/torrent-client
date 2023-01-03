#pragma once

#include "bencode.hpp"
#include "TorrentFileDeserializer.h"

namespace core
{
class TorrentFileDeserializerImpl : public TorrentFileDeserializer
{
public:
    TorrentFileInfo deserialize(const std::string&) override;

private:
    std::string getInfoHash(bencode::dict&);
    std::vector<std::string> getPiecesHashes(bencode::dict&);
};
}
