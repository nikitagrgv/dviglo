// Copyright (c) 2008-2023 the Urho3D project
// Copyright (c) 2022-2023 the Dviglo project
// License: MIT

#include <dviglo/graphics/animated_model.h>
#include <dviglo/graphics/animation_state.h>
#include <dviglo/scene/scene.h>

#include "mover.h"

#include <dviglo/common/debug_new.h>

Mover::Mover() :
    moveSpeed_(0.0f),
    rotationSpeed_(0.0f)
{
    // Only the scene update event is needed: unsubscribe from the rest for optimization
    SetUpdateEventMask(LogicComponentEvents::Update);
}

void Mover::SetParameters(float moveSpeed, float rotationSpeed, const BoundingBox& bounds)
{
    moveSpeed_ = moveSpeed;
    rotationSpeed_ = rotationSpeed;
    bounds_ = bounds;
}

void Mover::Update(float timeStep)
{
    node_->Translate(Vector3::FORWARD * moveSpeed_ * timeStep);

    // If in risk of going outside the plane, rotate the model right
    Vector3 pos = node_->GetPosition();
    if (pos.x < bounds_.min_.x || pos.x > bounds_.max_.x || pos.z < bounds_.min_.z || pos.z > bounds_.max_.z)
        node_->Yaw(rotationSpeed_ * timeStep);

    // Get the model's first (only) animation state and advance its time. Note the convenience accessor to other components
    // in the same scene node
    auto* model = node_->GetComponent<AnimatedModel>(true);
    if (model->GetNumAnimationStates())
    {
        AnimationState* state = model->GetAnimationStates()[0];
        state->AddTime(timeStep);
    }
}
