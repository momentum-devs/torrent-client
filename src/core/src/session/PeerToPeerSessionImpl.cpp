#include "PeerToPeerSessionImpl.h"

#include <iostream>

#include "fmt/format.h"

#include "bytes/Bitfield.h"
#include "bytes/BytesConverter.h"
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
PeerToPeerSessionImpl::PeerToPeerSessionImpl(boost::asio::io_context& ioContext,
                                             common::collection::ThreadSafeQueue<int>& piecesQueueInit,
                                             PeerEndpoint peerEndpointInit, std::string peerIdInit, int pieceSizeInit)
    : socket(ioContext),
      piecesQueue{piecesQueueInit},
      peerEndpoint{std::move(peerEndpointInit)},
      peerId{std::move(peerIdInit)},
      isChoked{true},
      pieceIndex{0},
      pieceSize{pieceSizeInit},
      pieceBytesRead{0},
      maxBlockSize{16384} // TODO: Make bigger number work property
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

    const auto handshakeMessage = HandshakeMessage{"BitTorrent protocol", infoHash, peerId};

    sendHandshake(handshakeMessage);
}

void PeerToPeerSessionImpl::sendHandshake(const HandshakeMessage& handshakeMessage)
{
    const auto serializedHandshakeMessage = HandshakeMessageSerializer().serialize(handshakeMessage);

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

    readMessage();
}

void PeerToPeerSessionImpl::onWriteUnchokeMessage(boost::system::error_code error, std::size_t bytes_transferred)
{
    std::cout << "Write unchoke message to " << socket.remote_endpoint() << ": " << error.message()
              << ", bytes transferred: " << bytes_transferred << std::endl;

    const auto interestedMessage = Message{MessageId::Interested, std::basic_string<unsigned char>{}};

    const auto serializedInterestedMessage = MessageSerializer().serialize(interestedMessage);

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
    boost::asio::async_read(socket, response, boost::asio::transfer_exactly(4),
                            [this](boost::system::error_code errorCode, std::size_t bytes)
                            { onReadMessageLength(errorCode, bytes); });
}

void PeerToPeerSessionImpl::onReadMessageLength(boost::system::error_code error, std::size_t bytes_transferred)
{
    const std::basic_string<unsigned char> data{std::istreambuf_iterator<char>(&response),
                                                std::istreambuf_iterator<char>()};

    int bytesToRead = common::bytes::BytesConverter::bytesToInt(data);

    if (bytesToRead == 0)
    {
        readMessage();
    }

    std::cout << bytesToRead << std::endl;

    boost::asio::async_read(socket, response, boost::asio::transfer_exactly(bytesToRead),
                            [this](boost::system::error_code errorCode, std::size_t bytes)
                            { onReadMessage(errorCode, bytes); });
}

void PeerToPeerSessionImpl::onReadMessage(boost::system::error_code error, std::size_t bytes_transferred)
{
    if (bytes_transferred == 0 or bytes_transferred == 4)
    {
        readMessage();

        return;
    }

    const std::basic_string<unsigned char> data{std::istreambuf_iterator<char>(&response),
                                                std::istreambuf_iterator<char>()};

    const auto message = MessageSerializer().deserialize(data);

    std::cout << "onReadMessage: Read " << toString(message.id) << " message from " << socket.remote_endpoint() << ": "
              << error.message() << ", bytes transferred: " << bytes_transferred << std::endl;

    switch (message.id)
    {
    case MessageId::Bitfield:
    {
        const auto bitfieldData = data.substr(5);

        const auto bitfield = common::bytes::Bitfield{bitfieldData};

        std::cout << "Bitfield: " << bitfield.toString() << std::endl;

        const auto unchokeMessage = Message{MessageId::Unchoke, std::basic_string<unsigned char>{}};

        const auto serializedUnchokeMessage = MessageSerializer().serialize(unchokeMessage);

        boost::asio::async_write(socket, boost::asio::buffer(serializedUnchokeMessage),
                                 [this](boost::system::error_code errorCode, std::size_t bytes)
                                 { onWriteUnchokeMessage(errorCode, bytes); });
        break;
    }
    case MessageId::Unchoke:
    {
        isChoked = false;

        const auto requestMessage =
            Message{MessageId::Request, common::bytes::BytesConverter::intToBytes(pieceIndex) +
                                            common::bytes::BytesConverter::intToBytes(pieceBytesRead) +
                                            common::bytes::BytesConverter::intToBytes(maxBlockSize)};

        auto serializedRequestMessage = MessageSerializer().serialize(requestMessage);

        boost::asio::async_write(socket, boost::asio::buffer(serializedRequestMessage),
                                 [this](boost::system::error_code errorCode, std::size_t bytes)
                                 {
                                     std::cout << "Write request message to " << socket.remote_endpoint() << ": "
                                               << errorCode.message() << ", bytes transferred: " << bytes << std::endl;
                                 });
        break;
    }
    case MessageId::Choke:
    {
        isChoked = true;
        break;
    }
    case MessageId::Piece:
    {
        pieceBytesRead += maxBlockSize;
        std::cout << pieceBytesRead << '/' << pieceSize << std::endl;

        if (pieceBytesRead >= pieceSize)
        {
            std::cout << "piece " << pieceIndex << " downloaded" << std::endl;
            break;
        }

        if (pieceSize - pieceBytesRead < maxBlockSize)
        {
            maxBlockSize = pieceSize - pieceBytesRead;
        }

        const auto requestMessage =
            Message{MessageId::Request, common::bytes::BytesConverter::intToBytes(pieceIndex) +
                                            common::bytes::BytesConverter::intToBytes(pieceBytesRead) +
                                            common::bytes::BytesConverter::intToBytes(maxBlockSize)};

        auto serializedRequestMessage = MessageSerializer().serialize(requestMessage);

        boost::asio::async_write(socket, boost::asio::buffer(serializedRequestMessage),
                                 [this](boost::system::error_code errorCode, std::size_t bytes)
                                 {
                                     std::cout << "Write request message to " << socket.remote_endpoint() << ": "
                                               << errorCode.message() << ", bytes transferred: " << bytes << std::endl;
                                 });
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

namespace
{
std::pair<iterator, bool> messageMatch(iterator begin, iterator end)
{
    const auto dataSize = end - begin;

    if (dataSize < 4)
    {
        std::cout << "1. too less data in buffer" << std::endl;
        return {begin, false};
    }

    const std::basic_string<unsigned char> messageLengthInBytes{
        static_cast<unsigned char>(*begin), static_cast<unsigned char>(*(begin + 1)),
        static_cast<unsigned char>(*(begin + 2)), static_cast<unsigned char>(*(begin + 3))};

    const auto messageLength = common::bytes::BytesConverter::bytesToInt(messageLengthInBytes);

    if (messageLength == 0)
    {
        std::cout << "2. " << std::endl;
        return {begin + 4, false};
    }

    if (dataSize < messageLength + 4)
    {
        std::cout << fmt::format("3. bytes in buffer: {}, message length: {}", dataSize, messageLength + 4)
                  << std::endl;
        return {begin, false};
    }

    std::cout << "4." << std::endl;
    return {begin + messageLength + 4, true};
}
}
}
