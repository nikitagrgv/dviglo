#include <iostream>

#include "Draggable.h"
#include <dviglo/dviglo_all.h>

using namespace dviglo;

class Selection
{
public:
    Signal<> changed;

    void postUpdate() { refresh_nodes(); }

    void render(DebugRenderer* renderer) const
    {
        for (const auto& node : nodes_)
        {
            for (const auto & component : node->GetComponents())
            {
                auto drawable = DynamicCast<Drawable>(component);
                if (drawable)
                {
                    renderer->AddBoundingBox(drawable->GetWorldBoundingBox(), Color::GREEN, false);
                }
            }
        }
    }

    void add(const WeakPtr<Node>& node)
    {
        if (node.Expired())
        {
            return;
        }

        auto it = nodes_.Find(node);
        if (it == end(nodes_))
        {
            nodes_.Push(node);
            changed.emit();
        }
    }

    void remove(const WeakPtr<Node>& node)
    {
        if (node.Expired())
        {
            return;
        }

        auto it = nodes_.Find(node);
        if (it != end(nodes_))
        {
            nodes_.Erase(it);
            changed.emit();
        }
    }

    bool isSelected(const WeakPtr<Node>& node) const { return nodes_.Find(node) != end(nodes_); }

    void clear()
    {
        nodes_.Clear();
        changed.emit();
    }

    const Vector<WeakPtr<Node>> &getNodes() const { return nodes_; }

private:
    void refresh_nodes()
    {
        nodes_.Erase(std::remove_if(begin(nodes_), end(nodes_),
                                    [](const WeakPtr<Node>& node) { return node.Expired(); }),
                     end(nodes_));
    }

private:
    Vector<WeakPtr<Node>> nodes_;
};

class NodeParameters
{
public:
    NodeParameters(Selection& selection)
        : selection_(selection)
    {
        selection_.changed.connect(selection_changed_, [this]() { on_selection_changed(); });
        init_gui();
    }

    void update()
    {
        auto nodes = selection_.getNodes();
    }

private:
    void init_gui()
    {
        UI* ui = DV_UI;
        auto root = ui->GetRoot();

        window_ = new Draggable<Window>();
        root->AddChild(window_);
        window_->SetColor(Color(0.5, 0.6, 0.3, 0.5));
        window_->SetLayout(LM_VERTICAL, 6, IntRect(6, 6, 6, 6));
        window_->SetAlignment(HA_LEFT, VA_TOP);
        window_->SetStyleAuto();
//        window_->SetMinWidth(200);
//        window_->SetMinHeight(200);
    }

    void on_selection_changed()
    {
        refresh();
    }

    void refresh()
    {
        window_->RemoveAllChildren();

        for (const auto& node : selection_.getNodes())
        {
            add_node_info(node, window_);
        }
    }

    void add_node_info(Node* node, UiElement* parent)
    {
        auto ver_layout = new UiElement();
        parent->AddChild(ver_layout);
        ver_layout->SetStyleAuto();
        ver_layout->SetLayout(LM_VERTICAL, 4);

        {
            auto hor_layout = new UiElement();
            ver_layout->AddChild(hor_layout);
            hor_layout->SetStyleAuto();
            hor_layout->SetLayout(LM_HORIZONTAL, 4);

            auto label_name = new Text();
            hor_layout->AddChild(label_name);
            label_name->SetStyleAuto();
            label_name->SetText("Name:");

            auto name = new Text();
            hor_layout->AddChild(name);
            name->SetStyleAuto();
            name->SetText(String(node->GetName()));
        }
        {
            auto hor_layout = new UiElement();
            ver_layout->AddChild(hor_layout);
            hor_layout->SetStyleAuto();
            hor_layout->SetLayout(LM_HORIZONTAL, 4);

            auto label_id = new Text();
            hor_layout->AddChild(label_id);
            label_id->SetStyleAuto();
            label_id->SetText("Id:");

            auto id = new Text();
            hor_layout->AddChild(id);
            id->SetStyleAuto();
            id->SetText(String(node->GetID()));
        }
    }

private:
    WeakPtr<Window> window_;
    Selection& selection_;
    Slot<> selection_changed_;
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
        subscribe_to_event(E_MOUSEBUTTONUP, DV_HANDLER(App, on_mouse_release));
        subscribe_to_event(E_POSTUPDATE, DV_HANDLER(App, on_post_update));
        subscribe_to_event(E_POSTRENDERUPDATE, DV_HANDLER(App, on_post_render_update));


        init_world();
//        File saveFile(DV_FILE_SYSTEM->GetProgramDir() + "data1/out.xml", FILE_WRITE);
//        scene_->save_xml(saveFile);
        init_gui();

        node_parameters_ = std::make_unique<NodeParameters>(selection_);
    }

private:
    void init_world()
    {
        ResourceCache* cache = DV_RES_CACHE;
        Renderer* renderer = DV_RENDERER;

        scene_ = new Scene();
        octree_ = scene_->create_component<Octree>();
        debug_renderer_ = scene_->create_component<DebugRenderer>();

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

        for (int i = 0; i < 5; i++)
        {
            for (int j = 0; j < 5; j++)
            {
                for (int k = 0; k < 5; k++)
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
    }

    void on_mouse_release(StringHash /*event*/, VariantMap& data)
    {
        const int qualifiers = data[MouseButtonUp::P_QUALIFIERS].GetI32();
        const bool ctrl = qualifiers & Qualifier::QUAL_CTRL;
        const bool shift = qualifiers & Qualifier::QUAL_SHIFT;

        if (DV_INPUT->GetMouseButtonDown(MOUSEB_RIGHT))
        {
            return;
        }

        auto viewport = DV_RENDERER->GetViewport(0);
        auto mouse_pos = DV_INPUT->GetMousePosition();
        auto ray = viewport->GetScreenRay(mouse_pos.x, mouse_pos.y);

        if (data[MouseButtonUp::P_BUTTON] != MOUSEB_LEFT)
        {
            return;
        }
        Vector<RayQueryResult> result;
        Vector3 const origin = camera_node_->GetPosition();
        Vector3 const dir = camera_node_->GetWorldDirection();
        RayOctreeQuery q(result, ray, RAY_TRIANGLE, M_INFINITY,
                         dviglo::DrawableTypes::Geometry);
        octree_->RaycastSingle(q);

        if (q.result_.Empty() && !ctrl && !shift)
        {
            selection_.clear();
        }

        for (auto r : q.result_)
        {
            const WeakPtr<Node> node(r.node_);
            if (ctrl && !shift)
            {
                selection_.add(node);
            }
            if (shift && ! ctrl)
            {
                selection_.remove(node);
            }
            if (!shift && !ctrl)
            {
                selection_.clear();
                selection_.add(node);
            }
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

        if (input->GetKeyPress(KEY_F1))
        {
            render_debug_ = !render_debug_;
        }

        const float dt = data[Update::P_TIMESTEP].GetFloat();

        node_parameters_->update();

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

    void on_post_update(StringHash /*event*/, VariantMap& data) { selection_.postUpdate(); }

    void on_post_render_update(StringHash /*event*/, VariantMap& data)
    {
        if (render_debug_)
        {
            DV_RENDERER->draw_debug_geometry(false);
        }
        selection_.render(debug_renderer_);
    }

private:
    std::unique_ptr<NodeParameters> node_parameters_;
    Selection selection_;

    bool render_debug_{false};

    Vector<WeakPtr<Node>> cubes_;
    WeakPtr<Octree> octree_;
    WeakPtr<DebugRenderer> debug_renderer_;
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
