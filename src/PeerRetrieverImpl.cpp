#include "PeerRetrieverImpl.h"

#include "GetProjectPath.h"
#include "HexEncoder.h"

PeerRetrieverImpl::PeerRetrieverImpl(std::unique_ptr<HttpClient> httpClientInit,
                                     std::unique_ptr<AnnounceResponseDeserializer> responseDeserializerInit)
    : httpClient{std::move(httpClientInit)}, responseDeserializer{std::move(responseDeserializerInit)}
{
}

RetrievePeersResponse PeerRetrieverImpl::retrievePeers(const RetrievePeersPayload& payload)
{
    const auto queryParameters = std::map<std::string, std::string>{{"info_hash", HexEncoder::decode(payload.infoHash)},
                                                                    {"peer_id", payload.peerId},
                                                                    {"port", payload.port},
                                                                    {"uploaded", payload.uploaded},
                                                                    {"downloaded", payload.downloaded},
                                                                    {"left", payload.left},
                                                                    {"compact", payload.compact}};

    auto response = httpClient->get({payload.announceUrl, std::nullopt, queryParameters});

    auto deserializedResponse = responseDeserializer->deserialize(response.data);

    return deserializedResponse;
}
