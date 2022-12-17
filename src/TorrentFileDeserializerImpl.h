#pragma once

#include "bencode.hpp"
#include "TorrentFileDeserializer.h"

struct TorrentFileInfo;

class TorrentFileDeserializerImpl: public TorrentFileDeserializer{
public:
    TorrentFileInfo deserialize(const std::string&) override;
private:
    std::string getAnnounce(bencode::dict&);
    std::string getInfoHash(bencode::dict&);
    long long getTorrentSize(bencode::dict&);
};
