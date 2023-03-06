#include "TorrentClientFactory.h"

#include "../torrentFile/TorrentFileDeserializerImpl.h"
#include "../tracker/AnnounceResponseDeserializerImpl.h"
#include "../tracker/PeersRetrieverImpl.h"
#include "fileSystem/FileSystemServiceFactory.h"
#include "httpClient/HttpClientFactory.h"
#include "TorrentClientImpl.h"

namespace core
{
std::unique_ptr<TorrentClient> TorrentClientFactory::createTorrentClient()
{
    std::shared_ptr<libs::fileSystem::FileSystemService> fileSystemService =
        libs::fileSystem::FileSystemServiceFactory().createFileSystemService();

    std::unique_ptr<core::TorrentFileDeserializer> torrentFileDeserializer =
        std::make_unique<core::TorrentFileDeserializerImpl>();

    std::unique_ptr<libs::httpClient::HttpClient> httpClient = libs::httpClient::HttpClientFactory().createHttpClient();

    std::unique_ptr<core::AnnounceResponseDeserializer> responseDeserializer =
        std::make_unique<core::AnnounceResponseDeserializerImpl>();

    std::unique_ptr<core::PeersRetriever> peerRetriever =
        std::make_unique<core::PeersRetrieverImpl>(std::move(httpClient), std::move(responseDeserializer));

    return std::make_unique<TorrentClientImpl>(std::move(fileSystemService), std::move(torrentFileDeserializer),
                                               std::move(httpClient), std::move(responseDeserializer),
                                               std::move(peerRetriever));
}
}
