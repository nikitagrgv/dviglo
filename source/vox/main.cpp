#include <iostream>

#include "dviglo/resource/resource_cache.h"
#include "dviglo/ui/check_box.h"
#include "dviglo/ui/list_view.h"
#include "dviglo/ui/scroll_view.h"
#include "dviglo/ui/slider.h"
#include "dviglo/ui/sprite.h"
#include <dviglo/core/core_events.h>
#include <dviglo/engine/application.h>
#include <dviglo/engine/engine_defs.h>
#include <dviglo/graphics/graphics.h>
#include <dviglo/graphics_api/texture_2d.h>
#include <dviglo/input/input.h>
#include <dviglo/io/log.h>
#include <dviglo/math/math_defs.h>
#include <dviglo/ui/button.h>
#include <dviglo/ui/ui.h>
#include <dviglo/ui/ui_element.h>
#include <dviglo/ui/ui_events.h>

using namespace dviglo;

class ImageSprite : public Sprite
{
public:
    void setImage(const SharedPtr<Image>& image)
    {
        auto texture = new Texture2D();
        texture->SetFilterMode(FILTER_NEAREST);
        texture->SetData(image);
        SetTexture(texture);
        SetFixedWidth(static_cast<int>(static_cast<float>(image->GetWidth()) * scale_));
        SetFixedHeight(static_cast<int>(static_cast<float>(image->GetHeight()) * scale_));
    }

    void setScale(float scale) { scale_ = scale; }

private:
    float scale_ = 1;
};

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

        auto* scroll = new ListView();
        DV_UI->GetRoot()->AddChild(scroll);
        scroll->SetStyleAuto();
        scroll->SetMinWidth(800);
        scroll->SetMinHeight(800);

        auto* container = new UiElement();
        container->SetLayout(LM_VERTICAL, 5, {3, 3, 3, 3});
        container->SetStyleAuto();
        scroll->SetContentElement(container);

        auto* texture = new Texture2D();
        texture->SetSize(200, 200, Graphics::GetRGBFormat(), TEXTURE_DYNAMIC, 0);

        image_ = texture->GetImage();
        for (int i = 0; i < image_->GetWidth(); i++)
        {
            for (int j = 0; j < image_->GetHeight(); j++)
            {
                image_->SetPixel(i, j, Color::GRAY);
            }
        }

        sprite_ = new ImageSprite();
        container->AddChild(sprite_);
        sprite_->SetStyleAuto();
        sprite_->setImage(image_);
        sprite_->setScale(3);

        slider_ = new Slider();
        container->AddChild(slider_);
        slider_->SetStyleAuto();
        slider_->SetRange(60);
        slider_->SetValue(1);
        slider_->SetMinHeight(20);

        auto* button = new Button();
        container->AddChild(button);
        button->SetMinHeight(24);
        button->SetMinWidth(24);
        button->SetStyleAuto();

        checkbox_ = new CheckBox();
        container->AddChild(checkbox_);
        checkbox_->SetChecked(true);
        checkbox_->SetStyleAuto();
    }

private:
    void on_update(StringHash /*event*/, VariantMap& data)
    {
        const float dt = data[Update::P_TIMESTEP].GetFloat();
        factor_ = (slider_->GetValue());
        sprite_->setImage(image_);
        update_image();
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

    void update_image()
    {
        if (!checkbox_->IsChecked())
        {
            return;
        }

        const int width = image_->GetWidth();
        const int height = image_->GetHeight();
        for (int i = 0; i < width; i++)
        {
            for (int j = 0; j < height; j++)
            {
                const auto fi = static_cast<float>(i - width / 2) / factor_;
                const auto fj = static_cast<float>(j - height / 2) / factor_;

                float r = Fract(fi * Tan(fj * 2.0f + 0.3f));
                float g = Fract(fj * Tan(fi * 3.0f + 0.2f));
                float a = Fract(fi * fj * Sin(fi - fj * 4.0f + 0.5f));
                float b = Fract(Sin(fi) * fj);

                image_->SetPixel(i, j, Color(r, g, b, a));
            }
        }
    }

private:
    float factor_ = 1;
    SharedPtr<Image> image_;
    WeakPtr<CheckBox> checkbox_;
    WeakPtr<ImageSprite> sprite_;
    WeakPtr<Slider> slider_;
};

int main(int argc, char** argv)
{
    dviglo::ParseArguments(argc, argv);
    const Context context;
    App app;
    return app.Run();
}
