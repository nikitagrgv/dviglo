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

        init_world();
    }

private:
    void init_world()
    {
        auto cache = DV_RES_CACHE;
        auto renderer = DV_RENDERER;

        auto scene = new Scene();
        scene->create_component<Octree>();

        camera_node_ = scene->create_child();
        auto camera = camera_node_->create_component<Camera>();
        camera_node_->SetPosition({-0.1f, 0.2f, -5.f});

        auto viewport = new Viewport(scene, camera);
        renderer->SetViewport(0, viewport);

        for (int i = 0; i < 10; i++)
        {
            for (int j = 0; j < 10; j++)
            {
                for (int k = 0; k < 10; k++)
                {
                    auto box_node = scene->create_child();
                    box_node->SetPosition({i * 2.f, j * 2.f, k * 2.f});
                    auto model = box_node->create_component<StaticModel>();
                    model->SetModel(cache->GetResource<Model>("models/box.mdl"));
                    cubes_.Push(WeakPtr(box_node));
                }
            }
        }

        auto light_node = scene->create_child();
        auto light = light_node->create_component<Light>();
        light->SetLightType(LIGHT_POINT);
        light_node->SetPosition({1, 1, 1});
        light->SetColor(Color::GREEN);
    }

    void on_update(StringHash /*event*/, VariantMap& data)
    {
        auto input = DV_INPUT;
        auto engine = DV_ENGINE;

        if (input->GetKeyPress(KEY_ESCAPE))
        {
            engine->Exit();
        }

        const float dt = data[Update::P_TIMESTEP].GetFloat();

        float move_speed = 3.f;
        if (input->GetKeyDown(KEY_SHIFT))
        {
            move_speed *= 2;
        }


        Vector3 dir;
        if (input->GetKeyDown(KEY_D))
        {
            dir.x += move_speed;
        }
        if (input->GetKeyDown(KEY_A))
        {
            dir.x -= move_speed;
        }
        if (input->GetKeyDown(KEY_W))
        {
            dir.z += move_speed;
        }
        if (input->GetKeyDown(KEY_S))
        {
            dir.z -= move_speed;
        }
        if (input->GetKeyDown(KEY_E))
        {
            dir.y += move_speed;
        }
        if (input->GetKeyDown(KEY_Q))
        {
            dir.y -= move_speed;
        }
        const Matrix3x4 rot_matr(Vector3{0, 0, 0}, camera_node_->GetWorldRotation(), 1.f);
        dir = rot_matr * dir;

        camera_node_->SetPosition(camera_node_->GetPosition() + dir * dt);

        bool const looks_around = input->GetMouseButtonDown(MOUSEB_RIGHT);
        DV_INPUT->SetMouseVisible(!looks_around);

        if (looks_around)
        {
            const float rot_speed = 15.0f;

            float const rot_pitch = input->GetMouseMoveY() * rot_speed * dt;
            float const rot_yaw = input->GetMouseMoveX() * rot_speed * dt;
            Quaternion rot = camera_node_->GetWorldRotation();
            rot =  Quaternion(0, rot_yaw, 0) * rot * Quaternion(rot_pitch, 0, 0);
            camera_node_->SetWorldRotation(rot);
        }
        // random rotate
        for (const auto& cube : cubes_)
        {
            cube->RotateAround(
                Vector3{}, Quaternion(dt * 5.f, Vector3{Random(1.0f), Random(1.0f), Random(1.0f)}),
                dviglo::TransformSpace::Local);
        }
    }

private:
    Vector<WeakPtr<Node>> cubes_;
    WeakPtr<Node> camera_node_;
};

int main(int argc, char** argv)
{
    dviglo::ParseArguments(argc, argv);
    const Context context;
    App app;
    return app.Run();
}
