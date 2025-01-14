// Copyright (c) 2008-2023 the Urho3D project
// Copyright (c) 2022-2023 the Dviglo project
// License: MIT

#pragma once

#include "../containers/str.h"
#include "math_defs.h"

namespace dviglo
{

/// Two-dimensional vector with integer values.
struct DV_API IntVector2
{
    /// Construct a zero vector.
    IntVector2() noexcept :
        x(0),
        y(0)
    {
    }

    /// Construct from coordinates.
    IntVector2(int x, int y) noexcept :
        x(x),
        y(y)
    {
    }

    /// Construct from an int array.
    explicit IntVector2(const int* data) noexcept :
        x(data[0]),
        y(data[1])
    {
    }

    /// Construct from an float array.
    explicit IntVector2(const float* data) :
        x((int)data[0]),
        y((int)data[1])
    {
    }
    /// Copy-construct from another vector.
    IntVector2(const IntVector2& rhs) noexcept = default;

    /// Assign from another vector.
    IntVector2& operator =(const IntVector2& rhs) noexcept = default;

    /// Test for equality with another vector.
    bool operator ==(const IntVector2& rhs) const { return x == rhs.x && y == rhs.y; }

    /// Test for inequality with another vector.
    bool operator !=(const IntVector2& rhs) const { return x != rhs.x || y != rhs.y; }

    /// Add a vector.
    IntVector2 operator +(const IntVector2& rhs) const { return IntVector2(x + rhs.x, y + rhs.y); }

    /// Return negation.
    IntVector2 operator -() const { return IntVector2(-x, -y); }

    /// Subtract a vector.
    IntVector2 operator -(const IntVector2& rhs) const { return IntVector2(x - rhs.x, y - rhs.y); }

    /// Multiply with a scalar.
    IntVector2 operator *(int rhs) const { return IntVector2(x * rhs, y * rhs); }

    /// Multiply with a vector.
    IntVector2 operator *(const IntVector2& rhs) const { return IntVector2(x * rhs.x, y * rhs.y); }

    /// Divide by a scalar.
    IntVector2 operator /(int rhs) const { return IntVector2(x / rhs, y / rhs); }

    /// Divide by a vector.
    IntVector2 operator /(const IntVector2& rhs) const { return IntVector2(x / rhs.x, y / rhs.y); }

    /// Add-assign a vector.
    IntVector2& operator +=(const IntVector2& rhs)
    {
        x += rhs.x;
        y += rhs.y;
        return *this;
    }

    /// Subtract-assign a vector.
    IntVector2& operator -=(const IntVector2& rhs)
    {
        x -= rhs.x;
        y -= rhs.y;
        return *this;
    }

    /// Multiply-assign a scalar.
    IntVector2& operator *=(int rhs)
    {
        x *= rhs;
        y *= rhs;
        return *this;
    }

    /// Multiply-assign a vector.
    IntVector2& operator *=(const IntVector2& rhs)
    {
        x *= rhs.x;
        y *= rhs.y;
        return *this;
    }

    /// Divide-assign a scalar.
    IntVector2& operator /=(int rhs)
    {
        x /= rhs;
        y /= rhs;
        return *this;
    }

    /// Divide-assign a vector.
    IntVector2& operator /=(const IntVector2& rhs)
    {
        x /= rhs.x;
        y /= rhs.y;
        return *this;
    }

    /// Return integer data.
    const int* Data() const { return &x; }

    /// Return as string.
    String ToString() const;

    /// Return hash value for HashSet & HashMap.
    hash32 ToHash() const { return (hash32)x * 31 + (hash32)y; }

    /// Return length.
    float Length() const { return sqrtf((float)(x * x + y * y)); }

    /// X coordinate.
    int x;
    /// Y coordinate.
    int y;

    /// Zero vector.
    static const IntVector2 ZERO;
    /// (-1,0) vector.
    static const IntVector2 LEFT;
    /// (1,0) vector.
    static const IntVector2 RIGHT;
    /// (0,1) vector.
    static const IntVector2 UP;
    /// (0,-1) vector.
    static const IntVector2 DOWN;
    /// (1,1) vector.
    static const IntVector2 ONE;
};

/// Two-dimensional vector.
struct DV_API Vector2
{
    /// Construct a zero vector.
    Vector2() noexcept :
        x(0.0f),
        y(0.0f)
    {
    }

    /// Copy-construct from another vector.
    Vector2(const Vector2& vector) noexcept = default;

    /// Construct from an IntVector2.
    explicit Vector2(const IntVector2& vector) noexcept :
        x((float)vector.x),
        y((float)vector.y)
    {
    }

    /// Construct from coordinates.
    Vector2(float x, float y) noexcept :
        x(x),
        y(y)
    {
    }

    /// Construct from int coordinates.
    Vector2(i32 x, i32 y) noexcept :
        x((float)x),
        y((float)y)
    {
    }

    /// Construct from a float array.
    explicit Vector2(const float* data) noexcept :
        x(data[0]),
        y(data[1])
    {
    }

    /// Assign from another vector.
    Vector2& operator =(const Vector2& rhs) noexcept = default;

    /// Test for equality with another vector without epsilon.
    bool operator ==(const Vector2& rhs) const { return x == rhs.x && y == rhs.y; }

    /// Test for inequality with another vector without epsilon.
    bool operator !=(const Vector2& rhs) const { return x != rhs.x || y != rhs.y; }

    /// Add a vector.
    Vector2 operator +(const Vector2& rhs) const { return Vector2(x + rhs.x, y + rhs.y); }

    /// Return negation.
    Vector2 operator -() const { return Vector2(-x, -y); }

    /// Subtract a vector.
    Vector2 operator -(const Vector2& rhs) const { return Vector2(x - rhs.x, y - rhs.y); }

    /// Multiply with a scalar.
    Vector2 operator *(float rhs) const { return Vector2(x * rhs, y * rhs); }

    /// Multiply with a vector.
    Vector2 operator *(const Vector2& rhs) const { return Vector2(x * rhs.x, y * rhs.y); }

    /// Divide by a scalar.
    Vector2 operator /(float rhs) const { return Vector2(x / rhs, y / rhs); }

    /// Divide by a vector.
    Vector2 operator /(const Vector2& rhs) const { return Vector2(x / rhs.x, y / rhs.y); }

    /// Add-assign a vector.
    Vector2& operator +=(const Vector2& rhs)
    {
        x += rhs.x;
        y += rhs.y;
        return *this;
    }

    /// Subtract-assign a vector.
    Vector2& operator -=(const Vector2& rhs)
    {
        x -= rhs.x;
        y -= rhs.y;
        return *this;
    }

    /// Multiply-assign a scalar.
    Vector2& operator *=(float rhs)
    {
        x *= rhs;
        y *= rhs;
        return *this;
    }

    /// Multiply-assign a vector.
    Vector2& operator *=(const Vector2& rhs)
    {
        x *= rhs.x;
        y *= rhs.y;
        return *this;
    }

    /// Divide-assign a scalar.
    Vector2& operator /=(float rhs)
    {
        float invRhs = 1.0f / rhs;
        x *= invRhs;
        y *= invRhs;
        return *this;
    }

    /// Divide-assign a vector.
    Vector2& operator /=(const Vector2& rhs)
    {
        x /= rhs.x;
        y /= rhs.y;
        return *this;
    }

    /// Normalize to unit length.
    void Normalize()
    {
        float lenSquared = LengthSquared();
        if (!dviglo::Equals(lenSquared, 1.0f) && lenSquared > 0.0f)
        {
            float invLen = 1.0f / sqrtf(lenSquared);
            x *= invLen;
            y *= invLen;
        }
    }

    /// Return length.
    float Length() const { return sqrtf(x * x + y * y); }

    /// Return squared length.
    float LengthSquared() const { return x * x + y * y; }

    /// Calculate dot product.
    float DotProduct(const Vector2& rhs) const { return x * rhs.x + y * rhs.y; }

    /// Calculate absolute dot product.
    float AbsDotProduct(const Vector2& rhs) const { return dviglo::Abs(x * rhs.x) + dviglo::Abs(y * rhs.y); }

    /// Project vector onto axis.
    float ProjectOntoAxis(const Vector2& axis) const { return DotProduct(axis.normalized()); }

    /// Returns the angle between this vector and another vector in degrees.
    float Angle(const Vector2& rhs) const { return dviglo::Acos(DotProduct(rhs) / (Length() * rhs.Length())); }

    /// Return absolute vector.
    Vector2 Abs() const { return Vector2(dviglo::Abs(x), dviglo::Abs(y)); }

    /// Linear interpolation with another vector.
    Vector2 Lerp(const Vector2& rhs, float t) const { return *this * (1.0f - t) + rhs * t; }

    /// Test for equality with another vector with epsilon.
    bool Equals(const Vector2& rhs) const { return dviglo::Equals(x, rhs.x) && dviglo::Equals(y, rhs.y); }

    /// Return whether any component is NaN.
    bool is_nan() const { return std::isnan(x) || std::isnan(y); }

    /// Return whether any component is Inf.
    bool is_inf() const { return std::isinf(x) || std::isinf(y); }

    /// Return normalized to unit length.
    Vector2 normalized() const
    {
        const float lenSquared = LengthSquared();
        if (!dviglo::Equals(lenSquared, 1.0f) && lenSquared > 0.0f)
        {
            const float invLen = 1.0f / sqrtf(lenSquared);
            return *this * invLen;
        }
        else
            return *this;
    }

    /// Return normalized to unit length or zero if length is too small.
    Vector2 NormalizedOrDefault(const Vector2& defaultValue = Vector2::ZERO, float eps = M_LARGE_EPSILON) const
    {
        const float lenSquared = LengthSquared();
        if (lenSquared < eps * eps)
            return defaultValue;
        return *this / sqrtf(lenSquared);
    }

    /// Return normalized vector with length in given range.
    Vector2 ReNormalized(float minLength, float maxLength, const Vector2& defaultValue = Vector2::ZERO, float eps = M_LARGE_EPSILON) const
    {
        const float lenSquared = LengthSquared();
        if (lenSquared < eps * eps)
            return defaultValue;

        const float len = sqrtf(lenSquared);
        const float newLen = Clamp(len, minLength, maxLength);
        return *this * (newLen / len);
    }

    /// Return float data.
    const float* Data() const { return &x; }

    /// Return as string.
    String ToString() const;

    /// X coordinate.
    float x;
    /// Y coordinate.
    float y;

    /// Zero vector.
    static const Vector2 ZERO;
    /// (-1,0) vector.
    static const Vector2 LEFT;
    /// (1,0) vector.
    static const Vector2 RIGHT;
    /// (0,1) vector.
    static const Vector2 UP;
    /// (0,-1) vector.
    static const Vector2 DOWN;
    /// (1,1) vector.
    static const Vector2 ONE;
};

/// Multiply Vector2 with a scalar.
inline Vector2 operator *(float lhs, const Vector2& rhs) { return rhs * lhs; }

/// Multiply IntVector2 with a scalar.
inline IntVector2 operator *(int lhs, const IntVector2& rhs) { return rhs * lhs; }

/// Per-component linear interpolation between two 2-vectors.
inline Vector2 VectorLerp(const Vector2& lhs, const Vector2& rhs, const Vector2& t) { return lhs + (rhs - lhs) * t; }

/// Per-component min of two 2-vectors.
inline Vector2 VectorMin(const Vector2& lhs, const Vector2& rhs) { return Vector2(Min(lhs.x, rhs.x), Min(lhs.y, rhs.y)); }

/// Per-component max of two 2-vectors.
inline Vector2 VectorMax(const Vector2& lhs, const Vector2& rhs) { return Vector2(Max(lhs.x, rhs.x), Max(lhs.y, rhs.y)); }

/// Per-component floor of 2-vector.
inline Vector2 VectorFloor(const Vector2& vec) { return Vector2(Floor(vec.x), Floor(vec.y)); }

/// Per-component round of 2-vector.
inline Vector2 VectorRound(const Vector2& vec) { return Vector2(Round(vec.x), Round(vec.y)); }

/// Per-component ceil of 2-vector.
inline Vector2 VectorCeil(const Vector2& vec) { return Vector2(Ceil(vec.x), Ceil(vec.y)); }

/// Per-component absolute value of 2-vector.
inline Vector2 VectorAbs(const Vector2& vec) { return Vector2(Abs(vec.x), Abs(vec.y)); }

/// Per-component floor of 2-vector. Returns IntVector2.
inline IntVector2 VectorFloorToInt(const Vector2& vec) { return IntVector2(FloorToInt(vec.x), FloorToInt(vec.y)); }

/// Per-component round of 2-vector. Returns IntVector2.
inline IntVector2 VectorRoundToInt(const Vector2& vec) { return IntVector2(RoundToInt(vec.x), RoundToInt(vec.y)); }

/// Per-component ceil of 2-vector. Returns IntVector2.
inline IntVector2 VectorCeilToInt(const Vector2& vec) { return IntVector2(CeilToInt(vec.x), CeilToInt(vec.y)); }

/// Per-component min of two 2-vectors.
inline IntVector2 VectorMin(const IntVector2& lhs, const IntVector2& rhs) { return IntVector2(Min(lhs.x, rhs.x), Min(lhs.y, rhs.y)); }

/// Per-component max of two 2-vectors.
inline IntVector2 VectorMax(const IntVector2& lhs, const IntVector2& rhs) { return IntVector2(Max(lhs.x, rhs.x), Max(lhs.y, rhs.y)); }

/// Per-component absolute value of integer 2-vector.
inline IntVector2 VectorAbs(const IntVector2& vec) { return IntVector2(Abs(vec.x), Abs(vec.y)); }

/// Return a random value from [0, 1) from 2-vector seed.
/// http://stackoverflow.com/questions/12964279/whats-the-origin-of-this-glsl-rand-one-liner
inline float StableRandom(const Vector2& seed) { return Fract(Sin(seed.DotProduct(Vector2(12.9898f, 78.233f)) * M_RADTODEG) * 43758.5453f); }

/// Return a random value from [0, 1) from scalar seed.
inline float StableRandom(float seed) { return StableRandom(Vector2(seed, seed)); }

}
