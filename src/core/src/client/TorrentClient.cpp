#include "TorrentClient.h"

#include <numeric>

#include "fmt/format.h"

#include "../session/HandshakeMessage.h"
#include "../session/PeerToPeerSessionImpl.h"
#include "collection/ThreadSafeQueue.h"
#include "PeerIdGenerator.h"

namespace core
{
TorrentClient::TorrentClient(std::unique_ptr<libs::fileSystem::FileSystemService> fileSystemServiceInit,
                             std::unique_ptr<TorrentFileDeserializer> torrentFileDeserializerInit,
                             std::unique_ptr<libs::httpClient::HttpClient> httpClientInit,
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
    srand(time(NULL));

    const auto torrentFileContent = fileSystemService->read(torrentFilePath);

    const auto torrentFileInfo = torrentFileDeserializer->deserialize(torrentFileContent);

    const auto numberOfPieces = static_cast<unsigned>(torrentFileInfo.piecesHashes.size());

    std::cout << fmt::format("File has {} pieces, each piece has {} bytes.", numberOfPieces, torrentFileInfo.pieceLength) << std::endl;

    std::vector<int> iotaData(numberOfPieces);

    std::iota(iotaData.begin(), iotaData.end(), 0);

    auto piecesQueue = libs::collection::ThreadSafeQueue{iotaData};

    const auto peerId = PeerIdGenerator::generate();

    const auto retrievePeersPayload = RetrievePeersPayload{torrentFileInfo.announce,
                                                           torrentFileInfo.infoHash,
                                                           peerId,
                                                           "0",
                                                           "0",
                                                           "0",
                                                           std::to_string(torrentFileInfo.length),
                                                           "1"};

    const auto response = peerRetriever->retrievePeers(retrievePeersPayload);

    std::cout << fmt::format("Got list of {} peers.", response.peersEndpoints.size()) << std::endl;

    const auto firstPeerEndpoint = response.peersEndpoints[rand() % 50];

    boost::asio::io_context context;

    std::unique_ptr<PeerToPeerSession> peerToPeerSession = std::make_unique<PeerToPeerSessionImpl>(
        context, piecesQueue, firstPeerEndpoint, peerId, torrentFileInfo.pieceLength);

    peerToPeerSession->startSession(torrentFileInfo.infoHash);

    context.run();
}
}
