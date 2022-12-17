#include "PeerRetrieverImpl.h"

#include <cpr/cpr.h>

#include "HexEncoder.h"

std::string PeerRetrieverImpl::retrievePeers(const RetrievePeersPayload& payload)
{
    auto response = cpr::Get(cpr::Url{payload.announceUrl},
                             cpr::Parameters{{"info_hash", HexEncoder::decode(payload.infoHash)},
                                             {"peer_id", payload.peerId},
                                             {"port", payload.port},
                                             {"uploaded", payload.uploaded},
                                             {"downloaded", payload.downloaded},
                                             {"left", payload.left},
                                             {"compact", payload.compact}},
                             cpr::Timeout{15000});
    return response.text;
}
