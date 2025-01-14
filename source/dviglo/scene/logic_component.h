// Copyright (c) 2008-2023 the Urho3D project
// Copyright (c) 2022-2023 the Dviglo project
// License: MIT

/// \file

#pragma once

#include "component.h"
#include "scene.h"

#include "../containers/flag_set.h"


namespace dviglo
{

enum class LogicComponentEvents
{
    /// Not use any events
    None            = 0,

    /// Use the scene update event
    Update          = 1 << 0,

    /// Use the scene post-update event
    PostUpdate      = 1 << 1,

    /// Use the physics update event
    FixedUpdate     = 1 << 2,

    /// Use the physics post-update event
    FixedPostUpdate = 1 << 3,

    /// Use all events
    All             = Update | PostUpdate | FixedUpdate | FixedPostUpdate
};
DV_FLAGS(LogicComponentEvents);

/// Helper base class for user-defined game logic components that hooks up to update events and forwards them to virtual functions similar to ScriptInstance class.
class DV_API LogicComponent : public Component
{
    DV_OBJECT(LogicComponent);

public:
    SlotSceneUpdate scene_update;
    SlotScenePostUpdate scene_post_update;

    /// Construct.
    explicit LogicComponent();
    /// Destruct.
    ~LogicComponent() override;

    /// Handle enabled/disabled state change. Changes update event subscription.
    void OnSetEnabled() override;

    /// Called when the component is added to a scene node. Other components may not yet exist.
    virtual void Start() { }

    /// Called before the first update. At this point all other components of the node should exist. Will also be called if update events are not wanted; in that case the event is immediately unsubscribed afterward.
    virtual void DelayedStart() { }

    /// Called when the component is detached from a scene node, usually on destruction. Note that you will no longer have access to the node and scene at that point.
    virtual void Stop() { }

    /// Called on scene update, variable timestep.
    virtual void Update(float timeStep);
    /// Called on scene post-update, variable timestep.
    virtual void PostUpdate(float timeStep);
    /// Called on physics update, fixed timestep.
    virtual void FixedUpdate(float timeStep);
    /// Called on physics post-update, fixed timestep.
    virtual void FixedPostUpdate(float timeStep);

    /// Set what update events should be subscribed to. Use this for optimization: by default all are in use. Note that this is not an attribute and is not saved or network-serialized, therefore it should always be called eg. in the subclass constructor.
    void SetUpdateEventMask(LogicComponentEvents mask);

    /// Return what update events are subscribed to.
    LogicComponentEvents GetUpdateEventMask() const { return updateEventMask_; }

    /// Return whether the DelayedStart() function has been called.
    bool IsDelayedStartCalled() const { return delayedStartCalled_; }

protected:
    /// Handle scene node being assigned at creation.
    void OnNodeSet(Node* node) override;
    /// Handle scene being assigned.
    void OnSceneSet(Scene* scene) override;

private:
    /// Subscribe/unsubscribe to update events based on current enabled state and update event mask.
    void UpdateEventSubscription();
    /// Handle scene update event.
    void handle_scene_update(Scene* scene, float time_step);
    /// Handle scene post-update event.
    void handle_scene_post_update(Scene* scene, float time_step);
#if defined(DV_BULLET) || defined(DV_BOX2D)
    /// Handle physics pre-step event.
    void HandlePhysicsPreStep(StringHash eventType, VariantMap& eventData);
    /// Handle physics post-step event.
    void HandlePhysicsPostStep(StringHash eventType, VariantMap& eventData);
#endif
    /// Requested event subscription mask.
    LogicComponentEvents updateEventMask_;
    /// Current event subscription mask.
    LogicComponentEvents currentEventMask_;
    /// Flag for delayed start.
    bool delayedStartCalled_;
};

} // namespace dviglo
