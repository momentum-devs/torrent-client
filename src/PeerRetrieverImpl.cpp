#include "PeerRetrieverImpl.h"

#include <cpr/cpr.h>
#include <random>

std::string hexDecode(const std::string& value)
{
    int hashLength = value.length();
    std::string decodedHexString;
    for (int i = 0; i < hashLength; i += 2)
    {
        std::string byte = value.substr(i, 2);
        char c = (char) (int) strtol(byte.c_str(), nullptr, 16);
        decodedHexString.push_back(c);
    }
    return decodedHexString;
}

std::string PeerRetrieverImpl::retrievePeers(const std::string& announceUrl)
{
//    std::string peerId = "-UT2021-";
//    std::random_device rd;
//    std::mt19937 gen(rd());
//    std::uniform_int_distribution<> distrib(1, 9);
//    for (int i = 0; i < 12; i++)
//    {
//        peerId += std::to_string(distrib(gen));
//    }
//
//    cpr::Response res = cpr::Get(cpr::Url{announceUrl}, cpr::Parameters {
//                                                            { "info_hash", std::string(hexDecode(infoHash)) },
//                                                            { "peer_id", std::string(peerId) },
//                                                            { "port", std::to_string(port) },
//                                                            { "uploaded", std::to_string(0) },
//                                                            { "downloaded", std::to_string(bytesDownloaded) },
//                                                            { "left", std::to_string(fileSize - bytesDownloaded) },
//                                                            { "compact", std::to_string(1) }
//                                                        }, cpr::Timeout{ 15000 }
//    );
    return "";
}
