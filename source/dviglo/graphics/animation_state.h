// Copyright (c) 2008-2023 the Urho3D project
// Copyright (c) 2022-2023 the Dviglo project
// License: MIT

/// \file

#pragma once

#include "../containers/hash_map.h"
#include "../containers/ptr.h"

namespace dviglo
{

class Animation;
class AnimatedModel;
class Deserializer;
class Node;
class Serializer;
class Skeleton;
class StringHash;
struct AnimationTrack;
struct Bone;

/// %Animation blending mode.
enum AnimationBlendMode
{
    // Lerp blending (default)
    ABM_LERP = 0,
    // Additive blending based on difference from bind pose
    ABM_ADDITIVE
};

/// %Animation instance per-track data.
struct AnimationStateTrack
{
    /// Construct with defaults.
    AnimationStateTrack();
    /// Destruct.
    ~AnimationStateTrack();

    /// Animation track.
    const AnimationTrack* track_;
    /// Bone pointer.
    Bone* bone_;
    /// Scene node pointer.
    WeakPtr<Node> node_;
    /// Blending weight.
    float weight_;
    /// Last key frame.
    i32 keyFrame_;
};

/// %Animation instance.
class DV_API AnimationState : public RefCounted
{
public:
    /// Construct with animated model and animation pointers.
    AnimationState(AnimatedModel* model, Animation* animation);
    /// Construct with root scene node and animation pointers.
    AnimationState(Node* node, Animation* animation);
    /// Destruct.
    ~AnimationState() override;

    /// Set start bone. Not supported in node animation mode. Resets any assigned per-bone weights.
    void SetStartBone(Bone* startBone);
    /// Set looping enabled/disabled.
    void SetLooped(bool looped);
    /// Set blending weight.
    void SetWeight(float weight);
    /// Set blending mode.
    void SetBlendMode(AnimationBlendMode mode);
    /// Set time position. Does not fire animation triggers.
    void SetTime(float time);
    /// Set per-bone blending weight by track index. Default is 1.0 (full), is multiplied  with the state's blending weight when applying the animation. Optionally recurses to child bones.
    void SetBoneWeight(i32 index, float weight, bool recursive = false);
    /// Set per-bone blending weight by name.
    void SetBoneWeight(const String& name, float weight, bool recursive = false);
    /// Set per-bone blending weight by name hash.
    void SetBoneWeight(StringHash nameHash, float weight, bool recursive = false);
    /// Modify blending weight.
    void AddWeight(float delta);
    /// Modify time position. %Animation triggers will be fired.
    void AddTime(float delta);
    /// Set blending layer.
    void SetLayer(unsigned char layer); // TODO: i8?

    /// Return animation.
    Animation* GetAnimation() const { return animation_; }

    /// Return animated model this state belongs to (model mode).
    AnimatedModel* GetModel() const;
    /// Return root scene node this state controls (node hierarchy mode).
    Node* GetNode() const;
    /// Return start bone.
    Bone* GetStartBone() const;
    /// Return per-bone blending weight by track index.
    float GetBoneWeight(i32 index) const;
    /// Return per-bone blending weight by name.
    float GetBoneWeight(const String& name) const;
    /// Return per-bone blending weight by name.
    float GetBoneWeight(StringHash nameHash) const;
    /// Return track index with matching bone node, or NINDEX if not found.
    i32 GetTrackIndex(Node* node) const;
    /// Return track index by bone name, or NINDEX if not found.
    i32 GetTrackIndex(const String& name) const;
    /// Return track index by bone name hash, or NINDEX if not found.
    i32 GetTrackIndex(StringHash nameHash) const;

    /// Return whether weight is nonzero.
    bool IsEnabled() const { return weight_ > 0.0f; }

    /// Return whether looped.
    bool IsLooped() const { return looped_; }

    /// Return blending weight.
    float GetWeight() const { return weight_; }

    /// Return blending mode.
    AnimationBlendMode blend_mode() const { return blendingMode_; }

    /// Return time position.
    float GetTime() const { return time_; }

    /// Return animation length.
    float GetLength() const;

    /// Return blending layer.
    unsigned char GetLayer() const { return layer_; }

    /// Apply the animation at the current time position.
    void Apply();

private:
    /// Apply animation to a skeleton. Transform changes are applied silently, so the model needs to dirty its root model afterward.
    void ApplyToModel();
    /// Apply animation to a scene node hierarchy.
    void ApplyToNodes();
    /// Apply track.
    void ApplyTrack(AnimationStateTrack& stateTrack, float weight, bool silent);

    /// Animated model (model mode).
    WeakPtr<AnimatedModel> model_;
    /// Root scene node (node hierarchy mode).
    WeakPtr<Node> node_;
    /// Animation.
    SharedPtr<Animation> animation_;
    /// Start bone.
    Bone* startBone_;
    /// Per-track data.
    Vector<AnimationStateTrack> stateTracks_;
    /// Looped flag.
    bool looped_;
    /// Blending weight.
    float weight_;
    /// Time position.
    float time_;
    /// Blending layer.
    unsigned char layer_;
    /// Blending mode.
    AnimationBlendMode blendingMode_;
};

}
