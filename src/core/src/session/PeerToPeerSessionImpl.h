#include <boost/asio.hpp>

#include "../tracker/PeerEndpoint.h"
#include "bytes/Bitfield.h"
#include "collection/ThreadSafeQueue.h"
#include "HandshakeMessage.h"
#include "PeerToPeerSession.h"

namespace core
{
class PeerToPeerSessionImpl : public PeerToPeerSession
{
public:
    PeerToPeerSessionImpl(boost::asio::io_context& ioContext, libs::collection::ThreadSafeQueue<int>&,
                          const PeerEndpoint& peerEndpoint, const std::string& peerId, int pieceSize);
    void startSession(const std::string& infoHash) override;

private:
    void sendHandshake(const HandshakeMessage& handshakeMessage);
    void onReadHandshake(boost::system::error_code error, std::size_t bytesTransferred, const std::string& infoHash);
    void onReadMessageLength(boost::system::error_code error, std::size_t bytesTransferred);
    void onReadMessage(boost::system::error_code error, std::size_t bytesTransferred, std::size_t bytesToRead);
    void returnPieceToQueue();

    boost::asio::ip::tcp::socket socket;
    std::string request;
    boost::asio::streambuf response;
    libs::collection::ThreadSafeQueue<int>& piecesQueue;
    PeerEndpoint peerEndpoint;
    const std::string peerId;
    bool isChoked;
    std::optional<int> pieceIndex;
    int pieceSize;
    int pieceBytesRead;
    int maxBlockSize;
    std::optional<libs::bytes::Bitfield> bitfield;
};
}
