// Copyright (c) 2008-2023 the Urho3D project
// Copyright (c) 2022-2023 the Dviglo project
// License: MIT

#pragma once

#include "collision_shape_2d.h"

namespace dviglo
{

/// 2D polygon collision component.
class DV_API CollisionPolygon2D : public CollisionShape2D
{
    DV_OBJECT(CollisionPolygon2D);

public:
    /// Construct.
    explicit CollisionPolygon2D();
    /// Destruct.
    ~CollisionPolygon2D() override;
    /// Register object factory.
    static void register_object();

    /// Set vertex count.
    void SetVertexCount(i32 count);
    /// Set vertex.
    void SetVertex(i32 index, const Vector2& vertex);
    /// Set vertices.
    void SetVertices(const Vector<Vector2>& vertices);

    /// Return vertex count.
    i32 GetVertexCount() const { return vertices_.Size(); }

    /// Return vertex.
    const Vector2& GetVertex(i32 index) const
    {
        assert(index >= 0);
        return (index < vertices_.Size()) ? vertices_[index] : Vector2::ZERO;
    }

    /// Return vertices.
    const Vector<Vector2>& GetVertices() const { return vertices_; }

    /// Set vertices attribute.
    void SetVerticesAttr(const Vector<byte>& value);
    /// Return vertices attribute.
    Vector<byte> GetVerticesAttr() const;

private:
    /// Apply node world scale.
    void ApplyNodeWorldScale() override;
    /// Recreate fixture.
    void RecreateFixture();

    /// Polygon shape.
    b2PolygonShape polygon_shape_;
    /// Vertices.
    Vector<Vector2> vertices_;
};

}
