#include <boost/program_options.hpp>
#include <iostream>

#include "core/include/TorrentClient.h"
#include "core/src/client/PeerIdGenerator.h"
#include "core/src/session/HandshakeMessageSerializer.h"
#include "core/src/torrentFile/TorrentFileDeserializerImpl.h"
#include "core/src/tracker/AnnounceResponseDeserializerImpl.h"
#include "core/src/tracker/PeersRetrieverImpl.h"
#include "fileSystem/FileSystemServiceFactory.h"
#include "httpClient/HttpClientFactory.h"

int main(int argc, char* argv[])
{
    // TODO: add parser class
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

    std::unique_ptr<core::TorrentFileDeserializer> torrentFileDeserializer =
        std::make_unique<core::TorrentFileDeserializerImpl>();

    std::unique_ptr<common::httpClient::HttpClient> httpClient =
        common::httpClient::HttpClientFactory().createHttpClient();

    std::unique_ptr<core::AnnounceResponseDeserializer> responseDeserializer =
        std::make_unique<core::AnnounceResponseDeserializerImpl>();

    std::unique_ptr<core::PeersRetriever> peerRetriever =
        std::make_unique<core::PeersRetrieverImpl>(std::move(httpClient), std::move(responseDeserializer));

    core::TorrentClient torrentClient{std::move(fileSystemService), std::move(torrentFileDeserializer),
                                      std::move(httpClient), std::move(responseDeserializer), std::move(peerRetriever)};

    torrentClient.download(torrentFilePath);

    return 0;
}
