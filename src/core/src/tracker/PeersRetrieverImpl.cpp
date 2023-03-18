#include "PeersRetrieverImpl.h"

#include "fmt/core.h"

#include "encoder/HexEncoder.h"

namespace core
{
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
        fmt::print("Processing {} announce\n", announceUrl);
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

        auto deserializedResponse = responseDeserializer->deserialize(response.data);

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
