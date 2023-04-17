#include "PeerToPeerSessionImpl.h"

#include <boost/beast/core/tcp_stream.hpp>
#include <optional>

#include "fmt/format.h"

#include "bytes/Bitfield.h"
#include "bytes/BytesConverter.h"
#include "HandshakeMessageSerializer.h"
#include "HashValidator.h"
#include "loguru.hpp"
#include "MessageSerializer.h"

namespace core
{
namespace
{
const int timeoutInSeconds{100};
const int maxBlockSize{16384};
}

PeerToPeerSessionImpl::PeerToPeerSessionImpl(boost::asio::io_context& ioContext,
                                             libs::collection::ThreadSafeQueue<int>& piecesQueueInit,
                                             const PeerEndpoint& peerEndpointInit, std::string peerIdInit,
                                             const std::shared_ptr<TorrentFileInfo>& torrentFileInfoInit,
                                             const std::shared_ptr<PieceRepository> pieceRepositoryInit,
                                             PeerToPeerSessionManager& managerInit)
    : socket(ioContext),
      piecesQueue{piecesQueueInit},
      peerEndpoint{peerEndpointInit},
      peerId{std::move(peerIdInit)},
      isChoked{true},
      hasErrorOccurred{false},
      pieceIndex{std::nullopt},
      torrentFileInfo{torrentFileInfoInit},
      pieceRepository{pieceRepositoryInit},
      pieceBytesRead{0},
      bitfield{std::nullopt},
      pieceData{},
      deadline(ioContext),
      sessionManager{managerInit}
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
                LOG_S(ERROR) << fmt::format("Cannot connect to {}:{}: ", endpoint.address().to_string(),
                                            endpoint.port())
                             << error.message();

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
                       LOG_S(ERROR) << "Error occurred while sending a handshake message to " << endpoint << ": "
                                    << error.message();

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
        hasErrorOccurred = true;

        return;
    }

    const std::string data{std::istreambuf_iterator<char>(&response), std::istreambuf_iterator<char>()};

    const auto receivedInfoHash = data.substr(28, 20);

    if (not HashValidator::compareHashes(torrentFileInfo->infoHash, receivedInfoHash))
    {
        LOG_S(ERROR) << "Received a handshake with different info hash from " << endpoint << ".";

        hasErrorOccurred = true;

        return;
    }

    LOG_S(INFO) << "Connected to " << endpoint << ".";

    asyncRead(4, [this](boost::system::error_code error, std::size_t bytes) { onReadMessageLength(error, bytes); });
}

void PeerToPeerSessionImpl::onReadMessageLength(boost::system::error_code error, std::size_t)
{
    if (error)
    {
        returnPieceToQueue();

        LOG_S(ERROR) << "Error occurred while reading message length from " << endpoint << ": " << error.message();

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

        LOG_S(ERROR) << "Message length received from " << endpoint << " is not equal to 4.";

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

        LOG_S(ERROR) << "Error occurred while reading piece " << *pieceIndex << " from " << endpoint << ": "
                     << error.message();

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

                       LOG_S(ERROR) << "Error occurred while sending interested message to " << endpoint << ": "
                                    << error.message();

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
            LOG_S(INFO) << "No pieces in queue, closing connection with " << endpoint << "...";

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
        LOG_S(INFO) << "No pieces in queue, closing connection with " << endpoint << "...";

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

                       LOG_S(ERROR) << "Error occurred while sending request message to " << endpoint << ": "
                                    << error.message();

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

    auto lastPiece = this->torrentFileInfo->piecesHashes.size() - 1;

    auto lastPieceSize = torrentFileInfo->length % torrentFileInfo->pieceLength;

    if (pieceBytesRead >= torrentFileInfo->pieceLength ||
        ((*pieceIndex == lastPiece) && (pieceBytesRead >= lastPieceSize)))
    {
        auto numberOfPiecesIndexes = static_cast<int>(piecesQueue.size());

        auto piecesIndexIter = 0;

        if (HashValidator::compareHashWithData(torrentFileInfo->piecesHashes[blockPieceIndex], pieceData))
        {
            pieceRepository->save(blockPieceIndex, pieceData);

            LOG_S(INFO) << "Piece " << blockPieceIndex << " downloaded from " << endpoint
                        << fmt::format(" ({}/{})", pieceRepository->getDownloadedPieces().size(),
                                       torrentFileInfo->piecesHashes.size());
        }
        else
        {
            LOG_S(ERROR) << "Piece  " << blockPieceIndex << " received from" << endpoint
                         << " has invalid hash, pushing piece index back to the queue...";

            returnPieceToQueue();
        }

        pieceData.clear();

        for (; piecesIndexIter < numberOfPiecesIndexes; ++piecesIndexIter)
        {
            pieceIndex = piecesQueue.pop();

            if (!pieceIndex)
            {
                LOG_S(INFO) << "No pieces in queue available, closing connection with peer " << endpoint << "...";

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
            LOG_S(INFO) << "No pieces in queue available, closing connection with peer " << endpoint << "...";

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

                       LOG_S(ERROR) << "Error occurred while sending request message to " << endpoint << ": "
                                    << error.message();

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

        return;
    }

    sessionManager.closeSession(peerEndpoint);
}

void PeerToPeerSessionImpl::asyncWrite(boost::asio::const_buffer writeBuffer,
                                       std::function<void(boost::system::error_code, std::size_t)> writeHandler)
{
    if (not hasErrorOccurred)
    {
        deadline.expires_from_now(boost::posix_time::seconds(timeoutInSeconds));

        boost::asio::async_write(socket, writeBuffer, writeHandler);

        return;
    }

    sessionManager.closeSession(peerEndpoint);
}

void PeerToPeerSessionImpl::checkDeadline()
{
    if (hasErrorOccurred)
    {
        sessionManager.closeSession(peerEndpoint);

        return;
    }

    if (deadline.expires_at() <= boost::asio::deadline_timer::traits_type::now())
    {
        socket.close();

        hasErrorOccurred = true;

        LOG_S(ERROR) << "Connection with " << endpoint << " ended with timeout.";

        returnPieceToQueue();
    }
    else
    {
        deadline.async_wait([this](boost::system::error_code) { checkDeadline(); });
    }
}

}
