#include <boost/asio.hpp>

#include "../tracker/PeerEndpoint.h"
#include "collection/ThreadSafeQueue.h"
#include "HandshakeMessage.h"
#include "PeerToPeerSession.h"

namespace core
{
class PeerToPeerSessionImpl : public PeerToPeerSession
{
public:
    PeerToPeerSessionImpl(boost::asio::io_context& ioContext, common::collection::ThreadSafeQueue<int>&,
                          PeerEndpoint peerEndpoint, std::string peerId);

    void startSession(const std::string& infoHash) override;

private:
    void sendHandshake(const HandshakeMessage& handshakeMessage);
    void onWriteHandshake(boost::system::error_code error, std::size_t bytes_transferred);
    void onReadHandshake(boost::system::error_code error, std::size_t bytes_transferred);
    void onWriteUnchokeMessage(boost::system::error_code error, std::size_t bytes_transferred);
    void onWriteInterestedMessage(boost::system::error_code error, std::size_t bytes_transferred);
    void readMessage();
    void onReadMessage(boost::system::error_code error, std::size_t bytes_transferred);

    boost::asio::ip::tcp::socket socket;
    std::string request;
    boost::asio::streambuf response;
    common::collection::ThreadSafeQueue<int>& piecesQueue;
    PeerEndpoint peerEndpoint;
    const std::string peerId;
};
}
