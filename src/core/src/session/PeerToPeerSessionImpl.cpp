#include "PeerToPeerSessionImpl.h"

#include <iostream>

#include "fmt/format.h"

#include "bytes/Bitfield.h"
#include "bytes/BytesConverter.h"
#include "encoder/HexEncoder.h"
#include "HandshakeMessageSerializer.h"
#include "MessageSerializer.h"

namespace
{
using iterator = boost::asio::buffers_iterator<boost::asio::streambuf::const_buffers_type>;
}

namespace core
{
PeerToPeerSessionImpl::PeerToPeerSessionImpl(boost::asio::io_context& ioContext,
                                             libs::collection::ThreadSafeQueue<int>& piecesQueueInit,
                                             const PeerEndpoint& peerEndpointInit, const std::string& peerIdInit,
                                             int pieceSizeInit)
    : socket(ioContext),
      piecesQueue{piecesQueueInit},
      peerEndpoint{peerEndpointInit},
      peerId{peerIdInit},
      isChoked{true},
      pieceIndex{std::nullopt},
      pieceSize{pieceSizeInit},
      pieceBytesRead{0},
      maxBlockSize{16384},
      bitfield{std::nullopt}
{
}

void PeerToPeerSessionImpl::startSession(const std::string& infoHash)
{
    const auto address = boost::asio::ip::make_address(peerEndpoint.address);

    boost::asio::ip::tcp::endpoint endpoint(address, peerEndpoint.port);

    boost::system::error_code error;

    socket.connect(endpoint, error);

    if (error)
    {
        std::cerr << "PeerConnectionError: " << error.message() << std::endl;

        return;
    }

    std::cout << "Connected to: " << endpoint << std::endl;

    const auto handshakeMessage = HandshakeMessage{"BitTorrent protocol", infoHash, peerId};

    sendHandshake(handshakeMessage);
}

void PeerToPeerSessionImpl::sendHandshake(const HandshakeMessage& handshakeMessage)
{
    const auto serializedHandshakeMessage = HandshakeMessageSerializer().serialize(handshakeMessage);

    const auto infoHash = handshakeMessage.infoHash;

    boost::asio::async_write(socket, boost::asio::buffer(serializedHandshakeMessage),
                             [this, infoHash](boost::system::error_code, std::size_t bytesTransferred)
                             {
                                 std::cout << "Sent handshake to " << socket.remote_endpoint()
                                           << ", bytes transferred: " << bytesTransferred << std::endl;

                                 const auto numberOfBytesInHandshake = 68;

                                 boost::asio::async_read(
                                     socket, response, boost::asio::transfer_exactly(numberOfBytesInHandshake),
                                     [this, infoHash](boost::system::error_code error, std::size_t bytes)
                                     { onReadHandshake(error, bytes, infoHash); });
                             });
}

void PeerToPeerSessionImpl::onReadHandshake(boost::system::error_code error, std::size_t bytesTransferred,
                                            const std::string& infoHash)
{
    if (error)
    {
        std::cerr << error.message() << std::endl;

        return;
    }

    const std::string data{std::istreambuf_iterator<char>(&response), std::istreambuf_iterator<char>()};

    const auto receivedInfoHash = libs::encoder::HexEncoder::encode(data.substr(28, 20));

    if (infoHash != receivedInfoHash)
    {
        std::cerr << fmt::format("Receive handshake with different info hash, should be {}, get {}.", infoHash,
                                 receivedInfoHash)
                  << std::endl;

        return;
    }

    std::cout << "Read handshake from " << socket.remote_endpoint() << ", bytes transferred: " << bytesTransferred
              << std::endl;

    boost::asio::async_read(socket, response, boost::asio::transfer_exactly(4),
                            [this](boost::system::error_code error, std::size_t bytes)
                            { onReadMessageLength(error, bytes); });
}

void PeerToPeerSessionImpl::onReadMessageLength(boost::system::error_code error, std::size_t)
{
    if (error)
    {
        returnPieceToQueue();

        std::cerr << error.message() << std::endl;

        return;
    }

    if (response.size() == 0) // Socket KeepAlive
    {
        boost::asio::async_read(socket, response, boost::asio::transfer_exactly(4),
                                [this](boost::system::error_code error, std::size_t bytes)
                                { onReadMessageLength(error, bytes); });

        return;
    }

    if (response.size() != 4)
    {
        returnPieceToQueue();

        std::cerr << fmt::format("Message length {} not equal 4.", response.size()) << std::endl;

        return;
    }

    const std::basic_string<unsigned char> bytes{std::istreambuf_iterator<char>(&response),
                                                 std::istreambuf_iterator<char>()};

    const auto bytesToRead = libs::bytes::BytesConverter::bytesToInt(bytes);

    std::cout << fmt::format("{} bytes should be read from payload data.", bytesToRead) << std::endl;

    if (bytesToRead == 0)
    {
        boost::asio::async_read(socket, response, boost::asio::transfer_exactly(4),
                                [this](boost::system::error_code error, std::size_t bytes)
                                { onReadMessageLength(error, bytes); });

        return;
    }

    boost::asio::async_read(socket, response, boost::asio::transfer_exactly(bytesToRead),
                            [this, bytesToRead](boost::system::error_code error, std::size_t bytes)
                            { onReadMessage(error, bytes, bytesToRead); });
}

void PeerToPeerSessionImpl::onReadMessage(boost::system::error_code error, std::size_t, std::size_t bytesToRead)
{
    if (error)
    {
        returnPieceToQueue();

        std::cerr << error.message() << std::endl;

        return;
    }

    if (response.size() != bytesToRead)
    {
        returnPieceToQueue();

        std::cerr << fmt::format("Message length {} not equal {}.", response.size(), bytesToRead) << std::endl;

        return;
    }

    const std::basic_string<unsigned char> data{std::istreambuf_iterator<char>(&response),
                                                std::istreambuf_iterator<char>()};

    const auto message = MessageSerializer().deserialize(data);

    std::cout << fmt::format("Received message {} which has {} bytes.", toString(message.id), bytesToRead) << std::endl;

    switch (message.id)
    {
    case MessageId::Bitfield:
    {
        const auto bitfieldData = data.substr(5);

        bitfield.emplace(libs::bytes::Bitfield{bitfieldData});

        const auto interestedMessage = Message{MessageId::Interested, std::basic_string<unsigned char>{}};

        const auto serializedInterestedMessage = MessageSerializer().serialize(interestedMessage);

        boost::asio::async_write(socket, boost::asio::buffer(serializedInterestedMessage),
                                 [this](boost::system::error_code error, std::size_t bytesTransferred)
                                 {
                                     if (error)
                                     {
                                         returnPieceToQueue();

                                         std::cerr << error.message() << std::endl;

                                         return;
                                     }

                                     std::cout << "Sent interested message to " << socket.remote_endpoint()
                                               << ", bytes transferred: " << bytesTransferred << std::endl;
                                 });

        break;
    }
    case MessageId::Unchoke:
    {
        if (!isChoked)
        {
            break;
        }

        isChoked = false;

        auto numberOfPiecesIndexes = static_cast<int>(piecesQueue.size());

        auto piecesIndexIter = 0;

        for(; piecesIndexIter < numberOfPiecesIndexes; ++piecesIndexIter)
        {
            pieceIndex = piecesQueue.pop();

            if (!pieceIndex)
            {
                std::cout << "No piece in queue - close connection" << std::endl;

                return;
            }

            if (bitfield->hasBitSet(*pieceIndex))
            {
                break;
            }
            else
            {
                piecesQueue.push(*pieceIndex);
            }
        }

        if(piecesIndexIter == numberOfPiecesIndexes)
        {
            std::cout << "No piece in queue available - close connection" << std::endl;

            return;
        }

        const auto requestMessage =
            Message{MessageId::Request, libs::bytes::BytesConverter::intToBytes(*pieceIndex) +
                                            libs::bytes::BytesConverter::intToBytes(pieceBytesRead) +
                                            libs::bytes::BytesConverter::intToBytes(maxBlockSize)};

        const auto serializedRequestMessage = MessageSerializer().serialize(requestMessage);

        boost::asio::async_write(
            socket, boost::asio::buffer(serializedRequestMessage),
            [this](boost::system::error_code error, std::size_t bytes)
            {
                if (error)
                {
                    returnPieceToQueue();

                    std::cerr << error.message() << std::endl;

                    return;
                }

                std::cout << "Sent request message to " << socket.remote_endpoint()
                          << fmt::format(
                                 " to download piece {}, beginning {}, bytes to read {}.\nBytes transferred: {}",
                                 *pieceIndex, pieceBytesRead, maxBlockSize, bytes)
                          << std::endl;
            });
        break;
    }
    case MessageId::Choke:
    {
        isChoked = true;

        returnPieceToQueue();

        break;
    }
    case MessageId::Piece:
    {
        pieceBytesRead += maxBlockSize;

        const auto blockPieceIndex = libs::bytes::BytesConverter::bytesToInt(message.payload.substr(0, 4));

        const auto blockNumber = libs::bytes::BytesConverter::bytesToInt(message.payload.substr(4, 4));

        std::cout << fmt::format("Downloaded {} part of the piece {}. Downloaded {} of {} bytes.", blockNumber,
                                 *pieceIndex, pieceBytesRead, pieceSize)
                  << std::endl;

        if (pieceBytesRead >= pieceSize)
        {
            std::cout << "Piece " << blockPieceIndex << " downloaded" << std::endl;

            auto numberOfPiecesIndexes = static_cast<int>(piecesQueue.size());

            auto piecesIndexIter = 0;

            for(; piecesIndexIter < numberOfPiecesIndexes; ++piecesIndexIter)
            {
                pieceIndex = piecesQueue.pop();

                if (!pieceIndex)
                {
                    std::cout << "No piece in queue - close connection" << std::endl;

                    return;
                }

                if (bitfield->hasBitSet(*pieceIndex))
                {
                    break;
                }
                else
                {
                    piecesQueue.push(*pieceIndex);
                }
            }

            if(piecesIndexIter == numberOfPiecesIndexes)
            {
                std::cout << "No piece in queue available - close connection" << std::endl;

                return;
            }

            pieceBytesRead = 0;

            std::cout << "Start downloading piece " << *pieceIndex << std::endl;
        }

        const auto byteToRequest =
            (pieceSize - pieceBytesRead < maxBlockSize) ? pieceSize - pieceBytesRead : maxBlockSize;

        const auto requestMessage =
            Message{MessageId::Request, libs::bytes::BytesConverter::intToBytes(*pieceIndex) +
                                            libs::bytes::BytesConverter::intToBytes(pieceBytesRead) +
                                            libs::bytes::BytesConverter::intToBytes(byteToRequest)};

        const auto serializedRequestMessage = MessageSerializer().serialize(requestMessage);

        boost::asio::async_write(
            socket, boost::asio::buffer(serializedRequestMessage),
            [this, byteToRequest](boost::system::error_code error, std::size_t bytes)
            {
                if (error)
                {
                    returnPieceToQueue();

                    std::cerr << error.message() << std::endl;

                    return;
                }

                std::cout << "Sent request message to " << socket.remote_endpoint()
                          << fmt::format(
                                 " to download piece {}, beginning {}, bytes to read {}.\nBytes transferred: {}",
                                 *pieceIndex, pieceBytesRead, byteToRequest, bytes)
                          << std::endl;
            });

        break;
    }
    case MessageId::Have:
    {
        const auto newPieceIndex = libs::bytes::BytesConverter::bytesToInt(message.payload);

        bitfield->setBit(newPieceIndex);

        break;
    }
    default:
        break;
    }

    boost::asio::async_read(socket, response, boost::asio::transfer_exactly(4),
                            [this](boost::system::error_code error, std::size_t bytes)
                            { onReadMessageLength(error, bytes); });
}

void PeerToPeerSessionImpl::returnPieceToQueue()
{
    if (pieceIndex)
    {
        piecesQueue.push(pieceIndex.value());
        pieceIndex = std::nullopt;
    }
}
}
