#include <boost/asio.hpp>

#include "HandshakeMessage.h"
#include "PeerEndpoint.h"
#include "PeerToPeerSession.h"

class PeerToPeerSessionImpl : public PeerToPeerSession
{
public:
    PeerToPeerSessionImpl(boost::asio::io_context& ioContext, unsigned numberOfPieces, PeerEndpoint peerEndpoint,
                          std::string peerId);
    void startSession(const std::string& infoHash) override;

private:
    void sendHandshake(const HandshakeMessage& handshakeMessage);
    void onWriteHandshake(boost::system::error_code error, std::size_t bytes_transferred);
    void onReadHandshake(boost::system::error_code error, std::size_t bytes_transferred);
    void onReadBitfieldMessage(boost::system::error_code error, std::size_t bytes_transferred);
    void onWriteUnchokeMessage(boost::system::error_code error, std::size_t bytes_transferred);
    void onWriteInterestedMessage(boost::system::error_code error, std::size_t bytes_transferred);
    void readMessage();
    void onReadMessage(boost::system::error_code error, std::size_t bytes_transferred);

    boost::asio::ip::tcp::socket socket;
    std::string request;
    boost::asio::streambuf response;
    const unsigned numberOfPieces;
    PeerEndpoint peerEndpoint;
    const std::string peerId;
};
