#include "PeersRetrieverImpl.h"

#include "encoder/HexEncoder.h"

namespace core
{
PeersRetrieverImpl::PeersRetrieverImpl(std::unique_ptr<common::httpClient::HttpClient> httpClientInit,
                                       std::unique_ptr<AnnounceResponseDeserializer> responseDeserializerInit)
    : httpClient{std::move(httpClientInit)}, responseDeserializer{std::move(responseDeserializerInit)}
{
}

RetrievePeersResponse PeersRetrieverImpl::retrievePeers(const RetrievePeersPayload& payload)
{
    const auto queryParameters =
        std::map<std::string, std::string>{{"info_hash", common::encoder::HexEncoder::decode(payload.infoHash)},
                                           {"peer_id", payload.peerId},
                                           {"port", payload.port},
                                           {"uploaded", payload.uploaded},
                                           {"downloaded", payload.downloaded},
                                           {"left", payload.left},
                                           {"compact", payload.compact}};

    const auto response = httpClient->get({payload.announceUrl, std::nullopt, queryParameters});

    auto deserializedResponse = responseDeserializer->deserialize(response.data);

    return deserializedResponse;
}
}
