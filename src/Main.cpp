#include <boost/asio.hpp>
#include <boost/program_options.hpp>
#include <iostream>

#include "AnnounceResponseDeserializerImpl.h"
#include "CprHttpClient.h"
#include "FileSystemServiceImpl.h"
#include "HandshakeMessageSerializer.h"
#include "PeerConnector.h"
#include "PeerIdGenerator.h"
#include "PeerRetrieverImpl.h"
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

    std::unique_ptr<FileSystemService> fileSystemService = std::make_unique<FileSystemServiceImpl>();

    auto torrentFileContent = fileSystemService->read(torrentFilePath);

    std::unique_ptr<TorrentFileDeserializer> torrentFileDeserializer = std::make_unique<TorrentFileDeserializerImpl>();

    auto torrentFileInfo = torrentFileDeserializer->deserialize(torrentFileContent);

    std::unique_ptr<HttpClient> httpClient = std::make_unique<CprHttpClient>();

    std::unique_ptr<AnnounceResponseDeserializer> responseDeserializer =
        std::make_unique<AnnounceResponseDeserializerImpl>();

    std::unique_ptr<PeerRetriever> peerRetriever =
        std::make_unique<PeerRetrieverImpl>(std::move(httpClient), std::move(responseDeserializer));

    auto retrievePeersPayload = RetrievePeersPayload{torrentFileInfo.announce,
                                                     torrentFileInfo.infoHash,
                                                     PeerIdGenerator::generate(),
                                                     "0",
                                                     "0",
                                                     "0",
                                                     std::to_string(torrentFileInfo.length),
                                                     "1"};

    auto response = peerRetriever->retrievePeers(retrievePeersPayload);

    std::cout << "Get list of " << response.peersEndpoints.size() << " peers" <<std::endl;

    auto firstPeerEndpoint = response.peersEndpoints[0];

    boost::asio::io_context context;

    auto handshakeMessage =
        HandshakeMessage{"BitTorrent protocol", torrentFileInfo.infoHash, PeerIdGenerator::generate()};

    PeerConnector peerConnector = PeerConnector{context, firstPeerEndpoint, handshakeMessage};

    context.run();

    return 0;
}
