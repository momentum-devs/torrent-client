#include "TorrentClientImpl.h"

#include <numeric>
#include <unordered_set>

#include "fmt/format.h"

#include "../session/HandshakeMessage.h"
#include "../session/PeerToPeerSessionImpl.h"
#include "../session/PeerToPeerSessionManager.h"
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
                                     std::unique_ptr<PeersRetriever> peersRetrieverInit)
    : fileSystemService{std::move(fileSystemServiceInit)},
      torrentFileDeserializer{std::move(torrentFileDeserializerInit)},
      httpClient{std::move(httpClientInit)},
      responseDeserializer{std::move(responseDeserializerInit)},
      peersRetriever{std::move(peersRetrieverInit)}
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

    std::shared_ptr<core::PiecesSerializer> piecesSerializer = std::make_shared<core::PiecesSerializerImpl>();

    std::shared_ptr<core::PieceRepository> pieceRepository = std::make_shared<core::PieceRepositoryImpl>(
        fileSystemService, piecesSerializer, torrentFileInfo->pieceLength, torrentFileInfo, destinationDirectory);

    std::vector<int> iotaData(numberOfPieces);

    std::iota(iotaData.begin(), iotaData.end(), 0);

    std::set<int> piecesIds{iotaData.begin(), iotaData.end()};

    const auto downloadedPiecesIds = pieceRepository->getDownloadedPieces();

    for (const auto pieceId : downloadedPiecesIds)
    {
        piecesIds.erase(static_cast<int>(pieceId));
    }

    auto piecesQueue = libs::collection::ThreadSafeQueue{std::vector(piecesIds.begin(), piecesIds.end())};

    std::cout << fmt::format("Already downloaded {} out of {} pieces, left {} pieces to download",
                             downloadedPiecesIds.size(), numberOfPieces, piecesQueue.size())
              << std::endl;

    const auto peerId = PeerIdGenerator::generate();

    boost::asio::io_context context;

    auto peerToPeerSessionManager = std::make_unique<PeerToPeerSessionManager>(
        context, piecesQueue, peerId, torrentFileInfo, pieceRepository, std::move(peersRetriever));

    peerToPeerSessionManager->startSessions();

    std::vector<std::thread> threads;

    const auto numberOfSupportedThreads = 160;

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

    if (piecesQueue.empty())
    {
        std::cout << "Torrent downloaded successfully" << std::endl;
    }
    else
    {
        std::cout << "There left " << piecesQueue.size() << " pieces to download" << std::endl;
    }
}
}
