#include "PeerToPeerSessionImpl.h"

#include <boost/beast/core/tcp_stream.hpp>
#include <iostream>
#include <optional>

#include "fmt/format.h"

#include "bytes/Bitfield.h"
#include "bytes/BytesConverter.h"
#include "HandshakeMessageSerializer.h"
#include "HashValidator.h"
#include "MessageSerializer.h"

namespace core
{
namespace
{
const int timeoutInSeconds{5};
const int maxBlockSize{16384};
}

PeerToPeerSessionImpl::PeerToPeerSessionImpl(boost::asio::io_context& ioContext,
                                             libs::collection::ThreadSafeQueue<int>& piecesQueueInit,
                                             const PeerEndpoint& peerEndpointInit, const std::string& peerIdInit,
                                             const std::shared_ptr<TorrentFileInfo> torrentFileInfoInit,
                                             const std::shared_ptr<PieceRepository> pieceRepositoryInit)
    : socket(ioContext),
      piecesQueue{piecesQueueInit},
      peerEndpoint{peerEndpointInit},
      peerId{peerIdInit},
      isChoked{true},
      hasErrorOccurred{false},
      pieceIndex{std::nullopt},
      torrentFileInfo{torrentFileInfoInit},
      pieceRepository{pieceRepositoryInit},
      pieceBytesRead{0},
      bitfield{std::nullopt},
      pieceData{},
      deadline(ioContext)
{
}

void PeerToPeerSessionImpl::startSession()
{
    const auto address = boost::asio::ip::make_address(peerEndpoint.address);

    endpoint = boost::asio::ip::tcp::endpoint(address, peerEndpoint.port);

    deadline.expires_from_now(boost::posix_time::seconds(timeoutInSeconds));

    deadline.async_wait([this](boost::system::error_code) { checkDeadline(); });

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
        handleBitfieldMessage(message);

        break;
    }
    case MessageId::Unchoke:
    {
        handleUnchokeMessage();

        break;
    }
    case MessageId::Choke:
    {
        handleChokeMessage();

        break;
    }
    case MessageId::Piece:
    {
        handlePieceMessage(message);

        break;
    }
    case MessageId::Have:
    {
        handleHaveMessage(message);

        break;
    }
    default:
        break;
    }

    asyncRead(4, [this](boost::system::error_code error, std::size_t bytes) { onReadMessageLength(error, bytes); });
}

void PeerToPeerSessionImpl::handleBitfieldMessage(const Message& bitfieldMessage)
{
    bitfield.emplace(libs::bytes::Bitfield{bitfieldMessage.payload});

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
}

void PeerToPeerSessionImpl::handleUnchokeMessage()
{
    if (!isChoked)
    {
        return;
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
        Message{MessageId::Request, libs::bytes::BytesConverter::int32ToBytes(*pieceIndex) +
                                        libs::bytes::BytesConverter::int32ToBytes(pieceBytesRead) +
                                        libs::bytes::BytesConverter::int32ToBytes(maxBlockSize)};

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
}

void PeerToPeerSessionImpl::handleChokeMessage()
{
    isChoked = true;

    returnPieceToQueue();
}

void PeerToPeerSessionImpl::handlePieceMessage(const Message& pieceMessage)
{
    pieceBytesRead += maxBlockSize;

    const auto blockPieceIndex = libs::bytes::BytesConverter::bytesToInt(pieceMessage.payload.substr(0, 4));

    pieceData += pieceMessage.payload.substr(8, pieceMessage.payload.size() - 8);

    if (pieceBytesRead >= torrentFileInfo->pieceLength)
    {
        std::cout << "Piece " << blockPieceIndex << " downloaded from " << endpoint << std::endl;

        auto numberOfPiecesIndexes = static_cast<int>(piecesQueue.size());

        auto piecesIndexIter = 0;

        if (HashValidator::compareHashWithData(torrentFileInfo->piecesHashes[blockPieceIndex], pieceData))
        {
            pieceRepository->save(blockPieceIndex, pieceData);
        }
        else
        {
            returnPieceToQueue();

            std::cerr << "Piece " << blockPieceIndex << " hash not valid, push this index to queue again!" << std::endl;
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
        Message{MessageId::Request, libs::bytes::BytesConverter::int32ToBytes(*pieceIndex) +
                                        libs::bytes::BytesConverter::int32ToBytes(pieceBytesRead) +
                                        libs::bytes::BytesConverter::int32ToBytes(byteToRequest)};

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
}

void PeerToPeerSessionImpl::handleHaveMessage(const Message& haveMessage)
{
    const auto newPieceIndex = libs::bytes::BytesConverter::bytesToInt(haveMessage.payload);

    bitfield->setBit(newPieceIndex);
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
        deadline.expires_from_now(boost::posix_time::seconds(timeoutInSeconds));

        boost::asio::async_read(socket, response, boost::asio::transfer_exactly(bytesToRead), readHandler);
    }
}

void PeerToPeerSessionImpl::asyncWrite(boost::asio::const_buffer writeBuffer,
                                       std::function<void(boost::system::error_code, std::size_t)> writeHandler)
{
    if (not hasErrorOccurred)
    {
        deadline.expires_from_now(boost::posix_time::seconds(timeoutInSeconds));

        boost::asio::async_write(socket, writeBuffer, writeHandler);
    }
}

void PeerToPeerSessionImpl::checkDeadline()
{
    if (hasErrorOccurred)
    {
        return;
    }

    if (deadline.expires_at() <= boost::asio::deadline_timer::traits_type::now())
    {
        socket.close();

        hasErrorOccurred = true;

        std::cerr << endpoint << ": timeout" << std::endl;

        returnPieceToQueue();
    }
    else
    {
        deadline.async_wait([this](boost::system::error_code) { checkDeadline(); });
    }
}

}
