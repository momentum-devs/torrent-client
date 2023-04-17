#include "PeerToPeerSessionManager.h"

#include <utility>

#include "fmt/format.h"

#include "loguru.hpp"
#include "PeerToPeerSessionImpl.h"

namespace core
{
namespace
{
const int refreshPeersTimeInSeconds{300};
}

PeerToPeerSessionManager::PeerToPeerSessionManager(boost::asio::io_context& ioContext,
                                                   libs::collection::ThreadSafeQueue<int>& piecesQueueInit,
                                                   std::string peerIdInit,
                                                   std::shared_ptr<TorrentFileInfo> torrentFileInfoInit,
                                                   std::shared_ptr<PieceRepository> pieceRepositoryInit,
                                                   std::unique_ptr<PeersRetriever> peersRetrieverInit)
    : context{ioContext},
      piecesQueue{piecesQueueInit},
      peerId{std::move(peerIdInit)},
      torrentFileInfo{std::move(torrentFileInfoInit)},
      pieceRepository{std::move(pieceRepositoryInit)},
      peersRetriever{std::move(peersRetrieverInit)},
      deadline(ioContext)
{
}

void PeerToPeerSessionManager::startSessions()
{
    refreshSessions();

    deadline.expires_from_now(boost::posix_time::seconds(refreshPeersTimeInSeconds));

    deadline.async_wait([this](boost::system::error_code) { checkDeadline(); });
}

void PeerToPeerSessionManager::closeSession(const PeerEndpoint& peerEndpoint)
{
    std::lock_guard<std::mutex> guard(lock);

    sessions.erase(peerEndpoint);
}

void PeerToPeerSessionManager::refreshSessions()
{
    for (const auto& announceUrl : torrentFileInfo->announceList)
    {

        boost::asio::post(context,
                          [this, announceUrl]
                          {
                              const auto retrievePeersPayload =
                                  RetrievePeersPayload{announceUrl,
                                                       torrentFileInfo->infoHash,
                                                       peerId,
                                                       "0",
                                                       "0",
                                                       "0",
                                                       std::to_string(torrentFileInfo->length),
                                                       "1"};

                              peersRetriever->retrievePeers(retrievePeersPayload,
                                                            [this](const std::vector<PeerEndpoint>& peersEndpoints)
                                                            { handleReceivedPeers(peersEndpoints); });
                          });
    }
}

void PeerToPeerSessionManager::handleReceivedPeers(const std::vector<PeerEndpoint>& peersEndpoints)
{
    std::lock_guard<std::mutex> guard(lock);

    LOG_S(INFO) << fmt::format("Received a list with {} peers.", peersEndpoints.size());

    for (const auto& peerEndpoint : peersEndpoints)
    {
        if (sessions.contains(peerEndpoint))
        {
            continue;
        }

        sessions.insert(
            {peerEndpoint, std::make_unique<PeerToPeerSessionImpl>(context, piecesQueue, peerEndpoint, peerId,
                                                                   torrentFileInfo, pieceRepository, *this)});

        sessions.at(peerEndpoint)->startSession();
    }
}

void PeerToPeerSessionManager::checkDeadline()
{
    if (deadline.expires_at() <= boost::asio::deadline_timer::traits_type::now())
    {
        refreshSessions();

        deadline.expires_from_now(boost::posix_time::seconds(refreshPeersTimeInSeconds));
    }

    deadline.async_wait([this](boost::system::error_code) { checkDeadline(); });
}

}
