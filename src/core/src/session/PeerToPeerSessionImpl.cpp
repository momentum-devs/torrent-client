#include "PeerToPeerSessionImpl.h"

#include <bitset>
#include <iostream>
#include <utility>

#include "errors/PeerConnectionError.h"
#include "HandshakeMessageSerializer.h"
#include "MessageSerializer.h"

namespace
{
using iterator = boost::asio::buffers_iterator<boost::asio::streambuf::const_buffers_type>;

std::pair<iterator, bool> messageMatch(iterator begin, iterator end);
}

PeerToPeerSessionImpl::PeerToPeerSessionImpl(boost::asio::io_context& ioContext, unsigned numberOfPiecesInit,
                                             PeerEndpoint peerEndpointInit, std::string peerIdInit)
    : socket(ioContext),
      numberOfPieces{numberOfPiecesInit},
      peerEndpoint{std::move(peerEndpointInit)},
      peerId{std::move(peerIdInit)}
{
}

void PeerToPeerSessionImpl::startSession(const std::string& infoHash)
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

    auto handshakeMessage = HandshakeMessage{"BitTorrent protocol", infoHash, peerId};

    sendHandshake(handshakeMessage);
}

void PeerToPeerSessionImpl::sendHandshake(const HandshakeMessage& handshakeMessage)
{
    auto serializedHandshakeMessage = HandshakeMessageSerializer().serialize(handshakeMessage);

    std::cout << "Sending handshake message: " << serializedHandshakeMessage << " to: " << socket.remote_endpoint()
              << std::endl;

    boost::asio::async_write(
        socket, boost::asio::buffer(serializedHandshakeMessage),
        std::bind(&PeerToPeerSessionImpl::onWriteHandshake, this, std::placeholders::_1, std::placeholders::_2));
}

void PeerToPeerSessionImpl::onWriteHandshake(boost::system::error_code error, std::size_t bytes_transferred)
{
    std::cout << "Write handshake to " << socket.remote_endpoint() << ": " << error.message()
              << ", bytes transferred: " << bytes_transferred << std::endl;

    const auto numberOfBytesInHandshake = 68;

    boost::asio::async_read(
        socket, response, boost::asio::transfer_exactly(numberOfBytesInHandshake),
        std::bind(&PeerToPeerSessionImpl::onReadHandshake, this, std::placeholders::_1, std::placeholders::_2));
}

void PeerToPeerSessionImpl::onReadHandshake(boost::system::error_code error, std::size_t bytes_transferred)
{
    std::string data{std::istreambuf_iterator<char>(&response), std::istreambuf_iterator<char>()};

    std::cout << "Read handshake from " << socket.remote_endpoint() << ": " << error.message()
              << ", bytes transferred: " << bytes_transferred << " handshake message: " << data << std::endl;

    const auto numberOfBytesInBitfield = static_cast<int>(ceilf(static_cast<float>(numberOfPieces) / 8.f)) + 5;

    boost::asio::async_read(socket, response, boost::asio::transfer_exactly(numberOfBytesInBitfield),
                            [this](boost::system::error_code errorCode, std::size_t bytes)
                            { onReadBitfieldMessage(errorCode, bytes); });
}

void PeerToPeerSessionImpl::onReadBitfieldMessage(boost::system::error_code error, std::size_t bytes_transferred)
{
    std::string data{std::istreambuf_iterator<char>(&response), std::istreambuf_iterator<char>()};

    for (size_t i = 5; i < data.size(); i++)
    {
        std::cout << static_cast<int>(static_cast<unsigned char>(data[i])) << " ";
    }

    std::cout << "Read bitfield message from " << socket.remote_endpoint() << ": " << error.message()
              << ", bytes transferred: " << bytes_transferred << " bitfield message: " << data << std::endl;

    const auto unchokeMessage = Message{MessageId::Unchoke, ""};

    auto serializedUnchokeMessage = MessageSerializer().serialize(unchokeMessage);

    boost::asio::async_write(
        socket, boost::asio::buffer(serializedUnchokeMessage),
        std::bind(&PeerToPeerSessionImpl::onWriteUnchokeMessage, this, std::placeholders::_1, std::placeholders::_2));
}

void PeerToPeerSessionImpl::onWriteUnchokeMessage(boost::system::error_code error, std::size_t bytes_transferred)
{
    std::cout << "Write unchoke message to " << socket.remote_endpoint() << ": " << error.message()
              << ", bytes transferred: " << bytes_transferred << std::endl;

    const auto interestedMessage = Message{MessageId::Interested, ""};

    auto serializedInterestedMessage = MessageSerializer().serialize(interestedMessage);

    boost::asio::async_write(socket, boost::asio::buffer(serializedInterestedMessage),
                             std::bind(&PeerToPeerSessionImpl::onWriteInterestedMessage, this, std::placeholders::_1,
                                       std::placeholders::_2));
}

void PeerToPeerSessionImpl::onWriteInterestedMessage(boost::system::error_code error, std::size_t bytes_transferred)
{
    std::cout << "Write interested message to " << socket.remote_endpoint() << ": " << error.message()
              << ", bytes transferred: " << bytes_transferred << std::endl;
}

void PeerToPeerSessionImpl::readMessage()
{
    boost::asio::async_read_until(
        socket, response, messageMatch,
        std::bind(&PeerToPeerSessionImpl::onReadMessage, this, std::placeholders::_1, std::placeholders::_2));
}

void PeerToPeerSessionImpl::onReadMessage(boost::system::error_code error, std::size_t bytes_transferred) {}

namespace
{
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
