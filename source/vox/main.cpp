#include <iostream>

#include "dviglo/resource/resource_cache.h"
#include "dviglo/ui/check_box.h"
#include "dviglo/ui/list_view.h"
#include "dviglo/ui/scroll_view.h"
#include <dviglo/core/core_events.h>
#include <dviglo/engine/application.h>
#include <dviglo/engine/engine_defs.h>
#include <dviglo/input/input.h>
#include <dviglo/io/log.h>
#include <dviglo/ui/button.h>
#include <dviglo/ui/ui.h>
#include <dviglo/ui/ui_element.h>
#include <dviglo/ui/ui_events.h>

using namespace dviglo;

class App : public Application
{
public:
    void Setup() override
    {
        engineParameters_[EP_FULL_SCREEN] = false;
        engineParameters_[EP_HEADLESS] = false;
        engineParameters_[EP_WINDOW_RESIZABLE] = true;
    }
    void Start() override
    {
        DV_INPUT->SetMouseVisible(true);
        subscribe_to_event(E_UPDATE, DV_HANDLER(App, on_update));
        subscribe_to_event(E_KEYDOWN, DV_HANDLER(App, on_key_down));


        auto* style = DV_RES_CACHE->GetResource<XmlFile>("ui/default_style.xml");
        DV_UI->GetRoot()->SetDefaultStyle(style);

        auto scroll = new ListView();
        DV_UI->GetRoot()->AddChild(scroll);
        scroll->SetStyleAuto();
        scroll->SetMinWidth(400);
        scroll->SetMinHeight(400);

        auto container = new UiElement();
        container->SetLayout(LM_VERTICAL, 5, {3, 3, 3, 3});
        container->SetStyleAuto();
        scroll->SetContentElement(container);

        for (int i = 0; i < 150; i++)
        {
            auto* button = new Button();
            container->AddChild(button);
            button->SetMinHeight(24);
            button->SetMinWidth(24);
            button->SetStyleAuto();
        }

        auto* checkbox = new CheckBox();
        container->AddChild(checkbox);
        checkbox->SetStyleAuto();
    }

private:
    void on_update(StringHash /*event*/, VariantMap& data)
    {
        const float dt = data[Update::P_TIMESTEP].GetFloat();
    }

    void on_key_down(StringHash /*event*/, VariantMap& data)
    {
        const i32 key = data[KeyDown::P_KEY].GetI32();
        if (key == KEY_ESCAPE)
        {
            DV_ENGINE->Exit();
        }
        if (key == KEY_SPACE)
        {

        }
    }
};

int main(int argc, char** argv)
{
    dviglo::ParseArguments(argc, argv);
    const Context context;
    App app;
    return app.Run();
}
