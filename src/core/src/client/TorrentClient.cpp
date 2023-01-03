#include "TorrentClient.h"

#include "fmt/format.h"

#include "../session/HandshakeMessage.h"
#include "../session/PeerToPeerSessionImpl.h"
#include "PeerIdGenerator.h"

namespace core
{
TorrentClient::TorrentClient(std::unique_ptr<common::fileSystem::FileSystemService> fileSystemServiceInit,
                             std::unique_ptr<TorrentFileDeserializer> torrentFileDeserializerInit,
                             std::unique_ptr<common::httpClient::HttpClient> httpClientInit,
                             std::unique_ptr<AnnounceResponseDeserializer> responseDeserializerInit,
                             std::unique_ptr<PeersRetriever> peerRetrieverInit)
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

    auto firstPeerEndpoint = response.peersEndpoints[7];

    boost::asio::io_context context;

    std::unique_ptr<PeerToPeerSession> peerConnector =
        std::make_unique<PeerToPeerSessionImpl>(context, numberOfPieces, firstPeerEndpoint, peerId);

    peerConnector->startSession(torrentFileInfo.infoHash);

    context.run();
}
}
