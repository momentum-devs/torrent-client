#include "PeerToPeerSessionImpl.h"

#include <iostream>

#include "errors/PeerConnectionError.h"
#include "HandshakeMessageSerializer.h"
#include "MessageSerializer.h"

namespace
{
using iterator = boost::asio::buffers_iterator<boost::asio::streambuf::const_buffers_type>;

std::pair<iterator, bool> messageMatch(iterator begin, iterator end);
}

namespace core
{
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

    boost::asio::async_write(socket, boost::asio::buffer(serializedHandshakeMessage),
                             [this](boost::system::error_code errorCode, std::size_t bytes)
                             { onWriteHandshake(errorCode, bytes); });
}

void PeerToPeerSessionImpl::onWriteHandshake(boost::system::error_code error, std::size_t bytes_transferred)
{
    std::cout << "Write handshake to " << socket.remote_endpoint() << ": " << error.message()
              << ", bytes transferred: " << bytes_transferred << std::endl;

    const auto numberOfBytesInHandshake = 68;

    boost::asio::async_read(socket, response, boost::asio::transfer_exactly(numberOfBytesInHandshake),
                            [this](boost::system::error_code errorCode, std::size_t bytes)
                            { onReadHandshake(errorCode, bytes); });
}

void PeerToPeerSessionImpl::onReadHandshake(boost::system::error_code error, std::size_t bytes_transferred)
{
    std::string data{std::istreambuf_iterator<char>(&response), std::istreambuf_iterator<char>()};

    std::cout << "Read handshake from " << socket.remote_endpoint() << ": " << error.message()
              << ", bytes transferred: " << bytes_transferred << " handshake message: " << data << std::endl;

    const auto numberOfBytesInBitfield = static_cast<int>(ceilf(static_cast<float>(numberOfPieces) / 8.f)) + 5;

    readMessage();
}

void PeerToPeerSessionImpl::onWriteUnchokeMessage(boost::system::error_code error, std::size_t bytes_transferred)
{
    std::cout << "Write unchoke message to " << socket.remote_endpoint() << ": " << error.message()
              << ", bytes transferred: " << bytes_transferred << std::endl;

    const auto interestedMessage = Message{MessageId::Interested, ""};

    auto serializedInterestedMessage = MessageSerializer().serialize(interestedMessage);

    boost::asio::async_write(socket, boost::asio::buffer(serializedInterestedMessage),
                             [this](boost::system::error_code errorCode, std::size_t bytes)
                             { onWriteInterestedMessage(errorCode, bytes); });
}

void PeerToPeerSessionImpl::onWriteInterestedMessage(boost::system::error_code error, std::size_t bytes_transferred)
{
    std::cout << "Write interested message to " << socket.remote_endpoint() << ": " << error.message()
              << ", bytes transferred: " << bytes_transferred << std::endl;
    readMessage();
}

void PeerToPeerSessionImpl::readMessage()
{
    boost::asio::async_read_until(socket, response, messageMatch,
                                  [this](boost::system::error_code errorCode, std::size_t bytes)
                                  { onReadMessage(errorCode, bytes); });
}

void PeerToPeerSessionImpl::onReadMessage(boost::system::error_code error, std::size_t bytes_transferred)
{
    if (bytes_transferred == 0)
    {
        std::cout << "onReadMessage: Receive KeepAlive message" << std::endl;
        readMessage();
        return;
    }

    std::string data{std::istreambuf_iterator<char>(&response), std::istreambuf_iterator<char>()};

    auto message = MessageSerializer().deserialize(data);

    std::cout << "onReadMessage: Read " << toString(message.id) << " message to " << socket.remote_endpoint() << ": "
              << error.message() << ", bytes transferred: " << bytes_transferred << std::endl;

    switch (message.id)
    {
    case MessageId::Bitfield:
    {
        for (size_t i = 5; i < data.size(); i++)
        {
            std::cout << static_cast<int>(static_cast<unsigned char>(data[i])) << " ";
        }

        std::cout << std::endl;

        const auto unchokeMessage = Message{MessageId::Unchoke, ""};

        auto serializedUnchokeMessage = MessageSerializer().serialize(unchokeMessage);

        boost::asio::async_write(socket, boost::asio::buffer(serializedUnchokeMessage),
                                 [this](boost::system::error_code errorCode, std::size_t bytes)
                                 { onWriteUnchokeMessage(errorCode, bytes); });
        break;
    }
    case MessageId::Unchoke:
    {
        // TODO: Implement
        break;
    }
    case MessageId::Choke:
    {
        // TODO: Implement
        break;
    }
    case MessageId::Piece:
    {
        // TODO: Implement
        break;
    }
    case MessageId::Cancel:
    {
        // TODO: Implement
        break;
    }
    case MessageId::Have:
    {
        // TODO: Implement
        break;
    }
    case MessageId::Interested:
    {
        // TODO: Implement
        break;
    }
    case MessageId::NotInterested:
    {
        // TODO: Implement
        break;
    }
    case MessageId::Request:
    {
        // TODO: Implement
        break;
    }
    default:
        break;
    }

    readMessage();
}

}
namespace
{
std::pair<iterator, bool> messageMatch(iterator begin, iterator end)
{
    auto dataSize = end - begin;

    if (dataSize < 5)
    {
        return {begin, false};
    }

    int messageLength = (static_cast<unsigned char>(*begin) << 24) + (static_cast<unsigned char>(*(begin + 1)) << 16) +
                        (static_cast<unsigned char>(*(begin + 2)) << 8) + static_cast<unsigned char>(*(begin + 3));

    if (dataSize < messageLength + 4)
    {
        return {begin, false};
    }

    return {begin + messageLength + 4, true};
}
}
