#include "PerlinNoise.h"

#include "dviglo/math/vector2.h"
#include <dviglo/math/math_defs.h>

namespace
{
float interpolate(float a0, float a1, float w)
{
//    return (a1 - a0) * w + a0;
//    return (a1 - a0) * (3.0 - w * 2.0) * w * w + a0;
    return (a1 - a0) * ((w * (w * 6.0 - 15.0) + 10.0) * w * w * w) + a0;
}

dviglo::Vector2 randomGradient(int ix, int iy)
{
    const unsigned w = 8 * sizeof(unsigned);
    const unsigned s = w / 2;
    unsigned a = ix, b = iy;
    a *= 3284157443;
    b ^= a << s | a >> w - s;
    b *= 1911520717;
    a ^= b << s | b >> w - s;
    a *= 2048419325;
    float random = a * (180. / ~(~0u >> 1));
    dviglo::Vector2 v;
    v.x = dviglo::Cos(random);
    v.y = dviglo::Sin(random);
    return v;
}

float dotGridGradient(int ix, int iy, float x, float y)
{
    dviglo::Vector2 gradient = randomGradient(ix, iy);
    float dx = x - (float)ix;
    float dy = y - (float)iy;
    return (dx * gradient.x + dy * gradient.y);
}

float perlin(float x, float y)
{
    int x0 = (int)dviglo::Floor(x);
    int x1 = x0 + 1;
    int y0 = (int)dviglo::Floor(y);
    int y1 = y0 + 1;

    float sx = x - (float)x0;
    float sy = y - (float)y0;

    float n0, n1, ix0, ix1, value;

    n0 = dotGridGradient(x0, y0, x, y);
    n1 = dotGridGradient(x1, y0, x, y);
    ix0 = interpolate(n0, n1, sx);

    n0 = dotGridGradient(x0, y1, x, y);
    n1 = dotGridGradient(x1, y1, x, y);
    ix1 = interpolate(n0, n1, sx);

    value = interpolate(ix0, ix1, sy);
    return value;
}

} // namespace

float PerlinNoise::getNoise(dviglo::Vector2 pos)
{
    return perlin(pos.x, pos.y);
}
