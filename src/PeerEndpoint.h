#pragma once

#include <ostream>
#include <string>

struct PeerEndpoint
{
    std::string address;
    unsigned port;
};

inline bool operator==(const PeerEndpoint& lhs, const PeerEndpoint& rhs)
{
    return (lhs.address == rhs.address) and (lhs.port == rhs.port);
}

inline std::ostream& operator<<(std::ostream& os, const PeerEndpoint& endpoint)
{
    return os << "address: " << endpoint.address << " port: " << endpoint.port;
}
