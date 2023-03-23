#pragma once

#include <memory>
#include <string>

#include "../src/session/PieceRepository.h"
#include "../src/torrentFile/TorrentFileDeserializer.h"
#include "../src/tracker/AnnounceResponseDeserializer.h"
#include "../src/tracker/PeersRetriever.h"
#include "fileSystem/FileSystemService.h"
#include "httpClient/HttpClient.h"
#include "TorrentClient.h"

namespace core
{
class TorrentClientImpl : public TorrentClient
{
public:
    TorrentClientImpl(std::shared_ptr<libs::fileSystem::FileSystemService> fileSystemService,
                      std::unique_ptr<TorrentFileDeserializer> torrentFileDeserializer,
                      std::unique_ptr<libs::httpClient::HttpClient> httpClient,
                      std::unique_ptr<AnnounceResponseDeserializer> responseDeserializer,
                      std::unique_ptr<PeersRetriever> peerRetriever);

    void download(const std::string& torrentFilePath, const std::string& destinationDirectory);

private:
    std::shared_ptr<libs::fileSystem::FileSystemService> fileSystemService;
    std::unique_ptr<TorrentFileDeserializer> torrentFileDeserializer;
    std::unique_ptr<libs::httpClient::HttpClient> httpClient;
    std::unique_ptr<AnnounceResponseDeserializer> responseDeserializer;
    std::unique_ptr<PeersRetriever> peersRetriever;
};
}
