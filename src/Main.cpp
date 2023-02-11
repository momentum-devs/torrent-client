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

    description.add_options()("destination_directory, d", boost::program_options::value<std::string>(),
                              "path to directory where file(s) will be saved");

    boost::program_options::variables_map variablesMap;

    auto parsedArguments = boost::program_options::parse_command_line(argc, argv, description);

    boost::program_options::store(parsedArguments, variablesMap);

    boost::program_options::notify(variablesMap);

    if (!variablesMap.count("torrent_file"))
    {
        std::cerr << "put argument: torrent_file, t - path to torrent file to download" << std::endl;
        return 0;
    }

    if (!variablesMap.count("destination_directory"))
    {
        std::cerr << "put argument: destination_directory, d - path to directory where file(s) will be saved"
                  << std::endl;
        return 0;
    }

    auto torrentFilePath = variablesMap["torrent_file"].as<std::string>();
    auto destinationDirectory = variablesMap["destination_directory"].as<std::string>();

    std::shared_ptr<libs::fileSystem::FileSystemService> fileSystemService =
        libs::fileSystem::FileSystemServiceFactory().createFileSystemService();

    std::unique_ptr<core::TorrentFileDeserializer> torrentFileDeserializer =
        std::make_unique<core::TorrentFileDeserializerImpl>();

    std::unique_ptr<libs::httpClient::HttpClient> httpClient = libs::httpClient::HttpClientFactory().createHttpClient();

    std::unique_ptr<core::AnnounceResponseDeserializer> responseDeserializer =
        std::make_unique<core::AnnounceResponseDeserializerImpl>();

    std::unique_ptr<core::PeersRetriever> peerRetriever =
        std::make_unique<core::PeersRetrieverImpl>(std::move(httpClient), std::move(responseDeserializer));

    core::TorrentClient torrentClient{std::move(fileSystemService), std::move(torrentFileDeserializer),
                                      std::move(httpClient), std::move(responseDeserializer), std::move(peerRetriever)};

    torrentClient.download(torrentFilePath, destinationDirectory);

    return 0;
}
