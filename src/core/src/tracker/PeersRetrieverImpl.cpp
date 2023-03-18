#include "PeersRetrieverImpl.h"

#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <chrono>
#include <regex>

#include "fmt/core.h"
#include "fmt/format.h"

#include "bytes/BytesConverter.h"
#include "encoder/HexEncoder.h"
#include "random/RandomGenerator.h"

namespace core
{
namespace
{

std::basic_string<unsigned char> buildUdpConnectionRequest()
{

    const auto connectionId = libs::bytes::BytesConverter::int64ToBytes(0x41727101980);

    const auto action = libs::bytes::BytesConverter::int32ToBytes(0);

    const auto transactionId =
        libs::bytes::BytesConverter::int32ToBytes(libs::random::RandomGenerator::generateNumber(0, 100000));

    auto connectionRequest = connectionId + action + transactionId;

    return connectionRequest;
}

std::basic_string<unsigned char> buildUdpAnnounceRequest(const RetrievePeersPayload& payload,
                                                         const std::basic_string<unsigned char>& connectionId)
{
    std::basic_string<unsigned char> announceRequest;

    announceRequest += connectionId;
    const auto action = libs::bytes::BytesConverter::int32ToBytes(1);
    announceRequest += action;
    announceRequest += libs::bytes::BytesConverter::int32ToBytes(libs::random::RandomGenerator::generateNumber(0, 100000)));
    announceRequest +=
        reinterpret_cast<const unsigned char*>(libs::encoder::HexEncoder::decode(payload.infoHash).c_str());
    announceRequest += reinterpret_cast<const unsigned char*>(payload.peerId.c_str());
    announceRequest += libs::bytes::BytesConverter::int64ToBytes(std::stol(payload.downloaded));
    announceRequest += libs::bytes::BytesConverter::int64ToBytes(std::stol(payload.left));
    announceRequest += libs::bytes::BytesConverter::int64ToBytes(std::stol(payload.uploaded));
    announceRequest += libs::bytes::BytesConverter::int32ToBytes(0);
    announceRequest += libs::bytes::BytesConverter::int32ToBytes(0);
    announceRequest += libs::bytes::BytesConverter::int32ToBytes(libs::random::RandomGenerator::generateNumber(0, 100000)));
    announceRequest += libs::bytes::BytesConverter::int32ToBytes(-1);
    announceRequest += libs::bytes::BytesConverter::int16ToBytes(std::stoi(payload.port));

    return announceRequest;
}

std::string findHostInUrl(const std::string& url)
{
    std::regex hostExpression("//(.+):");

    std::smatch hostNatch;

    std::string host;

    if (std::regex_search(url, hostNatch, hostExpression))
    {
        host = hostNatch[1].str();
    }
    else
    {
        throw std::runtime_error{fmt::format("Host not found in url: {}", url)};
    }

    return host;
}

std::string findPortNumberInUrl(const std::string& url)
{
    std::regex portExpression(":(\\d+)/");

    std::smatch portMatch;

    std::string portNumber;

    if (std::regex_search(url, portMatch, portExpression))
    {
        portNumber = portMatch[1].str();
    }
    else
    {
        throw std::runtime_error{fmt::format("Port number not found in url: {}", url)};
    }

    return portNumber;
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
            const auto host = findHostInUrl(announceUrl);

            const auto portNumber = findPortNumberInUrl(announceUrl);

            boost::asio::io_context context;

            boost::asio::ip::udp::resolver resolver(context);

            boost::asio::ip::udp::resolver::query query(host, portNumber);

            boost::asio::ip::udp::endpoint remoteEndpoint;

            try
            {
                remoteEndpoint = resolver.resolve(query)->endpoint();
            }
            catch (const std::exception&)
            {
                continue;
            }

            boost::asio::thread_pool ioc;

            boost::asio::ip::udp::socket socket(ioc);

            socket.open(boost::asio::ip::udp::v4());

            if (remoteEndpoint.protocol() == boost::asio::ip::udp::v6())
            {
                continue;
            }

            socket.send_to(boost::asio::buffer(buildUdpConnectionRequest()), remoteEndpoint);

            char connectResponseData[16];

            auto future = socket.async_receive(boost::asio::buffer(&connectResponseData, 16), boost::asio::use_future);

            using namespace std::chrono_literals;

            const auto futureStatus = future.wait_for(50ms);

            if (futureStatus == std::future_status::timeout || futureStatus == std::future_status::deferred)
            {
                continue;
            }

            ioc.join();

            const auto receivedConnectionId =
                std::basic_string<unsigned char>{connectResponseData + 8, connectResponseData + 16};

            socket.send_to(boost::asio::buffer(buildUdpAnnounceRequest(payload, receivedConnectionId)), remoteEndpoint);

            char announceResponseData[1024];

            boost::asio::streambuf response;

            const auto announceResponseBytesReceived = socket.receive(boost::asio::buffer(&announceResponseData, 1024));

            auto deserializedPeersEndpoints = responseDeserializer->deserializePeersEndpoints(
                std::string{announceResponseData + 20, announceResponseData + announceResponseBytesReceived});

            peerEndpoints.insert(deserializedPeersEndpoints.begin(), deserializedPeersEndpoints.end());

            continue;
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
            continue;
        }

        auto deserializedResponse = responseDeserializer->deserializeBencode(response.data);

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
