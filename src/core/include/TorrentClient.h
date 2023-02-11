#pragma once

#include <memory>
#include <string>

#include "../src/torrentFile/TorrentFileDeserializer.h"
#include "../src/tracker/AnnounceResponseDeserializer.h"
#include "../src/tracker/PeersRetriever.h"
#include "fileSystem/FileSystemService.h"
#include "httpClient/HttpClient.h"

namespace core
{
// TODO: add interface and factory
class TorrentClient
{
public:
    TorrentClient(std::unique_ptr<libs::fileSystem::FileSystemService> fileSystemService,
                  std::unique_ptr<TorrentFileDeserializer> torrentFileDeserializer,
                  std::unique_ptr<libs::httpClient::HttpClient> httpClient,
                  std::unique_ptr<AnnounceResponseDeserializer> responseDeserializer,
                  std::unique_ptr<PeersRetriever> peerRetriever);

    void download(const std::string& torrentFilePath);

private:
    std::unique_ptr<libs::fileSystem::FileSystemService> fileSystemService;
    std::unique_ptr<TorrentFileDeserializer> torrentFileDeserializer;
    std::unique_ptr<libs::httpClient::HttpClient> httpClient;
    std::unique_ptr<AnnounceResponseDeserializer> responseDeserializer;
    std::unique_ptr<PeersRetriever> peerRetriever;
};
}
