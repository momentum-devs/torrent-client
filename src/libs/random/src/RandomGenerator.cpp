#include "RandomGenerator.h"

#include <random>

namespace libs::random
{
int RandomGenerator::generateNumber(int begin, int end)
{
    std::random_device randomDevice;
    std::mt19937 pseudoRandomGenerator(randomDevice());

    std::uniform_int_distribution<int> distribution(begin, end);

    const int randomNumber = distribution(pseudoRandomGenerator);

    return randomNumber;
}
}
