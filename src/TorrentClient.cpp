#include "TorrentClient.h"

#include "fmt/format.h"
#include "HandshakeMessage.h"
#include "PeerConnector.h"
#include "PeerIdGenerator.h"

TorrentClient::TorrentClient(std::unique_ptr<FileSystemService> fileSystemServiceInit,
                             std::unique_ptr<TorrentFileDeserializer> torrentFileDeserializerInit,
                             std::unique_ptr<HttpClient> httpClientInit,
                             std::unique_ptr<AnnounceResponseDeserializer> responseDeserializerInit,
                             std::unique_ptr<PeerRetriever> peerRetrieverInit)
    : fileSystemService{std::move(fileSystemServiceInit)},
      torrentFileDeserializer{std::move(torrentFileDeserializerInit)},
      httpClient{std::move(httpClientInit)},
      responseDeserializer{std::move(responseDeserializerInit)},
      peerRetriever{std::move(peerRetrieverInit)}
{
}

void TorrentClient::download(const std::string& torrentFilePath)
{
    auto torrentFileContent = fileSystemService->read(torrentFilePath);

    auto torrentFileInfo = torrentFileDeserializer->deserialize(torrentFileContent);

    const auto numberOfPieces = static_cast<unsigned>(torrentFileInfo.piecesHashes.size());

    std::cout << fmt::format("File has {} pieces.", numberOfPieces) << std::endl;

    auto peerId = PeerIdGenerator::generate();

    auto retrievePeersPayload = RetrievePeersPayload{torrentFileInfo.announce,
                                                     torrentFileInfo.infoHash,
                                                     peerId,
                                                     "0",
                                                     "0",
                                                     "0",
                                                     std::to_string(torrentFileInfo.length),
                                                     "1"};

    auto response = peerRetriever->retrievePeers(retrievePeersPayload);

    std::cout << "Get list of " << response.peersEndpoints.size() << " peers" << std::endl;

    auto firstPeerEndpoint = response.peersEndpoints[9];

    boost::asio::io_context context;

    auto handshakeMessage = HandshakeMessage{"BitTorrent protocol", torrentFileInfo.infoHash, peerId};

    PeerConnector peerConnector = PeerConnector{context, firstPeerEndpoint, handshakeMessage, numberOfPieces};

    context.run();
}
