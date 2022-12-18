#include "PeerConnector.h"

#include <iostream>

#include "errors/PeerConnectionError.h"
#include "HandshakeMessageSerializer.h"

PeerConnector::PeerConnector(boost::asio::io_context& ioContext, const PeerEndpoint& peerEndpoint,
                             const HandshakeMessage& handshakeMessage)
    : socket(ioContext)
{
    boost::asio::ip::address address = boost::asio::ip::make_address(peerEndpoint.address);

    boost::asio::ip::tcp::endpoint endpoint(address, peerEndpoint.port);

    boost::system::error_code error;

    socket.connect(endpoint, error);

    if (error)
    {
        throw errors::PeerConnectionError{error.message()};
    }

    std::cout << "Connected to: " << endpoint << std::endl;

    sendHandshake(handshakeMessage);
}

void PeerConnector::sendHandshake(const HandshakeMessage& handshakeMessage)
{
    auto serializedHandshakeMessage = HandshakeMessageSerializer().serialize(handshakeMessage);

    std::cout << "Sending handshake message: " << serializedHandshakeMessage << " to: " << socket.remote_endpoint()
              << std::endl;

    boost::asio::async_write(
        socket, boost::asio::buffer(serializedHandshakeMessage),
        std::bind(&PeerConnector::onWriteHandshake, this, std::placeholders::_1, std::placeholders::_2));
}

void PeerConnector::onWriteHandshake(boost::system::error_code error, std::size_t bytes_transferred)
{
    std::cout << "Write handshake to " << socket.remote_endpoint() << ": " << error.message()
              << ", bytes transferred: " << bytes_transferred << std::endl;

    boost::asio::async_read(
        socket, response, boost::asio::transfer_exactly(68),
        std::bind(&PeerConnector::onReadHandshake, this, std::placeholders::_1, std::placeholders::_2));
}

void PeerConnector::onReadHandshake(boost::system::error_code error, std::size_t bytes_transferred)
{
    std::cout << "Read handshake from " << socket.remote_endpoint() << ": " << error.message()
              << ", bytes transferred: " << bytes_transferred << " handshake: " << std::istream(&response).rdbuf()
              << std::endl;
}
