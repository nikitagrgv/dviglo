// Copyright (c) 2008-2023 the Urho3D project
// Copyright (c) 2022-2023 the Dviglo project
// License: MIT

#include "camera.h"
#include "graphics.h"
#include "renderer.h"
#include "render_path.h"
#include "view.h"
#include "../resource/resource_cache.h"
#include "../resource/xml_file.h"
#include "../scene/scene.h"

#include "../common/debug_new.h"

using namespace std;


namespace dviglo
{

Viewport::Viewport()
{
    SetRenderPath((RenderPath*)nullptr);
}

Viewport::Viewport(Scene* scene, Camera* camera, RenderPath* renderPath) :
    scene_(scene),
    camera_(camera)
{
    SetRenderPath(renderPath);
}

Viewport::Viewport(Scene* scene, Camera* camera, const IntRect& rect, RenderPath* renderPath) :   // NOLINT(modernize-pass-by-value)
    scene_(scene),
    camera_(camera),
    rect(rect)
{
    SetRenderPath(renderPath);
}

Viewport::~Viewport() = default;

void Viewport::SetScene(Scene* scene)
{
    scene_ = scene;
}

void Viewport::SetCamera(Camera* camera)
{
    camera_ = camera;
}

void Viewport::SetCullCamera(Camera* camera)
{
    cullCamera_ = camera;
}

void Viewport::SetRenderPath(RenderPath* renderPath)
{
    if (renderPath)
        renderPath_ = renderPath;
    else
    {
        if (!GParams::is_headless())
            renderPath_ = DV_RENDERER->GetDefaultRenderPath();
    }
}

bool Viewport::SetRenderPath(XmlFile* file)
{
    SharedPtr<RenderPath> newRenderPath(new RenderPath());
    if (newRenderPath->Load(file))
    {
        renderPath_ = newRenderPath;
        return true;
    }
    return false;
}

Scene* Viewport::GetScene() const
{
    return scene_;
}

Camera* Viewport::GetCamera() const
{
    return camera_;
}

Camera* Viewport::GetCullCamera() const
{
    return cullCamera_;
}

View* Viewport::GetView() const
{
    return view_.get();
}

RenderPath* Viewport::GetRenderPath() const
{
    return renderPath_;
}

Ray Viewport::GetScreenRay(int x, int y) const
{
    if (!camera_)
        return Ray();

    float screenX;
    float screenY;

    if (rect == IntRect::ZERO)
    {
        Graphics* graphics = DV_GRAPHICS;
        screenX = (float)x / (float)graphics->GetWidth();
        screenY = (float)y / (float)graphics->GetHeight();
    }
    else
    {
        screenX = float(x - rect.left_) / (float)rect.Width();
        screenY = float(y - rect.top_) / (float)rect.Height();
    }

    return camera_->GetScreenRay(screenX, screenY);
}

IntVector2 Viewport::world_to_screen_point(const Vector3& worldPos) const
{
    if (!camera_)
        return IntVector2::ZERO;

    Vector2 screenPoint = camera_->world_to_screen_point(worldPos);

    int x;
    int y;
    if (rect == IntRect::ZERO)
    {
        /// \todo This is incorrect if the viewport is used on a texture rendertarget instead of the backbuffer, as it may have different dimensions.
        Graphics* graphics = DV_GRAPHICS;
        x = (int)(screenPoint.x * graphics->GetWidth());
        y = (int)(screenPoint.y * graphics->GetHeight());
    }
    else
    {
        x = (int)(rect.left_ + screenPoint.x * rect.Width());
        y = (int)(rect.top_ + screenPoint.y * rect.Height());
    }

    return IntVector2(x, y);
}

Vector3 Viewport::screen_to_world_point(int x, int y, float depth) const
{
    if (!camera_)
        return Vector3::ZERO;

    float screenX;
    float screenY;

    if (rect == IntRect::ZERO)
    {
        /// \todo This is incorrect if the viewport is used on a texture rendertarget instead of the backbuffer, as it may have different dimensions.
        Graphics* graphics = DV_GRAPHICS;
        screenX = (float)x / (float)graphics->GetWidth();
        screenY = (float)y / (float)graphics->GetHeight();
    }
    else
    {
        screenX = float(x - rect.left_) / (float)rect.Width();
        screenY = float(y - rect.top_) / (float)rect.Height();
    }

    return camera_->screen_to_world_point(Vector3(screenX, screenY, depth));
}

void Viewport::allocate_view()
{
    view_ = make_unique<View>();
}

} // namespace dviglo
