#include "PeerToPeerSessionImpl.h"

#include <iostream>

#include "fmt/format.h"

#include "bytes/Bitfield.h"
#include "bytes/BytesConverter.h"
#include "HandshakeMessageSerializer.h"
#include "HashValidator.h"
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
                                             const std::shared_ptr<TorrentFileInfo> torrentFileInfoInit)
    : socket(ioContext),
      piecesQueue{piecesQueueInit},
      peerEndpoint{peerEndpointInit},
      peerId{peerIdInit},
      isChoked{true},
      hasErrorOccurred{false},
      pieceIndex{std::nullopt},
      torrentFileInfo{torrentFileInfoInit},
      pieceBytesRead{0},
      maxBlockSize{16384},
      bitfield{std::nullopt},
      pieceData{}
{
}

void PeerToPeerSessionImpl::startSession()
{
    const auto address = boost::asio::ip::make_address(peerEndpoint.address);

    endpoint = boost::asio::ip::tcp::endpoint(address, peerEndpoint.port);

    socket.async_connect(
        endpoint,
        [this](boost::system::error_code error)
        {
            if (error)
            {
                std::cerr << "PeerConnectionError: " << error.message() << std::endl;

                hasErrorOccurred = true;

                return;
            }

            const auto handshakeMessage = HandshakeMessage{"BitTorrent protocol", torrentFileInfo->infoHash, peerId};

            sendHandshake(handshakeMessage);
        });
}

void PeerToPeerSessionImpl::sendHandshake(const HandshakeMessage& handshakeMessage)
{
    const auto serializedHandshakeMessage = HandshakeMessageSerializer().serialize(handshakeMessage);

    asyncWrite(boost::asio::buffer(serializedHandshakeMessage),
               [this](boost::system::error_code error, std::size_t)
               {
                   if (error)
                   {
                       std::cerr << error.message() << std::endl;

                       hasErrorOccurred = true;

                       return;
                   }

                   const auto numberOfBytesInHandshake = 68;

                   asyncRead(numberOfBytesInHandshake,
                             [this](boost::system::error_code error, std::size_t) { onReadHandshake(error); });
               });
}

void PeerToPeerSessionImpl::onReadHandshake(boost::system::error_code error)
{
    if (error)
    {
        std::cerr << endpoint << " : " << error.message() << std::endl;

        hasErrorOccurred = true;

        return;
    }

    const std::string data{std::istreambuf_iterator<char>(&response), std::istreambuf_iterator<char>()};

    const auto receivedInfoHash = data.substr(28, 20);

    if (not HashValidator::compareHashes(torrentFileInfo->infoHash, receivedInfoHash))
    {
        std::cerr << fmt::format("Receive handshake with different info hash, should be {}, get {}.",
                                 torrentFileInfo->infoHash, receivedInfoHash)
                  << std::endl;

        hasErrorOccurred = true;

        return;
    }

    std::cout << "Read handshake from " << endpoint << std::endl;

    asyncRead(4, [this](boost::system::error_code error, std::size_t bytes) { onReadMessageLength(error, bytes); });
}

void PeerToPeerSessionImpl::onReadMessageLength(boost::system::error_code error, std::size_t)
{
    if (error)
    {
        returnPieceToQueue();

        std::cerr << endpoint << " : " << error.message() << std::endl;

        hasErrorOccurred = true;

        return;
    }

    if (response.size() == 0) // Socket KeepAlive
    {
        asyncRead(4, [this](boost::system::error_code error, std::size_t bytes) { onReadMessageLength(error, bytes); });

        return;
    }

    if (response.size() != 4)
    {
        returnPieceToQueue();

        std::cerr << fmt::format("Message length {} not equal 4.", response.size()) << std::endl;

        hasErrorOccurred = true;

        return;
    }

    const std::basic_string<unsigned char> bytes{std::istreambuf_iterator<char>(&response),
                                                 std::istreambuf_iterator<char>()};

    const auto bytesToRead = libs::bytes::BytesConverter::bytesToInt(bytes);

    if (bytesToRead == 0)
    {
        asyncRead(4, [this](boost::system::error_code error, std::size_t bytes) { onReadMessageLength(error, bytes); });

        return;
    }

    asyncRead(bytesToRead, [this, bytesToRead](boost::system::error_code error, std::size_t bytes)
              { onReadMessage(error, bytes, bytesToRead); });
}

void PeerToPeerSessionImpl::onReadMessage(boost::system::error_code error, std::size_t, std::size_t bytesToRead)
{
    if (error)
    {
        returnPieceToQueue();

        std::cerr << endpoint << ", " << *pieceIndex << " : " << error.message() << std::endl;

        hasErrorOccurred = true;

        return;
    }

    if (response.size() != bytesToRead)
    {
        returnPieceToQueue();

        return;
    }

    const std::basic_string<unsigned char> data{std::istreambuf_iterator<char>(&response),
                                                std::istreambuf_iterator<char>()};

    const auto message = MessageSerializer().deserialize(data);

    switch (message.id)
    {
    case MessageId::Bitfield:
    {
        const auto bitfieldData = data.substr(1);

        bitfield.emplace(libs::bytes::Bitfield{bitfieldData});

        const auto interestedMessage = Message{MessageId::Interested, std::basic_string<unsigned char>{}};

        const auto serializedInterestedMessage = MessageSerializer().serialize(interestedMessage);

        asyncWrite(boost::asio::buffer(serializedInterestedMessage),
                   [this](boost::system::error_code error, std::size_t)
                   {
                       if (error)
                       {
                           returnPieceToQueue();

                           std::cerr << error.message() << std::endl;

                           hasErrorOccurred = true;

                           return;
                       }
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

        for (; piecesIndexIter < numberOfPiecesIndexes; ++piecesIndexIter)
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

        if (piecesIndexIter == numberOfPiecesIndexes)
        {
            std::cout << "No piece in queue available - close connection" << std::endl;

            return;
        }

        const auto requestMessage =
            Message{MessageId::Request, libs::bytes::BytesConverter::intToBytes(*pieceIndex) +
                                            libs::bytes::BytesConverter::intToBytes(pieceBytesRead) +
                                            libs::bytes::BytesConverter::intToBytes(maxBlockSize)};

        const auto serializedRequestMessage = MessageSerializer().serialize(requestMessage);

        asyncWrite(boost::asio::buffer(serializedRequestMessage),
                   [this](boost::system::error_code error, std::size_t)
                   {
                       if (error)
                       {
                           returnPieceToQueue();

                           std::cerr << error.message() << std::endl;

                           hasErrorOccurred = true;

                           return;
                       }
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

        pieceData += message.payload.substr(8, message.payload.size() - 8);

        if (pieceBytesRead >= torrentFileInfo->pieceLength)
        {

            std::cout << "Piece " << blockPieceIndex << " downloaded from " << endpoint << std::endl;

            auto numberOfPiecesIndexes = static_cast<int>(piecesQueue.size());

            auto piecesIndexIter = 0;

            if (HashValidator::compareHashWithData(torrentFileInfo->piecesHashes[blockPieceIndex], pieceData))
            {
                // TODO: save piece to file
            }
            else
            {
                returnPieceToQueue();

                std::cerr << "Piece " << blockPieceIndex << " hash not valid, push this index to queue again!"
                          << std::endl;
            }

            pieceData.clear();

            for (; piecesIndexIter < numberOfPiecesIndexes; ++piecesIndexIter)
            {
                pieceIndex = piecesQueue.pop();

                if (!pieceIndex)
                {
                    std::cout << "No piece in queue - close connection" << std::endl;

                    hasErrorOccurred = true;

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

            if (piecesIndexIter == numberOfPiecesIndexes)
            {
                std::cout << "No piece in queue available - close connection" << std::endl;

                hasErrorOccurred = true;

                return;
            }

            pieceBytesRead = 0;
        }

        const auto byteToRequest = (torrentFileInfo->pieceLength - pieceBytesRead < maxBlockSize) ?
                                       torrentFileInfo->pieceLength - pieceBytesRead :
                                       maxBlockSize;

        const auto requestMessage =
            Message{MessageId::Request, libs::bytes::BytesConverter::intToBytes(*pieceIndex) +
                                            libs::bytes::BytesConverter::intToBytes(pieceBytesRead) +
                                            libs::bytes::BytesConverter::intToBytes(byteToRequest)};

        const auto serializedRequestMessage = MessageSerializer().serialize(requestMessage);

        asyncWrite(boost::asio::buffer(serializedRequestMessage),
                   [this](boost::system::error_code error, std::size_t)
                   {
                       if (error)
                       {
                           returnPieceToQueue();

                           std::cerr << error.message() << std::endl;

                           hasErrorOccurred = true;

                           return;
                       }
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

    asyncRead(4, [this](boost::system::error_code error, std::size_t bytes) { onReadMessageLength(error, bytes); });
}

void PeerToPeerSessionImpl::returnPieceToQueue()
{
    if (pieceIndex)
    {
        piecesQueue.push(pieceIndex.value());
        pieceIndex = std::nullopt;
        pieceData.clear();
    }
}

void PeerToPeerSessionImpl::asyncRead(std::size_t bytesToRead,
                                      std::function<void(boost::system::error_code, std::size_t)> readHandler)
{
    if (not hasErrorOccurred)
    {
        boost::asio::async_read(socket, response, boost::asio::transfer_exactly(bytesToRead), readHandler);
    }
}

void PeerToPeerSessionImpl::asyncWrite(boost::asio::const_buffer writeBuffer,
                                       std::function<void(boost::system::error_code, std::size_t)> writeHandler)
{
    if (not hasErrorOccurred)
    {
        boost::asio::async_write(socket, writeBuffer, writeHandler);
    }
}
}
