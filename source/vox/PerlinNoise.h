#pragma once

#include <dviglo/math/vector2.h>

class PerlinNoise
{
public:
    static float getNoise(dviglo::Vector2 pos);
};
