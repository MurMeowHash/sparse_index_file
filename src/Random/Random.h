#pragma once

#include <random>
#include "../utils/utils.h"

class Random {
private:
    static std::random_device rd;
    static std::mt19937 mt;
public:
    NODISCARD static uint next(uint min, uint max);
    NODISCARD static std::mt19937 getRandomDevice();
};