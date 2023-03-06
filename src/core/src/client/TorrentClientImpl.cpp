#include "TorrentClientImpl.h"

#include <numeric>

#include "fmt/format.h"

#include "../session/HandshakeMessage.h"
#include "../session/PeerToPeerSessionImpl.h"
#include "../session/PieceRepositoryImpl.h"
#include "../session/PiecesSerializerImpl.h"
#include "collection/ThreadSafeQueue.h"
#include "PeerIdGenerator.h"

namespace core
{
TorrentClientImpl::TorrentClientImpl(std::shared_ptr<libs::fileSystem::FileSystemService> fileSystemServiceInit,
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

void TorrentClientImpl::download(const std::string& torrentFilePath, const std::string& destinationDirectory)
{
    const auto torrentFileContent = fileSystemService->read(torrentFilePath);

    const auto torrentFileInfo =
        std::make_shared<TorrentFileInfo>(torrentFileDeserializer->deserialize(torrentFileContent));

    const auto numberOfPieces = static_cast<unsigned>(torrentFileInfo->piecesHashes.size());

    std::cout << fmt::format("File has {} pieces, each piece has {} bytes.", numberOfPieces,
                             torrentFileInfo->pieceLength)
              << std::endl;

    std::vector<int> iotaData(numberOfPieces);

    std::iota(iotaData.begin(), iotaData.end(), 0);

    auto piecesQueue = libs::collection::ThreadSafeQueue{iotaData};

    const auto peerId = PeerIdGenerator::generate();

    const auto retrievePeersPayload = RetrievePeersPayload{torrentFileInfo->announce,
                                                           torrentFileInfo->infoHash,
                                                           peerId,
                                                           "0",
                                                           "0",
                                                           "0",
                                                           std::to_string(torrentFileInfo->length),
                                                           "1"};

    const auto response = peerRetriever->retrievePeers(retrievePeersPayload);

    std::cout << fmt::format("Got list of {} peers.", response.peersEndpoints.size()) << std::endl;

    std::shared_ptr<core::PiecesSerializer> piecesSerializer = std::make_shared<core::PiecesSerializerImpl>();

    const auto outputFilePath = fmt::format("{}/{}", destinationDirectory, torrentFileInfo->name);

    const auto metadataFilePath = fmt::format("{}/.{}.metadata", destinationDirectory, torrentFileInfo->name);

    std::shared_ptr<core::PieceRepository> pieceRepository = std::make_shared<core::PieceRepositoryImpl>(
        fileSystemService, piecesSerializer, torrentFileInfo->pieceLength, outputFilePath, metadataFilePath);

    boost::asio::io_context context;

    std::vector<std::unique_ptr<PeerToPeerSession>> sessions;

    for (const auto& peerEndpoint : response.peersEndpoints)
    {
        sessions.push_back(std::make_unique<PeerToPeerSessionImpl>(context, piecesQueue, peerEndpoint, peerId,
                                                                   torrentFileInfo, pieceRepository));
        sessions.back()->startSession();
    }

    std::vector<std::thread> threads;

    const auto numberOfSupportedThreads = std::thread::hardware_concurrency() * 2;

    threads.reserve(numberOfSupportedThreads);

    for (unsigned int n = 0; n < numberOfSupportedThreads; ++n)
    {
        threads.emplace_back([&] { context.run(); });
    }

    for (auto& thread : threads)
    {
        if (thread.joinable())
        {
            thread.join();
        }
    }
}
}
