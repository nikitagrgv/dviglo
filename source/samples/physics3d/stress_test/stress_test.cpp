// Copyright (c) 2008-2023 the Urho3D project
// Copyright (c) 2022-2023 the Dviglo project
// License: MIT

#include <dviglo/core/core_events.h>
#include <dviglo/engine/engine.h>
#include <dviglo/graphics/camera.h>
#include <dviglo/graphics/debug_renderer.h>
#include <dviglo/graphics/graphics.h>
#include <dviglo/graphics/light.h>
#include <dviglo/graphics/material.h>
#include <dviglo/graphics/model.h>
#include <dviglo/graphics/octree.h>
#include <dviglo/graphics/renderer.h>
#include <dviglo/graphics/static_model.h>
#include <dviglo/graphics/zone.h>
#include <dviglo/input/input.h>
#include <dviglo/io/file.h>
#include <dviglo/io/file_system.h>
#include <dviglo/physics/collision_shape.h>
#include <dviglo/physics/physics_world.h>
#include <dviglo/physics/rigid_body.h>
#include <dviglo/resource/resource_cache.h>
#include <dviglo/scene/scene.h>
#include <dviglo/ui/font.h>
#include <dviglo/ui/text.h>
#include <dviglo/ui/ui.h>

#include "stress_test.h"

#include <dviglo/common/debug_new.h>

DV_DEFINE_APPLICATION_MAIN(PhysicsStressTest)

PhysicsStressTest::PhysicsStressTest() :
    draw_debug_(false)
{
}

void PhysicsStressTest::Start()
{
    // Execute base class startup
    Sample::Start();

    // Create the scene content
    create_scene();

    // Create the UI content
    create_instructions();

    // Setup the viewport for displaying the scene
    setup_viewport();

    // Hook up to the frame update and render post-update events
    subscribe_to_events();

    // Set the mouse mode to use in the sample
    Sample::InitMouseMode(MM_RELATIVE);
}

void PhysicsStressTest::create_scene()
{
    ResourceCache* cache = DV_RES_CACHE;

    scene_ = new Scene();

    // Create octree, use default volume (-1000, -1000, -1000) to (1000, 1000, 1000)
    // Create a physics simulation world with default parameters, which will update at 60fps. Like the Octree must
    // exist before creating drawable components, the PhysicsWorld must exist before creating physics components.
    // Finally, create a DebugRenderer component so that we can draw physics debug geometry
    scene_->create_component<Octree>();
    scene_->create_component<PhysicsWorld>();
    scene_->create_component<DebugRenderer>();

    // Create a Zone component for ambient lighting & fog control
    Node* zoneNode = scene_->create_child("Zone");
    auto* zone = zoneNode->create_component<Zone>();
    zone->SetBoundingBox(BoundingBox(-1000.0f, 1000.0f));
    zone->SetAmbientColor(Color(0.15f, 0.15f, 0.15f));
    zone->SetFogColor(Color(0.5f, 0.5f, 0.7f));
    zone->SetFogStart(100.0f);
    zone->SetFogEnd(300.0f);

    // Create a directional light to the world. Enable cascaded shadows on it
    Node* lightNode = scene_->create_child("DirectionalLight");
    lightNode->SetDirection(Vector3(0.6f, -1.0f, 0.8f));
    auto* light = lightNode->create_component<Light>();
    light->SetLightType(LIGHT_DIRECTIONAL);
    light->SetCastShadows(true);
    light->SetShadowBias(BiasParameters(0.00025f, 0.5f));
    // Set cascade splits at 10, 50 and 200 world units, fade shadows out at 80% of maximum shadow distance
    light->SetShadowCascade(CascadeParameters(10.0f, 50.0f, 200.0f, 0.0f, 0.8f));

    {
        // Create a floor object, 500 x 500 world units. Adjust position so that the ground is at zero Y
        Node* floorNode = scene_->create_child("Floor");
        floorNode->SetPosition(Vector3(0.0f, -0.5f, 0.0f));
        floorNode->SetScale(Vector3(500.0f, 1.0f, 500.0f));
        auto* floorObject = floorNode->create_component<StaticModel>();
        floorObject->SetModel(cache->GetResource<Model>("models/box.mdl"));
        floorObject->SetMaterial(cache->GetResource<Material>("materials/stone_tiled.xml"));

        // Make the floor physical by adding RigidBody and CollisionShape components
        /*RigidBody* body = */floorNode->create_component<RigidBody>();
        auto* shape = floorNode->create_component<CollisionShape>();
        shape->SetBox(Vector3::ONE);
    }

    {
        // Create static mushrooms with triangle mesh collision
        const unsigned NUM_MUSHROOMS = 50;
        for (unsigned i = 0; i < NUM_MUSHROOMS; ++i)
        {
            Node* mushroomNode = scene_->create_child("Mushroom");
            mushroomNode->SetPosition(Vector3(Random(400.0f) - 200.0f, 0.0f, Random(400.0f) - 200.0f));
            mushroomNode->SetRotation(Quaternion(0.0f, Random(360.0f), 0.0f));
            mushroomNode->SetScale(5.0f + Random(5.0f));
            auto* mushroomObject = mushroomNode->create_component<StaticModel>();
            mushroomObject->SetModel(cache->GetResource<Model>("models/mushroom.mdl"));
            mushroomObject->SetMaterial(cache->GetResource<Material>("materials/mushroom.xml"));
            mushroomObject->SetCastShadows(true);

            /*RigidBody* body = */mushroomNode->create_component<RigidBody>();
            auto* shape = mushroomNode->create_component<CollisionShape>();
            // By default the highest LOD level will be used, the LOD level can be passed as an optional parameter
            shape->SetTriangleMesh(mushroomObject->GetModel());
        }
    }

    {
        // Create a large amount of falling physics objects
        const unsigned NUM_OBJECTS = 1000;
        for (unsigned i = 0; i < NUM_OBJECTS; ++i)
        {
            Node* boxNode = scene_->create_child("Box");
            boxNode->SetPosition(Vector3(0.0f, i * 2.0f + 100.0f, 0.0f));
            auto* boxObject = boxNode->create_component<StaticModel>();
            boxObject->SetModel(cache->GetResource<Model>("models/box.mdl"));
            boxObject->SetMaterial(cache->GetResource<Material>("materials/stone_small.xml"));
            boxObject->SetCastShadows(true);

            // Give the RigidBody mass to make it movable and also adjust friction
            auto* body = boxNode->create_component<RigidBody>();
            body->SetMass(1.0f);
            body->SetFriction(1.0f);
            // Disable collision event signaling to reduce CPU load of the physics simulation
            body->SetCollisionEventMode(COLLISION_NEVER);
            auto* shape = boxNode->create_component<CollisionShape>();
            shape->SetBox(Vector3::ONE);
        }
    }

    // Create the camera. Limit far clip distance to match the fog. Note: now we actually create the camera node outside
    // the scene, because we want it to be unaffected by scene load / save
    cameraNode_ = new Node();
    auto* camera = cameraNode_->create_component<Camera>();
    camera->SetFarClip(300.0f);

    // Set an initial position for the camera scene node above the floor
    cameraNode_->SetPosition(Vector3(0.0f, 3.0f, -20.0f));
}

void PhysicsStressTest::create_instructions()
{
    // Construct new Text object, set string to display and font to use
    auto* instructionText = DV_UI->GetRoot()->create_child<Text>();
    instructionText->SetText(
        "Use WASD keys and mouse to move\n"
        "LMB to spawn physics objects\n"
        "F5 to save scene, F7 to load\n"
        "Space to toggle physics debug geometry"
    );
    instructionText->SetFont(DV_RES_CACHE->GetResource<Font>("fonts/anonymous pro.ttf"), 15);
    // The text has multiple rows. Center them in relation to each other
    instructionText->SetTextAlignment(HA_CENTER);

    // Position the text relative to the screen center
    instructionText->SetHorizontalAlignment(HA_CENTER);
    instructionText->SetVerticalAlignment(VA_CENTER);
    instructionText->SetPosition(0, DV_UI->GetRoot()->GetHeight() / 4);
}

void PhysicsStressTest::setup_viewport()
{
    // Set up a viewport to the Renderer subsystem so that the 3D scene can be seen
    SharedPtr<Viewport> viewport(new Viewport(scene_, cameraNode_->GetComponent<Camera>()));
    DV_RENDERER->SetViewport(0, viewport);
}

void PhysicsStressTest::subscribe_to_events()
{
    // Subscribe handle_update() function for processing update events
    subscribe_to_event(E_UPDATE, DV_HANDLER(PhysicsStressTest, handle_update));

    // Subscribe handle_post_render_update() function for processing the post-render update event, during which we request
    // debug geometry
    subscribe_to_event(E_POSTRENDERUPDATE, DV_HANDLER(PhysicsStressTest, handle_post_render_update));
}

void PhysicsStressTest::move_camera(float timeStep)
{
    // Do not move if the UI has a focused element (the console)
    if (DV_UI->GetFocusElement())
        return;

    Input* input = DV_INPUT;

    // Movement speed as world units per second
    const float MOVE_SPEED = 20.0f;
    // Mouse sensitivity as degrees per pixel
    const float MOUSE_SENSITIVITY = 0.1f;

    // Use this frame's mouse motion to adjust camera node yaw and pitch. Clamp the pitch between -90 and 90 degrees
    IntVector2 mouseMove = input->GetMouseMove();
    yaw_ += MOUSE_SENSITIVITY * mouseMove.x;
    pitch_ += MOUSE_SENSITIVITY * mouseMove.y;
    pitch_ = Clamp(pitch_, -90.0f, 90.0f);

    // Construct new orientation for the camera scene node from yaw and pitch. Roll is fixed to zero
    cameraNode_->SetRotation(Quaternion(pitch_, yaw_, 0.0f));

    // Read WASD keys and move the camera scene node to the corresponding direction if they are pressed
    if (input->GetKeyDown(KEY_W))
        cameraNode_->Translate(Vector3::FORWARD * MOVE_SPEED * timeStep);
    if (input->GetKeyDown(KEY_S))
        cameraNode_->Translate(Vector3::BACK * MOVE_SPEED * timeStep);
    if (input->GetKeyDown(KEY_A))
        cameraNode_->Translate(Vector3::LEFT * MOVE_SPEED * timeStep);
    if (input->GetKeyDown(KEY_D))
        cameraNode_->Translate(Vector3::RIGHT * MOVE_SPEED * timeStep);

    // "Shoot" a physics object with left mousebutton
    if (input->GetMouseButtonPress(MOUSEB_LEFT))
        SpawnObject();

    // Check for loading / saving the scene
    if (input->GetKeyPress(KEY_F5))
    {
        File saveFile(DV_FILE_SYSTEM->GetProgramDir() + "data/scenes/physics_stress_test.xml", FILE_WRITE);
        scene_->save_xml(saveFile);
    }
    if (input->GetKeyPress(KEY_F7))
    {
        File loadFile(DV_FILE_SYSTEM->GetProgramDir() + "data/scenes/physics_stress_test.xml", FILE_READ);
        scene_->load_xml(loadFile);
    }

    // Toggle physics debug geometry with space
    if (input->GetKeyPress(KEY_SPACE))
        draw_debug_ = !draw_debug_;
}

void PhysicsStressTest::SpawnObject()
{
    // Create a smaller box at camera position
    Node* boxNode = scene_->create_child("SmallBox");
    boxNode->SetPosition(cameraNode_->GetPosition());
    boxNode->SetRotation(cameraNode_->GetRotation());
    boxNode->SetScale(0.25f);
    auto* boxObject = boxNode->create_component<StaticModel>();
    boxObject->SetModel(DV_RES_CACHE->GetResource<Model>("models/box.mdl"));
    boxObject->SetMaterial(DV_RES_CACHE->GetResource<Material>("materials/stone_small.xml"));
    boxObject->SetCastShadows(true);

    // Create physics components, use a smaller mass also
    auto* body = boxNode->create_component<RigidBody>();
    body->SetMass(0.25f);
    body->SetFriction(0.75f);
    auto* shape = boxNode->create_component<CollisionShape>();
    shape->SetBox(Vector3::ONE);

    const float OBJECT_VELOCITY = 10.0f;

    // Set initial velocity for the RigidBody based on camera forward vector. Add also a slight up component
    // to overcome gravity better
    body->SetLinearVelocity(cameraNode_->GetRotation() * Vector3(0.0f, 0.25f, 1.0f) * OBJECT_VELOCITY);
}

void PhysicsStressTest::handle_update(StringHash eventType, VariantMap& eventData)
{
    using namespace Update;

    // Take the frame time step, which is stored as a float
    float timeStep = eventData[P_TIMESTEP].GetFloat();

    // Move the camera, scale movement with time step
    move_camera(timeStep);
}

void PhysicsStressTest::handle_post_render_update(StringHash eventType, VariantMap& eventData)
{
    // If draw debug mode is enabled, draw physics debug geometry. Use depth test to make the result easier to interpret
    if (draw_debug_)
        scene_->GetComponent<PhysicsWorld>()->draw_debug_geometry(true);
}
