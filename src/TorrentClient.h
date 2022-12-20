#pragma once

#include <memory>
#include <string>

#include "AnnounceResponseDeserializer.h"
#include "FileSystemService.h"
#include "HttpClient.h"
#include "PeerRetriever.h"
#include "TorrentFileDeserializer.h"

class TorrentClient
{
public:
    TorrentClient(std::unique_ptr<FileSystemService> fileSystemService,
                  std::unique_ptr<TorrentFileDeserializer> torrentFileDeserializer,
                  std::unique_ptr<HttpClient> httpClient,
                  std::unique_ptr<AnnounceResponseDeserializer> responseDeserializer,
                  std::unique_ptr<PeerRetriever> peerRetriever);

    void download(const std::string& torrentFilePath);

private:
    std::unique_ptr<FileSystemService> fileSystemService;
    std::unique_ptr<TorrentFileDeserializer> torrentFileDeserializer;
    std::unique_ptr<HttpClient> httpClient;
    std::unique_ptr<AnnounceResponseDeserializer> responseDeserializer;
    std::unique_ptr<PeerRetriever> peerRetriever;
};