// Copyright (c) 2008-2023 the Urho3D project
// Copyright (c) 2022-2023 the Dviglo project
// License: MIT

#include "vector2.h"

#include <cstdio>

#include "../common/debug_new.h"

namespace dviglo
{

const Vector2 Vector2::ZERO;
const Vector2 Vector2::LEFT(-1.0f, 0.0f);
const Vector2 Vector2::RIGHT(1.0f, 0.0f);
const Vector2 Vector2::UP(0.0f, 1.0f);
const Vector2 Vector2::DOWN(0.0f, -1.0f);
const Vector2 Vector2::ONE(1.0f, 1.0f);

const IntVector2 IntVector2::ZERO;
const IntVector2 IntVector2::LEFT(-1, 0);
const IntVector2 IntVector2::RIGHT(1, 0);
const IntVector2 IntVector2::UP(0, 1);
const IntVector2 IntVector2::DOWN(0, -1);
const IntVector2 IntVector2::ONE(1, 1);

String Vector2::ToString() const
{
    char tempBuffer[CONVERSION_BUFFER_LENGTH];
    sprintf(tempBuffer, "%g %g", x, y);
    return String(tempBuffer);
}

String IntVector2::ToString() const
{
    char tempBuffer[CONVERSION_BUFFER_LENGTH];
    sprintf(tempBuffer, "%d %d", x, y);
    return String(tempBuffer);
}

}
