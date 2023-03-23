#include "PeersRetrieverImpl.h"

#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <chrono>
#include <regex>

#include "fmt/core.h"
#include "fmt/format.h"

#include "bytes/BytesConverter.h"
#include "encoder/HexEncoder.h"
#include "FormatUdpAnnounceRequestPayload.h"
#include "random/RandomGenerator.h"

namespace core
{
namespace
{

std::basic_string<unsigned char> formatUdpConnectionRequest();
std::basic_string<unsigned char> formatUdpAnnounceRequest(const FormatUdpAnnounceRequestPayload&);
std::string findHostInUrl(const std::string& url);
std::string findPortNumberInUrl(const std::string& url);
}

PeersRetrieverImpl::PeersRetrieverImpl(std::unique_ptr<libs::httpClient::HttpClient> httpClientInit,
                                       std::unique_ptr<AnnounceResponseDeserializer> responseDeserializerInit)
    : httpClient{std::move(httpClientInit)}, responseDeserializer{std::move(responseDeserializerInit)}
{
}

void PeersRetrieverImpl::retrievePeers(const RetrievePeersPayload& payload,
                                       std::function<void(const std::vector<PeerEndpoint>&)> processPeersHandler)
{
    if (payload.announceUrl.find("udp://") != std::string::npos)
    {
        const auto host = findHostInUrl(payload.announceUrl);

        const auto portNumber = findPortNumberInUrl(payload.announceUrl);

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
            return;
        }

        boost::asio::ip::udp::socket socket(context);

        socket.open(boost::asio::ip::udp::v4());

        if (remoteEndpoint.protocol() == boost::asio::ip::udp::v6())
        {
            return;
        }

        const auto connectionRequestMessage = formatUdpConnectionRequest();

        socket.send_to(boost::asio::buffer(connectionRequestMessage), remoteEndpoint);

        char connectResponseData[16];

        socket.receive(boost::asio::buffer(&connectResponseData, 16));

        const auto receivedConnectionId =
            std::basic_string<unsigned char>{connectResponseData + 8, connectResponseData + 16};

        const auto announceRequestMessage =
            formatUdpAnnounceRequest({payload.infoHash, payload.peerId, payload.port, payload.uploaded,
                                      payload.downloaded, payload.left, receivedConnectionId});

        socket.send_to(boost::asio::buffer(announceRequestMessage), remoteEndpoint);

        char announceResponseData[1024];

        boost::asio::streambuf response;

        const auto announceResponseBytesReceived = socket.receive(boost::asio::buffer(&announceResponseData, 1024));

        const auto deserializedPeersEndpoints = responseDeserializer->deserializePeersEndpoints(
            std::string{announceResponseData + 20, announceResponseData + announceResponseBytesReceived});

        processPeersHandler(deserializedPeersEndpoints);
    }
    else
    {
        const auto queryParameters =
            std::map<std::string, std::string>{{"info_hash", libs::encoder::HexEncoder::decode(payload.infoHash)},
                                               {"peer_id", payload.peerId},
                                               {"port", payload.port},
                                               {"uploaded", payload.uploaded},
                                               {"downloaded", payload.downloaded},
                                               {"left", payload.left},
                                               {"compact", payload.compact}};

        const auto response = httpClient->get({payload.announceUrl, std::nullopt, queryParameters});

        if (response.statusCode != 200)
        {
            return;
        }

        const auto deserializedResponse = responseDeserializer->deserializeBencode(response.data);

        processPeersHandler(deserializedResponse.peersEndpoints);
    }
}

namespace
{
std::basic_string<unsigned char> formatUdpConnectionRequest()
{

    const auto connectionId = libs::bytes::BytesConverter::int64ToBytes(0x41727101980);

    const auto action = libs::bytes::BytesConverter::int32ToBytes(0);

    const auto transactionId =
        libs::bytes::BytesConverter::int32ToBytes(libs::random::RandomGenerator::generateNumber(0, 100000));

    auto connectionRequest = connectionId + action + transactionId;

    return connectionRequest;
}

std::basic_string<unsigned char> formatUdpAnnounceRequest(const FormatUdpAnnounceRequestPayload& payload)
{
    std::basic_string<unsigned char> announceRequest;

    announceRequest += payload.connectionId;
    const auto action = libs::bytes::BytesConverter::int32ToBytes(1);
    announceRequest += action;
    announceRequest +=
        libs::bytes::BytesConverter::int32ToBytes(libs::random::RandomGenerator::generateNumber(0, 100000));
    announceRequest +=
        reinterpret_cast<const unsigned char*>(libs::encoder::HexEncoder::decode(payload.infoHash).c_str());
    announceRequest += reinterpret_cast<const unsigned char*>(payload.peerId.c_str());
    announceRequest += libs::bytes::BytesConverter::int64ToBytes(std::stol(payload.downloaded));
    announceRequest += libs::bytes::BytesConverter::int64ToBytes(std::stol(payload.left));
    announceRequest += libs::bytes::BytesConverter::int64ToBytes(std::stol(payload.uploaded));
    announceRequest += libs::bytes::BytesConverter::int32ToBytes(0);
    announceRequest += libs::bytes::BytesConverter::int32ToBytes(0);
    announceRequest +=
        libs::bytes::BytesConverter::int32ToBytes(libs::random::RandomGenerator::generateNumber(0, 100000));
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
    std::regex portExpression(":(\\d+)");

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

}
