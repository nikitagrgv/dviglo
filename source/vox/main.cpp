#include <iostream>

#include "Draggable.h"
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
        subscribe_to_event(E_MOUSEBUTTONUP, DV_HANDLER(App, on_mouse_release));

        init_world();
//        File saveFile(DV_FILE_SYSTEM->GetProgramDir() + "data1/out.xml", FILE_WRITE);
//        scene_->save_xml(saveFile);
        init_gui();
    }

private:
    void init_world()
    {
        ResourceCache* cache = DV_RES_CACHE;
        Renderer* renderer = DV_RENDERER;

        scene_ = new Scene();
        octree_ = scene_->create_component<Octree>();

        camera_node_ = scene_->create_child();
        auto camera = camera_node_->create_component<Camera>();
        camera->SetFov(80);
        camera_node_->SetPosition({-0.1f, 0.2f, -5.f});

        auto viewport = new Viewport(scene_, camera);
        renderer->SetViewport(0, viewport);

        auto zone_node = scene_->create_child();
        auto zone = zone_node->create_component<Zone>();
        zone->SetBoundingBox(BoundingBox(Sphere(Vector3(), 400)));
        zone->SetAmbientColor(Color(0.3, 0.5, 0.8));

        auto tech = cache->GetResource<Technique>("techniques/diff.xml");
        auto mat = new Material();
        mat->SetTechnique(0, tech);
        auto texture = cache->GetResource<Texture2D>("vox/1.jpg");
        mat->SetTexture(TU_DIFFUSE, texture);

        for (int i = 0; i < 10; i++)
        {
            for (int j = 0; j < 10; j++)
            {
                for (int k = 0; k < 10; k++)
                {
                    auto box_node = scene_->create_child();
                    box_node->SetPosition({i * 2.f, j * 2.f, k * 2.f});
                    auto model = box_node->create_component<StaticModel>();
                    model->SetModel(cache->GetResource<Model>("models/pyramid.mdl"));
                    model->SetMaterial(mat);
                    cubes_.Push(WeakPtr(box_node));
                }
            }
        }

        {
            auto light_node = scene_->create_child();
            auto light = light_node->create_component<Light>();
            light->SetLightType(LIGHT_POINT);
            light_node->SetPosition({1, 1, 1});
            light->SetColor(Color::GREEN);
        }
        {
            auto light_node = scene_->create_child();
            auto light = light_node->create_component<Light>();
            light->SetLightType(LIGHT_DIRECTIONAL);
            light_node->SetDirection({-1, -1, -1});
            light->SetColor(Color(0.2,0.3,0.7));
        }

        auto skybox_node = scene_->create_child();
        auto skybox = skybox_node->create_component<Skybox>();
        skybox->SetModel(cache->GetResource<Model>("models/box.mdl"));
        skybox->SetMaterial(cache->GetResource<Material>("materials/skybox.xml"));
        auto cubemap = new TextureCube();
        cubemap->SetData((CubeMapFace)0, cache->GetResource<Image>("vox/1.jpg"));
        cubemap->SetData((CubeMapFace)1, cache->GetResource<Image>("vox/2.jpg"));
        cubemap->SetData((CubeMapFace)2, cache->GetResource<Image>("vox/3.jpg"));
        cubemap->SetData((CubeMapFace)3, cache->GetResource<Image>("vox/4.jpg"));
        cubemap->SetData((CubeMapFace)4, cache->GetResource<Image>("vox/5.jpg"));
        cubemap->SetData((CubeMapFace)5, cache->GetResource<Image>("vox/8.jpg"));
        skybox->GetMaterial()->SetTexture(TU_DIFFUSE, cubemap);
    }

    void init_gui()
    {
        UI* ui = DV_UI;
        auto root = ui->GetRoot();
        auto* style = DV_RES_CACHE->GetResource<XmlFile>("ui/default_style.xml");
        root->SetDefaultStyle(style);

//        auto window = new Draggable<Window>();
//        root->AddChild(window);
//        window->SetColor(Color(0.5, 0.6, 0.3, 0.5));
//        window->SetMinWidth(384);
//        window->SetMinHeight(384);
//        window->SetLayout(LM_VERTICAL, 6, IntRect(6, 6, 6, 6));
//        window->SetAlignment(HA_CENTER, VA_CENTER);
//        window->SetStyleAuto();
    }

    void on_mouse_release(StringHash /*event*/, VariantMap& data)
    {
        if (data[MouseButtonUp::P_BUTTON] != MOUSEB_LEFT)
        {
            return;
        }
        Vector<RayQueryResult> result;
        Vector3 const origin = camera_node_->GetPosition();
        Vector3 const dir = camera_node_->GetWorldDirection();
        RayOctreeQuery q(result, Ray(origin, dir), RAY_TRIANGLE, M_INFINITY,
                         dviglo::DrawableTypes::Geometry);
        octree_->RaycastSingle(q);

        for (auto r : q.result_)
        {
            Log::WriteFormat(1, "NODE: %d", r.node_->GetID());
            r.node_->Remove();
        }
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
            rot = Quaternion(0, rot_yaw, 0) * rot * Quaternion(rot_pitch, 0, 0);
            camera_node_->SetWorldRotation(rot);
        }
        // random rotate
        cubes_.Erase(std::remove_if(begin(cubes_), end(cubes_),
                                    [](const WeakPtr<Node>& node) { return node.Expired(); }),
                     end(cubes_));
        for (int i = 0; i < cubes_.Size(); i++)
        {
            auto& cube = cubes_[i];
            auto p_rot = cube->GetRotation();

            float r1 = Random(-50.f, 50.0f) * dt;
            float r2 = Random(-50.f, 50.0f) * dt;
            float r3 = Random(-50.f, 50.0f) * dt;

            Quaternion rot = p_rot * Quaternion(r1, r2, r3);

            cube->SetRotation(rot);
        }
    }

private:
    Vector<WeakPtr<Node>> cubes_;
    WeakPtr<Octree> octree_;
    WeakPtr<Scene> scene_;
    WeakPtr<Node> camera_node_;
};

int main(int argc, char** argv)
{
    dviglo::ParseArguments(argc, argv);
    const Context context;
    App app;
    return app.Run();
}
