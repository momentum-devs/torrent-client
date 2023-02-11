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
    PeerToPeerSessionImpl(boost::asio::io_context& ioContext, common::collection::ThreadSafeQueue<int>&,
                          const PeerEndpoint& peerEndpoint, const std::string& peerId, int pieceSize);
    void startSession(const std::string& infoHash) override;

private:
    void sendHandshake(const HandshakeMessage& handshakeMessage);
    void onReadHandshake(boost::system::error_code error, std::size_t bytesTransferred, const std::string& infoHash);
    void onReadMessageLength(boost::system::error_code error, std::size_t bytesTransferred);
    void onReadMessage(boost::system::error_code error, std::size_t bytesTransferred, std::size_t bytesToRead);
    void returnPieceToQueue();
    void asyncRead(std::size_t bytesToRead, std::function<void(boost::system::error_code, std::size_t)> readHandler);
    void asyncWrite(boost::asio::const_buffer writeBuffer,
                    std::function<void(boost::system::error_code, std::size_t)> writeHandler);

    boost::asio::ip::tcp::socket socket;
    std::string request;
    boost::asio::streambuf response;
    common::collection::ThreadSafeQueue<int>& piecesQueue;
    PeerEndpoint peerEndpoint;
    const std::string peerId;
    bool isChoked;
    bool hasErrorOccurred;
    std::optional<int> pieceIndex;
    int pieceSize;
    int pieceBytesRead;
    int maxBlockSize;
    std::optional<common::bytes::Bitfield> bitfield;
};
}
