// Copyright (c) 2008-2023 the Urho3D project
// Copyright (c) 2022-2023 the Dviglo project
// License: MIT

#include "appstate_benchmark02.h"
#include "app_state_manager.h"
#include "benchmark02_woman_mover.h"

#include <dviglo/core/context.h>
#include <dviglo/graphics/animated_model.h>
#include <dviglo/graphics/animation.h>
#include <dviglo/graphics/animation_controller.h>
#include <dviglo/graphics/animation_state.h>
#include <dviglo/input/input.h>
#include <dviglo/resource/resource_cache.h>
#include <dviglo/scene/scene_events.h>
#include <dviglo/scene/spline_path.h>

#include <dviglo/common/debug_new.h>

AppState_Benchmark02::AppState_Benchmark02()
{
    name_ = "Orcs & Humans";

    // This constructor is called once when the application runs, so we can register here
    DV_CONTEXT->RegisterFactory<Benchmark02_WomanMover>();
}

void AppState_Benchmark02::OnEnter()
{
    assert(!scene_);
    LoadSceneXml("benchmark/scenes/benchmark02.xml");

    Vector3 castlePos = scene_->GetChild("Castle")->GetPosition();
    BoundingBox castleTop(castlePos - Vector3(7.f, 0.f, 7.f), castlePos + Vector3(7.f, 0.f, 7.f));

    Vector<Node*> womans;
    scene_->GetChildrenWithTag(womans, "woman");
    for (Node* woman : womans)
    {
        Benchmark02_WomanMover* mover = woman->create_component<Benchmark02_WomanMover>();
        mover->SetParameters(2.f, 100.f, castleTop);

        AnimatedModel* modelObject = woman->GetComponent<AnimatedModel>();
        Animation* walkAnimation = DV_RES_CACHE->GetResource<Animation>("models/kachujin/kachujin_walk.ani");
        AnimationState* state = modelObject->AddAnimationState(walkAnimation);
        if (state)
        {
            state->SetWeight(1.0f);
            state->SetLooped(true);
            state->SetTime(Random(walkAnimation->GetLength()));
        }
    }

    Vector<Node*> mutants;
    scene_->GetChildrenWithTag(mutants, "mutant");
    for (Node* mutant : mutants)
    {
        AnimationController* animCtrl = mutant->create_component<AnimationController>();
        animCtrl->PlayExclusive("models/mutant/mutant_idle0.ani", 0, true, 0.f);
        animCtrl->SetTime("models/mutant/mutant_idle0.ani", Random(animCtrl->GetLength("models/mutant/mutant_idle0.ani")));
    }

    Node* mutantGeneral = scene_->GetChild("MutantGeneral");
    AnimationController* generalAnimCtrl = mutantGeneral->create_component<AnimationController>();
    generalAnimCtrl->PlayExclusive("models/mutant/mutant_idle1.ani", 0, true, 0.f);
    generalAnimCtrl->SetTime("models/mutant/mutant_idle1.ani", Random(generalAnimCtrl->GetLength("models/mutant/mutant_idle1.ani")));

    Node* cameraNode = scene_->GetChild("Camera");
    
    Node* cameraPath = scene_->GetChild("CameraPath");
    SplinePath* cameraSplinePath = cameraPath->create_component<SplinePath>();
    cameraSplinePath->SetControlledNode(cameraNode);
    for (Node* child : cameraPath->GetChildren())
        cameraSplinePath->AddControlPoint(child);
    cameraSplinePath->SetSpeed(2.f);
    cameraSplinePath->SetInterpolationMode(InterpolationMode::CATMULL_ROM_FULL_CURVE);

    Node* cameraTargetNode = scene_->create_child("CameraTarget");
    Node* cameraTargetPath = scene_->GetChild("CameraTargetPath");
    SplinePath* cameraTargetSplinePath = cameraPath->create_component<SplinePath>();
    cameraTargetSplinePath->SetControlledNode(cameraTargetNode);
    for (Node* child : cameraPath->GetChildren())
        cameraTargetSplinePath->AddControlPoint(child);
    cameraTargetSplinePath->SetSpeed(2.f);
    cameraTargetSplinePath->SetInterpolationMode(InterpolationMode::CATMULL_ROM_FULL_CURVE);

    DV_INPUT->SetMouseVisible(false);
    setup_viewport();
    subscribe_to_event(scene_, E_SCENEUPDATE, DV_HANDLER(AppState_Benchmark02, HandleSceneUpdate));
    fpsCounter_.Clear();
}

void AppState_Benchmark02::OnLeave()
{
    UnsubscribeFromAllEvents();
    DestroyViewport();
    scene_ = nullptr;
}

void AppState_Benchmark02::HandleSceneUpdate(StringHash eventType, VariantMap& eventData)
{
    float timeStep = eventData[SceneUpdate::P_TIMESTEP].GetFloat();

    fpsCounter_.Update(timeStep);
    UpdateCurrentFpsElement();

    if (DV_INPUT->GetKeyDown(KEY_ESCAPE))
    {
        APP_STATE_MANAGER->SetRequiredAppStateId(APPSTATEID_MAINSCREEN);
        return;
    }

    Node* cameraPath = scene_->GetChild("CameraPath");
    SplinePath* cameraSplinePath = cameraPath->GetComponent<SplinePath>();
    cameraSplinePath->Move(timeStep);

    Node* cameraTargetPath = scene_->GetChild("CameraTargetPath");
    SplinePath* cameraTargetSplinePath = cameraPath->GetComponent<SplinePath>();
    cameraTargetSplinePath->Move(timeStep);

    Node* cameraTargetNode = scene_->GetChild("CameraTarget");
    Node* cameraNode = scene_->GetChild("Camera");
    cameraNode->LookAt(cameraTargetNode->GetPosition());

    if (cameraSplinePath->IsFinished())
        APP_STATE_MANAGER->SetRequiredAppStateId(APPSTATEID_RESULTSCREEN);
}
