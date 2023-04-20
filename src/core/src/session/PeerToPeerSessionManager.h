#pragma once

#include <atomic>
#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/io_context.hpp>
#include <memory>
#include <unordered_map>

#include "../torrentFile/TorrentFileInfo.h"
#include "../tracker/PeerEndpoint.h"
#include "../tracker/PeersRetriever.h"
#include "collection/ThreadSafeQueue.h"
#include "PeerToPeerSession.h"
#include "PieceQueueManager.h"
#include "PieceRepository.h"

namespace core
{
class PeerToPeerSessionManager
{
public:
    PeerToPeerSessionManager(boost::asio::io_context& ioContext, PieceQueueManager&, std::string peerId,
                             std::shared_ptr<TorrentFileInfo> torrentFileInfo,
                             std::shared_ptr<PieceRepository> pieceRepository,
                             std::unique_ptr<PeersRetriever> peerRetriever);

    void startSessions();
    void closeSession(const PeerEndpoint& peerEndpoint);

private:
    void refreshSessions();
    void handleReceivedPeers(const std::vector<PeerEndpoint>&);
    void checkDeadline();

    boost::asio::io_context& context;
    PieceQueueManager& piecesQueueManager;
    const std::string peerId;
    const std::shared_ptr<TorrentFileInfo> torrentFileInfo;
    const std::shared_ptr<PieceRepository> pieceRepository;
    std::unique_ptr<PeersRetriever> peersRetriever;
    std::map<PeerEndpoint, std::unique_ptr<PeerToPeerSession>> sessions;
    std::mutex lock;
    boost::asio::deadline_timer deadline;
};
}
