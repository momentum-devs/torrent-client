#include "PeerConnector.h"

#include <bitset>
#include <iostream>

#include "errors/PeerConnectionError.h"
#include "HandshakeMessageSerializer.h"
#include "MessageSerializer.h"

namespace
{
using iterator = boost::asio::buffers_iterator<boost::asio::streambuf::const_buffers_type>;

std::pair<iterator, bool> messageMatch(iterator begin, iterator end)
{
    auto dataSize = end - begin;

    if (dataSize < 5)
    {
        return {begin, false};
    }

    int messageLength = (*begin << 24) + (*(begin + 1) << 16) + (*(begin + 2) << 8) + *(begin + 3);

    if (dataSize < messageLength + 4)
    {
        return {begin, false};
    }

    return {begin + messageLength + 4, true};
}
}

PeerConnector::PeerConnector(boost::asio::io_context& ioContext, const PeerEndpoint& peerEndpoint,
                             const HandshakeMessage& handshakeMessage, unsigned numberOfPiecesInit)
    : socket(ioContext), numberOfPieces{numberOfPiecesInit}
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

    const auto numberOfBytesInHandshake = 68;

    boost::asio::async_read(
        socket, response, boost::asio::transfer_exactly(numberOfBytesInHandshake),
        std::bind(&PeerConnector::onReadHandshake, this, std::placeholders::_1, std::placeholders::_2));
}

void PeerConnector::onReadHandshake(boost::system::error_code error, std::size_t bytes_transferred)
{
    std::string data{std::istreambuf_iterator<char>(&response), std::istreambuf_iterator<char>()};

    std::cout << "Read handshake from " << socket.remote_endpoint() << ": " << error.message()
              << ", bytes transferred: " << bytes_transferred << " handshake message: " << data << std::endl;

    const auto numberOfBytesInBitfield = static_cast<int>(ceilf(static_cast<float>(numberOfPieces) / 8.f)) + 5;

    boost::asio::async_read(socket, response, boost::asio::transfer_exactly(numberOfBytesInBitfield),
                            [this](boost::system::error_code errorCode, std::size_t bytes)
                            { onReadBitfieldMessage(errorCode, bytes); });
}

void PeerConnector::onReadBitfieldMessage(boost::system::error_code error, std::size_t bytes_transferred)
{
    std::string data{std::istreambuf_iterator<char>(&response), std::istreambuf_iterator<char>()};

    for (int i = 5; i < data.size(); i++)
    {
        std::cout << static_cast<int>(static_cast<unsigned char>(data[i])) << " ";
    }

    std::cout << "Read bitfield message from " << socket.remote_endpoint() << ": " << error.message()
              << ", bytes transferred: " << bytes_transferred << " bitfield message: " << data << std::endl;

    const auto unchokeMessage = Message{MessageId::Unchoke, ""};

    auto serializedUnchokeMessage = MessageSerializer().serialize(unchokeMessage);

    boost::asio::async_write(
        socket, boost::asio::buffer(serializedUnchokeMessage),
        std::bind(&PeerConnector::onWriteUnchokeMessage, this, std::placeholders::_1, std::placeholders::_2));
}

void PeerConnector::onWriteUnchokeMessage(boost::system::error_code error, std::size_t bytes_transferred)
{
    std::cout << "Write unchoke message to " << socket.remote_endpoint() << ": " << error.message()
              << ", bytes transferred: " << bytes_transferred << std::endl;

    const auto interestedMessage = Message{MessageId::Interested, ""};

    auto serializedInterestedMessage = MessageSerializer().serialize(interestedMessage);

    boost::asio::async_write(
        socket, boost::asio::buffer(serializedInterestedMessage),
        std::bind(&PeerConnector::onWriteInterestedMessage, this, std::placeholders::_1, std::placeholders::_2));
}

void PeerConnector::onWriteInterestedMessage(boost::system::error_code error, std::size_t bytes_transferred)
{
    std::cout << "Write interested message to " << socket.remote_endpoint() << ": " << error.message()
              << ", bytes transferred: " << bytes_transferred << std::endl;
}

void PeerConnector::readMessage()
{
    boost::asio::async_read_until(
        socket, response, messageMatch,
        std::bind(&PeerConnector::onReadMessage, this, std::placeholders::_1, std::placeholders::_2));
}

void PeerConnector::onReadMessage(boost::system::error_code error, std::size_t bytes_transferred) {}
