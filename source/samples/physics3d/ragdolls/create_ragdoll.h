// Copyright (c) 2008-2023 the Urho3D project
// Copyright (c) 2022-2023 the Dviglo project
// License: MIT

#pragma once

#include <dviglo/physics/collision_shape.h>
#include <dviglo/physics/constraint.h>

using namespace dviglo;

/// Custom component that creates a ragdoll upon collision.
class CreateRagdoll : public Component
{
    DV_OBJECT(CreateRagdoll);

public:
    /// Construct.
    explicit CreateRagdoll();

protected:
    /// Handle node being assigned.
    void OnNodeSet(Node* node) override;

private:
    /// Handle scene node's physics collision.
    void HandleNodeCollision(StringHash eventType, VariantMap& eventData);
    /// Make a bone physical by adding RigidBody and CollisionShape components.
    void CreateRagdollBone(const String& boneName, ShapeType type, const Vector3& size, const Vector3& position, const Quaternion& rotation);
    /// Join two bones with a Constraint component.
    void CreateRagdollConstraint(const String& boneName, const String& parentName, ConstraintType type, const Vector3& axis, const Vector3& parentAxis, const Vector2& highLimit, const Vector2& lowLimit, bool disableCollision = true);
};
