#include <boost/asio.hpp>

#include "HandshakeMessage.h"
#include "PeerEndpoint.h"

class PeerConnector
{
public:
    PeerConnector(boost::asio::io_context& ioContext, const PeerEndpoint& peerEndpoint,
                  const HandshakeMessage& handshakeMessage);

private:
    void sendHandshake(const HandshakeMessage& handshakeMessage);
    void onWriteHandshake(boost::system::error_code error, std::size_t bytes_transferred);
    void onReadHandshake(boost::system::error_code error, std::size_t bytes_transferred);

    boost::asio::ip::tcp::socket socket;
    std::string request;
    boost::asio::streambuf response;
};
