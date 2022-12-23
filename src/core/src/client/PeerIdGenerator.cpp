#include "PeerIdGenerator.h"

#include <random>

std::string PeerIdGenerator::generate()
{
    std::string peerId = "-UT2021-";

    std::random_device rd;

    std::mt19937 gen(rd());

    std::uniform_int_distribution<> distrib(1, 9);

    for (int i = 0; i < 12; i++)
    {
        peerId += std::to_string(distrib(gen));
    }

    return peerId;
}
