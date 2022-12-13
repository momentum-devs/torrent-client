#pragma once

#include "bencode.hpp"
#include "TorrentFileParser.h"

struct TorrentFileInfo;

class TorrentFileParserImpl: public TorrentFileParser{
public:
    TorrentFileInfo parse(std::string) override;
private:
    std::string getAnnounce(bencode::dict&);
    std::string getInfoHash(bencode::dict&);
    long long getTorrentSize(bencode::dict&);
};