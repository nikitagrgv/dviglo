// Copyright (c) 2008-2023 the Urho3D project
// Copyright (c) 2022-2023 the Dviglo project
// License: MIT

#include "../core/context.h"
#include "constraint_pulley_2d.h"
#include "physics_utils_2d.h"
#include "rigid_body_2d.h"

#include "../common/debug_new.h"

namespace dviglo
{

extern const char* PHYSICS2D_CATEGORY;

ConstraintPulley2D::ConstraintPulley2D() :
    ownerBodyGroundAnchor_(-1.0f, 1.0f),
    otherBodyGroundAnchor_(1.0f, 1.0f),
    ownerBodyAnchor_(-1.0f, 0.0f),
    otherBodyAnchor_(1.0f, 0.0f)
{

}

ConstraintPulley2D::~ConstraintPulley2D() = default;

void ConstraintPulley2D::register_object()
{
    DV_CONTEXT->RegisterFactory<ConstraintPulley2D>(PHYSICS2D_CATEGORY);

    DV_ACCESSOR_ATTRIBUTE("Is Enabled", IsEnabled, SetEnabled, true, AM_DEFAULT);
    DV_ACCESSOR_ATTRIBUTE("Owner Body Ground Anchor", GetOwnerBodyGroundAnchor, SetOwnerBodyGroundAnchor, Vector2::ZERO,
        AM_DEFAULT);
    DV_ACCESSOR_ATTRIBUTE("Other Body Ground Anchor", GetOtherBodyGroundAnchor, SetOtherBodyGroundAnchor, Vector2::ZERO,
        AM_DEFAULT);
    DV_ACCESSOR_ATTRIBUTE("Owner Body Anchor", GetOwnerBodyAnchor, SetOwnerBodyAnchor, Vector2::ZERO, AM_DEFAULT);
    DV_ACCESSOR_ATTRIBUTE("Other Body Anchor", GetOtherBodyAnchor, SetOtherBodyAnchor, Vector2::ZERO, AM_DEFAULT);
    DV_ACCESSOR_ATTRIBUTE("Ratio", GetRatio, SetRatio, 0.0f, AM_DEFAULT);
    DV_COPY_BASE_ATTRIBUTES(Constraint2D);
}

void ConstraintPulley2D::SetOwnerBodyGroundAnchor(const Vector2& groundAnchor)
{
    if (groundAnchor == ownerBodyGroundAnchor_)
        return;

    ownerBodyGroundAnchor_ = groundAnchor;

    RecreateJoint();
    MarkNetworkUpdate();
}

void ConstraintPulley2D::SetOtherBodyGroundAnchor(const Vector2& groundAnchor)
{
    if (groundAnchor == otherBodyGroundAnchor_)
        return;

    otherBodyGroundAnchor_ = groundAnchor;

    RecreateJoint();
    MarkNetworkUpdate();
}

void ConstraintPulley2D::SetOwnerBodyAnchor(const Vector2& anchor)
{
    if (anchor == ownerBodyAnchor_)
        return;

    ownerBodyAnchor_ = anchor;

    RecreateJoint();
    MarkNetworkUpdate();
}

void ConstraintPulley2D::SetOtherBodyAnchor(const Vector2& anchor)
{
    if (anchor == otherBodyAnchor_)
        return;

    otherBodyAnchor_ = anchor;

    RecreateJoint();
    MarkNetworkUpdate();
}

void ConstraintPulley2D::SetRatio(float ratio)
{
    if (ratio == jointDef_.ratio)
        return;

    jointDef_.ratio = ratio;

    RecreateJoint();
    MarkNetworkUpdate();
}

b2JointDef* ConstraintPulley2D::GetJointDef()
{
    if (!ownerBody_ || !otherBody_)
        return nullptr;

    b2Body* bodyA = ownerBody_->GetBody();
    b2Body* bodyB = otherBody_->GetBody();
    if (!bodyA || !bodyB)
        return nullptr;

    jointDef_.Initialize(bodyA, bodyB, ToB2Vec2(ownerBodyGroundAnchor_), ToB2Vec2(otherBodyGroundAnchor_),
        ToB2Vec2(ownerBodyAnchor_), ToB2Vec2(otherBodyAnchor_), jointDef_.ratio);

    return &jointDef_;
}

}
