// Copyright (c) 2008-2023 the Urho3D project
// Copyright (c) 2022-2023 the Dviglo project
// License: MIT

#pragma once

#include <dviglo/input/controls.h>
#include <dviglo/physics/physics_utils.h>
#include <dviglo/scene/logic_component.h>

namespace dviglo
{
    class Constraint;
    class Node;
    class RigidBody;
}

using namespace dviglo;

const unsigned CTRL_FORWARD = (1u << 0u);
const unsigned CTRL_BACK = (1u << 1u);
const unsigned CTRL_LEFT = (1u << 2u);
const unsigned CTRL_RIGHT = (1u << 3u);
const unsigned CTRL_BRAKE = (1u << 4u);
const float YAW_SENSITIVITY = 0.1f;
const float ENGINE_POWER = 10.0f;
const float MAX_WHEEL_ANGLE = 22.5f;

// Vehicle component, responsible for physical movement according to controls.
// Encapsulates RaycastVehicle
class Vehicle : public LogicComponent
{
    DV_OBJECT(Vehicle)

public :
    /// Construct.
    explicit Vehicle();
    /// Destruct.
    ~Vehicle() override;

    /// Register object factory and attributes.
    static void register_object();

    /// Perform post-load after deserialization. Acquire the components from the scene nodes.
    void apply_attributes() override;

    /// Initialize the vehicle. Create rendering and physics components. Called by the application.
    void Init();

    /// Handle physics world update. Called by LogicComponent base class.
    void FixedUpdate(float timeStep) override;
    /// Updating wheel effects here.
    void PostUpdate(float timeStep) override;

    /// Movement controls.
    Controls controls_;

    /// Get steering value.
    float GetSteering() { return steering_; }

    /// Set steering value.
    void SetSteering(float steering) { steering_ = steering; }

    /// Get wheel radius.
    float GetWheelRadius() { return wheelRadius_; }

    /// Get wheel width.
    float GetWheelWidth() { return wheelWidth_; }

private:
    /// Creates particle emitter.
    void CreateEmitter(Vector3 place);

    /// Current left/right steering amount (-1 to 1.)
    float steering_;
    /// Tmp storage for steering
    float vehicleSteering_;
    /// Linear momentum supplied by engine to RigidBody
    float engineForce_;
    /// Rotational momentum preventing (dampening) wheels rotation
    float brakingForce_;
    /// Maximum linear momentum supplied by engine to RigidBody
    float maxEngineForce_;
    /// Stored wheel radius
    float wheelRadius_;
    /// Suspension rest length (in meters)
    float suspensionRestLength_;
    /// Width of wheel (used only in calculation of wheel placement)
    float wheelWidth_;
    /// Suspension stiffness
    float suspensionStiffness_;
    /// Suspension damping
    float suspensionDamping_;
    /// Suspension compression
    float suspensionCompression_;
    /// Wheel friction
    float wheelFriction_;
    /// Wheel roll influence (how much car will turn sidewise)
    float rollInfluence_;
    /// Emitter data for saving.
    Vector<Node*> particleEmitterNodeList_;
    /// Value to calculate acceleration.
    Vector3 prevVelocity_;
    /// Storing points for emitters
    Vector3 connectionPoints_[4];
    /// Do not recreate emitters if they are already created.
    bool emittersCreated;
};
