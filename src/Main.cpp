#include <boost/program_options.hpp>
#include <iostream>

#include "AnnounceResponseDeserializerImpl.h"
#include "fileSystem/FileSystemServiceFactory.h"
#include "fmt/format.h"
#include "HandshakeMessageSerializer.h"
#include "httpClient/HttpClientFactory.h"
#include "PeerIdGenerator.h"
#include "PeersRetrieverImpl.h"
#include "TorrentClient.h"
#include "TorrentFileDeserializerImpl.h"

int main(int argc, char* argv[])
{
    boost::program_options::options_description description;

    description.add_options()("torrent_file, t", boost::program_options::value<std::string>(),
                              "path to torrent file to download");

    boost::program_options::variables_map variablesMap;

    auto parsedArguments = boost::program_options::parse_command_line(argc, argv, description);

    boost::program_options::store(parsedArguments, variablesMap);

    boost::program_options::notify(variablesMap);

    if (!variablesMap.count("torrent_file"))
    {
        std::cout << "put argument: torrent_file, t - path to torrent file to download" << std::endl;
        return 0;
    }

    auto torrentFilePath = variablesMap["torrent_file"].as<std::string>();

    std::unique_ptr<common::fileSystem::FileSystemService> fileSystemService =
        common::fileSystem::FileSystemServiceFactory().createFileSystemService();

    auto torrentFileContent = fileSystemService->read(torrentFilePath);

    std::unique_ptr<TorrentFileDeserializer> torrentFileDeserializer = std::make_unique<TorrentFileDeserializerImpl>();

    auto torrentFileInfo = torrentFileDeserializer->deserialize(torrentFileContent);

    const auto numberOfPieces = static_cast<unsigned>(torrentFileInfo.piecesHashes.size());

    std::cout << fmt::format("File has {} pieces.", numberOfPieces) << std::endl;

    std::unique_ptr<common::httpClient::HttpClient> httpClient =
        common::httpClient::HttpClientFactory().createHttpClient();

    std::unique_ptr<AnnounceResponseDeserializer> responseDeserializer =
        std::make_unique<AnnounceResponseDeserializerImpl>();
    std::unique_ptr<PeersRetriever> peerRetriever =
        std::make_unique<PeersRetrieverImpl>(std::move(httpClient), std::move(responseDeserializer));

    TorrentClient torrentClient{std::move(fileSystemService), std::move(torrentFileDeserializer), std::move(httpClient),
                                std::move(responseDeserializer), std::move(peerRetriever)};

    torrentClient.download(torrentFilePath);

    return 0;
}
