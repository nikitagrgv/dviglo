// Copyright (c) 2008-2023 the Urho3D project
// Copyright (c) 2022-2023 the Dviglo project
// License: MIT

#pragma once

#include "../containers/ptr.h"
#include "../containers/ref_counted.h"
#include "../containers/vector.h"
#include "animation_defs.h"

namespace dviglo
{

class Object;
class ValueAnimation;
class Variant;
struct VAnimEventFrame;

/// Base class for a value animation instance, which includes animation runtime information and updates the target object's value automatically.
class DV_API ValueAnimationInfo : public RefCounted
{
public:
    /// Construct without target object.
    ValueAnimationInfo(ValueAnimation* animation, WrapMode wrapMode, float speed);
    /// Construct with target object.
    ValueAnimationInfo(Object* target, ValueAnimation* animation, WrapMode wrapMode, float speed);
    /// Copy construct.
    ValueAnimationInfo(const ValueAnimationInfo& other);
    /// Destruct.
    ~ValueAnimationInfo() override;

    /// Advance time position and apply. Return true when the animation is finished. No-op when the target object is not defined.
    bool Update(float timeStep);
    /// Set time position and apply. Return true when the animation is finished. No-op when the target object is not defined.
    bool SetTime(float time);

    /// Set wrap mode.
    void SetWrapMode(WrapMode wrapMode) { wrap_mode_ = wrapMode; }

    /// Set speed.
    void SetSpeed(float speed) { speed_ = speed; }

    /// Return target object.
    Object* GetTarget() const;

    /// Return animation.
    ValueAnimation* GetAnimation() const { return animation_; }

    /// Return wrap mode.
    WrapMode wrap_mode() const { return wrap_mode_; }

    /// Return time position.
    float GetTime() const { return currentTime_; }

    /// Return speed.
    float GetSpeed() const { return speed_; }

protected:
    /// Apply new animation value to the target object. Called by Update().
    virtual void ApplyValue(const Variant& newValue);
    /// Calculate scaled time.
    float CalculateScaledTime(float currentTime, bool& finished) const;
    /// Return event frames.
    void GetEventFrames(float beginTime, float endTime, Vector<const VAnimEventFrame*>& eventFrames);

    /// Target object.
    WeakPtr<Object> target_;
    /// Attribute animation.
    SharedPtr<ValueAnimation> animation_;
    /// Wrap mode.
    WrapMode wrap_mode_;
    /// Animation speed.
    float speed_;
    /// Current time.
    float currentTime_;
    /// Last scaled time.
    float lastScaledTime_;
};

}
