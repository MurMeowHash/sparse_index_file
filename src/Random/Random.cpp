#include "Random.h"

std::random_device Random::rd;
std::mt19937 Random::mt{rd()};

uint Random::next(uint min, uint max) {
    std::uniform_int_distribution<uint> distribution{min, max};
    return distribution(mt);
}

std::mt19937 Random::getRandomDevice() {
    return mt;
}