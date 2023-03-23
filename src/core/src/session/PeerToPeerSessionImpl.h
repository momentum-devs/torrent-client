#include <boost/asio.hpp>

#include "../torrentFile/TorrentFileInfo.h"
#include "../tracker/PeerEndpoint.h"
#include "bytes/Bitfield.h"
#include "collection/ThreadSafeQueue.h"
#include "HandshakeMessage.h"
#include "Message.h"
#include "PeerToPeerSession.h"
#include "PeerToPeerSessionManager.h"
#include "PieceRepository.h"

namespace core
{
class PeerToPeerSessionImpl : public PeerToPeerSession
{
public:
    PeerToPeerSessionImpl(boost::asio::io_context& ioContext, libs::collection::ThreadSafeQueue<int>&,
                          const PeerEndpoint& peerEndpoint, std::string peerId,
                          const std::shared_ptr<TorrentFileInfo>& torrentFileInfo,
                          const std::shared_ptr<PieceRepository> pieceRepository, PeerToPeerSessionManager& manager);

    void startSession() override;

private:
    void sendHandshake(const HandshakeMessage& handshakeMessage);
    void onReadHandshake(boost::system::error_code error);
    void onReadMessageLength(boost::system::error_code error, std::size_t bytesTransferred);
    void onReadMessage(boost::system::error_code error, std::size_t bytesTransferred, std::size_t bytesToRead);
    void handleBitfieldMessage(const Message& bitfieldMessage);
    void handleUnchokeMessage();
    void handleChokeMessage();
    void handlePieceMessage(const Message& pieceMessage);
    void handleHaveMessage(const Message& haveMessage);
    void returnPieceToQueue();
    void asyncRead(std::size_t bytesToRead, std::function<void(boost::system::error_code, std::size_t)> readHandler);
    void asyncWrite(boost::asio::const_buffer writeBuffer,
                    std::function<void(boost::system::error_code, std::size_t)> writeHandler);
    void checkDeadline();

    boost::asio::ip::tcp::socket socket;
    std::string request;
    boost::asio::streambuf response;
    libs::collection::ThreadSafeQueue<int>& piecesQueue;
    PeerEndpoint peerEndpoint;
    const std::string peerId;
    bool isChoked;
    bool hasErrorOccurred;
    std::optional<int> pieceIndex;
    const std::shared_ptr<TorrentFileInfo> torrentFileInfo;
    const std::shared_ptr<PieceRepository> pieceRepository;
    int pieceBytesRead;
    std::optional<libs::bytes::Bitfield> bitfield;
    std::basic_string<unsigned char> pieceData;
    boost::asio::ip::basic_endpoint<boost::asio::ip::tcp> endpoint;
    boost::asio::deadline_timer deadline;
    PeerToPeerSessionManager& sessionManager;
};
}
