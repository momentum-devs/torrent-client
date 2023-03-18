#pragma once

#include <ostream>
#include <string>

namespace core
{
struct PeerEndpoint
{
    std::string address;
    unsigned port;
};

inline uint64_t endpointToUint64(const PeerEndpoint& peerEndpoint)
{
    std::stringstream ss{peerEndpoint.address};
    uint64_t result = 0;
    unsigned address;
    while (ss >> address)
    {
        result <<= 8;
        result += address;
    }
    result <<= 16;
    result += peerEndpoint.port;
    return result;
}

inline bool operator==(const PeerEndpoint& lhs, const PeerEndpoint& rhs)
{
    return (lhs.address == rhs.address) and (lhs.port == rhs.port);
}

inline bool operator<(const PeerEndpoint& lhs, const PeerEndpoint& rhs)
{
    return endpointToUint64(lhs) < endpointToUint64(rhs);
}

inline std::ostream& operator<<(std::ostream& os, const PeerEndpoint& endpoint)
{
    return os << "address: " << endpoint.address << " port: " << endpoint.port;
}
}
