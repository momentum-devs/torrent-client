#include "PeersRetrieverImpl.h"

#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <iostream>
#include <random>
#include <regex>

#include "fmt/format.h"

#include "bytes/BytesConverter.h"
#include "fmt/core.h"

#include "encoder/HexEncoder.h"

namespace core
{
namespace
{

int getRandomNumber()
{
    std::random_device randomDevice;
    std::mt19937 pseudoRandomGenerator(randomDevice());

    std::uniform_int_distribution<int> distribution(0, 10000);

    const int randomNumber = distribution(pseudoRandomGenerator);

    return randomNumber;
}

std::basic_string<unsigned char> buildUdpConnectionRequest()
{
    std::basic_string<unsigned char> connectionRequest;

    // connection id
    connectionRequest += libs::bytes::BytesConverter::int64ToBytes(0x41727101980);

    // action
    connectionRequest += libs::bytes::BytesConverter::int32ToBytes(0);

    // transaction id
    const auto x = getRandomNumber();

    std::cout << "transaction id: " << x << std::endl;
    connectionRequest += libs::bytes::BytesConverter::int32ToBytes(x);

    return connectionRequest;
}

std::basic_string<unsigned char> buildUdpAnnounceRequest(const RetrievePeersPayload& payload,
                                                         const std::basic_string<unsigned char>& connectionId)
{
    std::basic_string<unsigned char> announceRequest;

    announceRequest += connectionId;
    const auto action = libs::bytes::BytesConverter::int32ToBytes(1);
    announceRequest += action;
    announceRequest += libs::bytes::BytesConverter::int32ToBytes(getRandomNumber());
    announceRequest +=
        reinterpret_cast<const unsigned char*>(libs::encoder::HexEncoder::decode(payload.infoHash).c_str());
    announceRequest += reinterpret_cast<const unsigned char*>(payload.peerId.c_str());
    announceRequest += libs::bytes::BytesConverter::int64ToBytes(std::stol(payload.downloaded));
    announceRequest += libs::bytes::BytesConverter::int64ToBytes(std::stol(payload.left));
    announceRequest += libs::bytes::BytesConverter::int64ToBytes(std::stol(payload.uploaded));
    announceRequest += libs::bytes::BytesConverter::int32ToBytes(0);
    announceRequest += libs::bytes::BytesConverter::int32ToBytes(0);
    announceRequest += libs::bytes::BytesConverter::int32ToBytes(getRandomNumber());
    announceRequest += libs::bytes::BytesConverter::int32ToBytes(-1);
    announceRequest += libs::bytes::BytesConverter::int16ToBytes(std::stoi(payload.port));

    return announceRequest;
}

}
PeersRetrieverImpl::PeersRetrieverImpl(std::unique_ptr<libs::httpClient::HttpClient> httpClientInit,
                                       std::unique_ptr<AnnounceResponseDeserializer> responseDeserializerInit)
    : httpClient{std::move(httpClientInit)}, responseDeserializer{std::move(responseDeserializerInit)}
{
}

RetrievePeersResponse PeersRetrieverImpl::retrievePeers(const RetrievePeersPayload& payload)
{
    std::set<PeerEndpoint> peerEndpoints;

    for (const auto& announceUrl : payload.announceList)
    {
        if (announceUrl.find("udp://") != std::string::npos)
        {
            std::regex hostExpression("//(.+):");

            std::smatch hostNatch;

            std::string host;

            if (std::regex_search(announceUrl, hostNatch, hostExpression))
            {
                host = hostNatch[1].str();
            }
            else
            {
                throw std::runtime_error{fmt::format("Host not found in announce url: {}", announceUrl)};
            }

            std::regex portExpression(":(\\d+)/");

            std::smatch portNatch;

            std::string portNumber;

            if (std::regex_search(announceUrl, portNatch, portExpression))
            {
                portNumber = portNatch[1].str();
            }
            else
            {
                throw std::runtime_error{fmt::format("Port number not found in announce url: {}", announceUrl)};
            }

            boost::asio::io_context context;

            boost::asio::ip::udp::resolver resolver(context);

            boost::asio::ip::udp::resolver::query query(host, portNumber);

            const auto iter = resolver.resolve(query);

            auto remoteEndpoint = iter->endpoint();

            boost::asio::ip::udp::socket socket(context);
            socket.open(boost::asio::ip::udp::v4());

            socket.send_to(boost::asio::buffer(buildUdpConnectionRequest()), remoteEndpoint);

            char connectResponseData[16];

            boost::system::error_code errorCode;

            socket.receive(boost::asio::buffer(&connectResponseData, 16), 0, errorCode);

            const auto receivedConnectionId =
                std::basic_string<unsigned char>{connectResponseData + 8, connectResponseData + 16};

            socket.send_to(boost::asio::buffer(buildUdpAnnounceRequest(payload, receivedConnectionId)), remoteEndpoint);

            char announceResponseData[1024];

            boost::asio::streambuf response;

            const auto announceResponseBytesReceived = socket.receive(boost::asio::buffer(&announceResponseData, 1024));

            auto deserializedPeersEndpoints = responseDeserializer->deserializePeersEndpoints(
                std::string{announceResponseData + 20, announceResponseData + announceResponseBytesReceived});

            peerEndpoints.insert(deserializedPeersEndpoints.begin(), deserializedPeersEndpoints.end());
            continue ;
        }

        const auto queryParameters =
            std::map<std::string, std::string>{{"info_hash", libs::encoder::HexEncoder::decode(payload.infoHash)},
                                               {"peer_id", payload.peerId},
                                               {"port", payload.port},
                                               {"uploaded", payload.uploaded},
                                               {"downloaded", payload.downloaded},
                                               {"left", payload.left},
                                               {"compact", payload.compact}};

        const auto response = httpClient->get({announceUrl, std::nullopt, queryParameters});

        if (response.statusCode != 200)
        {
            fmt::print("Tracker {} response with code {} \n", announceUrl, response.statusCode);
            continue;
        }

        auto deserializedResponse = responseDeserializer->deserializeBencode(response.data);

        fmt::print("From {} get {} peers \n", announceUrl, deserializedResponse.peersEndpoints.size());

        peerEndpoints.insert(deserializedResponse.peersEndpoints.begin(), deserializedResponse.peersEndpoints.end());
    }


    if (peerEndpoints.empty())
    {
        throw std::runtime_error{"No peer available"};
    }

    RetrievePeersResponse response{0, std::vector<PeerEndpoint>(peerEndpoints.begin(), peerEndpoints.end())};

    return response;
}

}
