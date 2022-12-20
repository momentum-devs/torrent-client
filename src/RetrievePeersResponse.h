#pragma once

#include <vector>

#include "collection/StlOperators.h"
#include "PeerEndpoint.h"

struct RetrievePeersResponse
{
    long long interval;
    std::vector<PeerEndpoint> peersEndpoints;
};

inline bool operator==(const RetrievePeersResponse& lhs, const RetrievePeersResponse& rhs)
{
    return (lhs.interval == rhs.interval) and (lhs.peersEndpoints == rhs.peersEndpoints);
}

inline std::ostream& operator<<(std::ostream& os, const RetrievePeersResponse& retrievePeersResponse)
{
    return os << "interval: " << retrievePeersResponse.interval
              << " peersEndpoints: " << retrievePeersResponse.peersEndpoints;
}
