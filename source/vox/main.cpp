#include <iostream>

// #include "dviglo/resource/resource_cache.h"
// #include "dviglo/ui/check_box.h"
// #include "dviglo/ui/list_view.h"
// #include "dviglo/ui/scroll_view.h"
// #include "dviglo/ui/slider.h"
// #include "dviglo/ui/sprite.h"
// #include <dviglo/core/core_events.h>
// #include <dviglo/engine/application.h>
// #include <dviglo/engine/engine_defs.h>
// #include <dviglo/graphics/graphics.h>
// #include <dviglo/graphics_api/texture_2d.h>
// #include <dviglo/input/input.h>
// #include <dviglo/io/log.h>
// #include <dviglo/math/math_defs.h>
// #include <dviglo/ui/button.h>
// #include <dviglo/ui/ui.h>
// #include <dviglo/ui/ui_element.h>
// #include <dviglo/ui/ui_events.h>
// #include <dviglo/math/random.h>
#include <dviglo/dviglo_all.h>

using namespace dviglo;

class Paintable
{
public:
    virtual ~Paintable() = default;
    virtual int getWidth() const = 0;
    virtual int getHeight() const = 0;
    virtual void set(int x, int y, const Color& color) = 0;
};

class Painter
{
public:
    virtual ~Painter() = default;
    Painter(std::unique_ptr<Paintable> paintable)
        : paintable_(std::move(paintable))
    {
    }

    int getWidth() const
    {
        assert(paintable_);
        return paintable_->getWidth();
    }

    int getHeight() const
    {
        assert(paintable_);
        return paintable_->getHeight();
    }

    void set(int x, int y, const Color& color)
    {
        assert(paintable_);
        paintable_->set(x, y, color);
    }

    virtual void draw() = 0;

private:
    std::unique_ptr<Paintable> paintable_;
};

class ImagePainter : public Painter
{
public:
    ImagePainter(std::unique_ptr<Paintable> paintable)
        : Painter(std::move(paintable))
    {
    }

    Vector2 randomVector(Vector2 pos)
    {
        constexpr float seed = 123;
        return Vector2{StableRandom(Vector3{pos.x, pos.y, 0}),
                       StableRandom(Vector3{pos.x, pos.y, seed})};
    }

    Vector2 randomVector(IntVector2 pos)
    {
        constexpr float seed = 123;
        return Vector2{StableRandom(Vector3{(float)pos.x, (float)pos.y, 0}),
                       StableRandom(Vector3{(float)pos.x, (float)pos.y, seed})};
    }

    Vector2 randomDir(Vector2 pos)
    {
        constexpr float seed = 123;
        return Vector2{StableRandom(Vector3{pos.x, pos.y, 0}) - 0.5f,
                       StableRandom(Vector3{pos.x, pos.y, seed}) - 0.5f}
            .normalized();
    }

    Vector2 randomDir(IntVector2 pos)
    {
        constexpr float seed = 123;
        return Vector2{StableRandom(Vector3{(float)pos.x, (float)pos.y, 0}) - 0.5f,
                       StableRandom(Vector3{(float)pos.x, (float)pos.y, seed}) - 0.5f}
            .normalized();
    }

    Vector2 randomVectorAligned(Vector2 pos, float align)
    {
        Vector2 aligned;
        aligned.x = Floor(pos.x / align) * align;
        aligned.y = Floor(pos.y / align) * align;
        return randomVector(aligned);
    }

    float interpolate(float a, float b, float w) { return (b - a) * w + a; }

    float gridGradient(Vector2 pos, float align)
    {
        const float half_align = align / 2;
        const int x0 = FloorToInt(pos.x / align);
        const int y0 = FloorToInt(pos.y / align);
        const int x1 = x0 + 1;
        const int y1 = y0 + 1;

        const Vector2 weight = (pos - Vector2(x0, y0) * align) / align;

        const Vector2 loc_pos = pos - Vector2(x0, y0) * align - Vector2(half_align, half_align);
//        const Vector2 loc_dir = loc_pos.normalized();

        const Vector2 loc_dir00 = (loc_pos - Vector2(-half_align, -half_align)).normalized();
        const Vector2 loc_dir10 = (loc_pos - Vector2(+half_align, -half_align)).normalized();
        const Vector2 loc_dir01 = (loc_pos - Vector2(-half_align, +half_align)).normalized();
        const Vector2 loc_dir11 = (loc_pos - Vector2(+half_align, +half_align)).normalized();

        const Vector2 rand_dir00 = randomDir(IntVector2{x0, y0});
        const Vector2 rand_dir10 = randomDir(IntVector2{x1, y0});
        const Vector2 rand_dir01 = randomDir(IntVector2{x0, y1});
        const Vector2 rand_dir11 = randomDir(IntVector2{x1, y1});

        const float r00 = rand_dir00.DotProduct(loc_dir00);
        const float r10 = rand_dir10.DotProduct(loc_dir10);
        const float r01 = rand_dir01.DotProduct(loc_dir01);
        const float r11 = rand_dir11.DotProduct(loc_dir11);

        return interpolate(interpolate(r00, r10, weight.x), interpolate(r01, r11, weight.x),
                           weight.y);

        //        const Vector2 dir = (randomVector(cell) - Vector2(0.5f, 0.5f)).normalized();

//        IntVector2 cell;
//        cell.x = (int)(Floor(pos.x / align) * align);
//        cell.y = (int)(Floor(pos.y / align) * align);

//        Vector2 loc_pos;
//        loc_pos.x = Fract(pos.x / align) * align - align/2;
//        loc_pos.y = Fract(pos.y / align) * align - align/2;

//        const Vector2 dir = (randomVector(cell) - Vector2(0.5f, 0.5f)).normalized();
//        return dir.DotProduct(loc_pos.normalized());
    }

    void draw() override
    {
        const int w = getWidth();
        const int h = getHeight();

        for (int i = 0; i < h; i++)
        {
            for (int j = 0; j < w; j++)
            {
                const float x = static_cast<float>(j) - offset.x * 100;
                const float y = static_cast<float>(i) - offset.y * 100;
                const Vector2 xy{x, y};
                const auto col = [this, i, j](float r, float g, float b) { set(j, i, {r, g, b}); };

//                float r{};
//                float g{};
//                float b{};
//
//                float intensity = gridGradient(xy, 10);
//                if (intensity > 0)
//                {
//                    r = intensity;
//                }
//                else
//                {
//                    b = -intensity;
//                }
//                col(r, 0, b);

                float intensity = (gridGradient(xy, 10) + 1) * 0.5f;
                col(intensity, intensity, intensity);
            }
        }

        for (int i = 0; i < Min(4, h); i++)
        {
            for (int j = 0; j < w; j++)
            {
                const float r = (j % 2 == 0) ? 0 : 1;
                set(j, i, {r, r, r});
            }
        }
    }

    Vector2 offset{0, 0};
};

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
        texture->SetSize(160, 160, Graphics::GetRGBFormat(), TEXTURE_DYNAMIC, 0);

        image_ = texture->GetImage();
        for (int i = 0; i < image_->GetWidth(); i++)
        {
            for (int j = 0; j < image_->GetHeight(); j++)
            {
                image_->SetPixel(i, j, Color::GRAY);
            }
        }

        class PaintableImage : public Paintable
        {
        public:
            PaintableImage(WeakPtr<Image> image)
                : image_(std::move(image))
            {
            }

            int getWidth() const override { return image_->GetWidth(); }
            int getHeight() const override { return image_->GetHeight(); }
            void set(int x, int y, const dviglo::Color& color) override
            {
                image_->SetPixel(x, y, color);
            }

        private:
            WeakPtr<Image> image_;
        };
        std::unique_ptr<Paintable> paintable = std::make_unique<PaintableImage>(image_);

        painter_ = std::make_unique<ImagePainter>(std::move(paintable));

        sprite_ = new ImageSprite();
        container->AddChild(sprite_);
        sprite_->SetStyleAuto();
        sprite_->setImage(image_);
        sprite_->setScale(4);

        {
            slider_x_ = new Slider();
            container->AddChild(slider_x_);
            slider_x_->SetStyleAuto();
            slider_x_->SetRange(2);
            slider_x_->SetValue(1);
            slider_x_->SetMinHeight(30);
        }
        {
            slider_y_ = new Slider();
            container->AddChild(slider_y_);
            slider_y_->SetStyleAuto();
            slider_y_->SetRange(2);
            slider_y_->SetValue(1);
            slider_y_->SetMinHeight(30);
        }
        checkbox_ = new CheckBox();
        container->AddChild(checkbox_);
        checkbox_->SetChecked(true);
        checkbox_->SetStyleAuto();
        checkbox_->SetFixedWidth(40);
        checkbox_->SetFixedHeight(40);
    }

private:
    void on_update(StringHash /*event*/, VariantMap& data)
    {
        const float dt = data[Update::P_TIMESTEP].GetFloat();
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
        painter_->offset.x = slider_x_->GetValue() - slider_x_->GetRange() / 2;
        painter_->offset.y = slider_y_->GetValue() - slider_y_->GetRange() / 2;
        painter_->draw();
    }

private:
    std::unique_ptr<ImagePainter> painter_;

    SharedPtr<Image> image_;
    WeakPtr<CheckBox> checkbox_;
    WeakPtr<ImageSprite> sprite_;
    WeakPtr<Slider> slider_x_;
    WeakPtr<Slider> slider_y_;
};

int main(int argc, char** argv)
{
    dviglo::ParseArguments(argc, argv);
    const Context context;
    App app;
    return app.Run();
}
