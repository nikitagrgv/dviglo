// Copyright (c) 2008-2023 the Urho3D project
// Copyright (c) 2022-2023 the Dviglo project
// License: MIT

#include "app_state_manager.h"

#include <dviglo/core/core_events.h>
#include <dviglo/engine/application.h>
#include <dviglo/engine/engine_defs.h>
#include <dviglo/input/input.h>
#include <dviglo/io/file_system.h>
#include <dviglo/io/fs_base.h>
#include <dviglo/resource/resource_cache.h>
#include <dviglo/ui/text.h>
#include <dviglo/ui/ui.h>

using namespace std;


class App : public Application
{
    DV_OBJECT(App);

    SlotBeginFrame begin_frame;

public:
    App()
    {
        // The first handler for the first event in each frame.
        // To prevent a crash, we can only change the current scene at the start of a frame,
        // before any scene events are processed
        begin_frame.connect(DV_TIME->begin_frame, bind(&App::apply_app_state, this, placeholders::_1, placeholders::_2));
    }

    ~App()
    {
        delete AppStateManager::instance_;
    }

    void Setup() override
    {
        engineParameters_[EP_WINDOW_TITLE] = "Dviglo Benchmark";
        engineParameters_[EP_LOG_NAME] = get_pref_path("dviglo", "logs") + "other_benchmark.log";
        engineParameters_[EP_FULL_SCREEN] = false;
        engineParameters_[EP_WINDOW_WIDTH] = 960;
        engineParameters_[EP_WINDOW_HEIGHT] = 720;
        engineParameters_[EP_FRAME_LIMITER] = false;
    }

    // This elements can be used anywhere in the program
    void CreateCurrentFpsUiElement()
    {
        UiElement* root = DV_UI->GetRoot();
        root->SetDefaultStyle(DV_RES_CACHE->GetResource<XmlFile>("ui/default_style.xml"));

        Text* fpsElement = root->create_child<Text>(CURRENT_FPS_STR);
        fpsElement->SetStyleAuto();
        fpsElement->SetTextEffect(TE_SHADOW);
        fpsElement->SetPosition(10, 10);
        fpsElement->SetText("FPS");
    }

    void Start() override
    {
        // Указатель на объект хранится в самом классе
        new AppStateManager();

        APP_STATE_MANAGER->SetRequiredAppStateId(APPSTATEID_MAINSCREEN);

        DV_INPUT->SetToggleFullscreen(false); // Block Alt+Enter

        CreateCurrentFpsUiElement();
    }

    void apply_app_state(i32 frame_number, float time_step)
    {
        APP_STATE_MANAGER->Apply();
    }
};

DV_DEFINE_APPLICATION_MAIN(App);
