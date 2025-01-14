// Copyright (c) 2008-2023 the Urho3D project
// Copyright (c) 2022-2023 the Dviglo project
// License: MIT

#include "sprite_batch_base.h"

#include "camera.h"
#include "graphics.h"

namespace dviglo
{

void SpriteBatchBase::add_triangle()
{
    // Рендерили четырёхугольники, а теперь нужно рендерить треугольники
    if (q_num_vertices_ > 0)
        flush();

    memcpy(t_vertices_ + t_num_vertices_, &triangle_, sizeof(triangle_));
    t_num_vertices_ += vertices_per_triangle_;

    // Если после добавления вершин мы заполнили массив до предела, то рендерим порцию
    if (t_num_vertices_ == max_triangles_in_portion_ * vertices_per_triangle_)
        flush();
}

void SpriteBatchBase::set_shape_color(u32 color)
{
    triangle_.v0.color = color;
    triangle_.v1.color = color;
    triangle_.v2.color = color;
}

void SpriteBatchBase::set_shape_color(const Color& color)
{
    set_shape_color(color.ToU32());
}

void SpriteBatchBase::add_quad()
{
    // Рендерили треугольники, а теперь нужно рендерить четырехугольники
    if (t_num_vertices_ > 0)
        flush();

    if (quad_.texture != q_current_texture_ || quad_.vs != q_current_vs_ || quad_.ps != q_current_ps_)
    {
        flush();

        q_current_vs_ = quad_.vs;
        q_current_ps_ = quad_.ps;
        q_current_texture_ = quad_.texture;
    }

    memcpy(q_vertices_ + q_num_vertices_, &(quad_.v0), sizeof(QVertex) * vertices_per_quad_);
    q_num_vertices_ += vertices_per_quad_;

    // Если после добавления вершин мы заполнили массив до предела, то рендерим порцию
    if (q_num_vertices_ == max_quads_in_portion_ * vertices_per_quad_)
        flush();
}

IntRect SpriteBatchBase::get_viewport_rect()
{
    Graphics* graphics = DV_GRAPHICS;

    if (!is_virtual_screen_used())
        return IntRect(0, 0, graphics->GetWidth(), graphics->GetHeight());

    float real_aspect = (float)graphics->GetWidth() / graphics->GetHeight();
    float virtual_aspect = (float)virtual_screen_size.x / virtual_screen_size.y;

    float virtual_screen_scale;
    if (real_aspect > virtual_aspect)
    {
        // Окно шире, чем надо. Будут пустые полосы по бокам
        virtual_screen_scale = (float)graphics->GetHeight() / virtual_screen_size.y;
    }
    else
    {
        // Высота окна больше, чем надо. Будут пустые полосы сверху и снизу
        virtual_screen_scale = (float)graphics->GetWidth() / virtual_screen_size.x;
    }

    i32 viewport_width = (i32)(virtual_screen_size.x * virtual_screen_scale);
    i32 viewport_height = (i32)(virtual_screen_size.y * virtual_screen_scale);

    // Центрируем вьюпорт
    i32 viewport_x = (graphics->GetWidth() - viewport_width) / 2;
    i32 viewport_y = (graphics->GetHeight() - viewport_height) / 2;

    return IntRect(viewport_x, viewport_y, viewport_width + viewport_x, viewport_height + viewport_y);
}

Vector2 SpriteBatchBase::to_virtual_pos(const Vector2& real_pos)
{
    if (!is_virtual_screen_used())
        return real_pos;

    IntRect viewport_rect = get_viewport_rect();
    float factor = (float)virtual_screen_size.x / viewport_rect.Width();

    float virtual_x = (real_pos.x - viewport_rect.left_) * factor;
    float virtual_y = (real_pos.y - viewport_rect.top_) * factor;

    return Vector2(virtual_x, virtual_y);
}

void SpriteBatchBase::update_view_proj_matrix()
{
    Graphics* graphics = DV_GRAPHICS;

    if (camera)
    {
        Matrix4 matrix = camera->GetGPUProjection() * camera->GetView();
        graphics->SetShaderParameter(VSP_VIEWPROJ, matrix);
        return;
    }

    i32 width;
    i32 height;

    if (is_virtual_screen_used())
    {
        width = virtual_screen_size.x;
        height = virtual_screen_size.y;
    }
    else
    {
        width = graphics->GetWidth();
        height = graphics->GetHeight();
    }

    float pixel_width = 2.f / width; // Двойка, так как длина отрезка [-1, 1] равна двум
    float pixel_height = 2.f / height;

    Matrix4 matrix = Matrix4(pixel_width,  0.f,           0.f, -1.f,
                             0.f,         -pixel_height,  0.f,  1.f,
                             0.f,          0.f,           1.f,  0.f,
                             0.f,          0.f,           0.f,  1.f);

    graphics->SetShaderParameter(VSP_VIEWPROJ, matrix);
}

using GpuIndex16 = u16;

SpriteBatchBase::SpriteBatchBase()
{
    q_index_buffer_.reset(new IndexBuffer());
    q_index_buffer_->SetShadowed(true);

    // Индексный буфер всегда содержит набор четырёхугольников, поэтому его можно сразу заполнить
    q_index_buffer_->SetSize(max_quads_in_portion_ * indices_per_quad_, false);
    GpuIndex16* buffer = (GpuIndex16*)q_index_buffer_->Lock(0, q_index_buffer_->GetIndexCount());
    for (i32 i = 0; i < max_quads_in_portion_; i++)
    {
        // Первый треугольник четырёхугольника
        buffer[i * indices_per_quad_ + 0] = i * vertices_per_quad_ + 0;
        buffer[i * indices_per_quad_ + 1] = i * vertices_per_quad_ + 1;
        buffer[i * indices_per_quad_ + 2] = i * vertices_per_quad_ + 2;

        // Второй треугольник
        buffer[i * indices_per_quad_ + 3] = i * vertices_per_quad_ + 2;
        buffer[i * indices_per_quad_ + 4] = i * vertices_per_quad_ + 3;
        buffer[i * indices_per_quad_ + 5] = i * vertices_per_quad_ + 0;
    }
    q_index_buffer_->Unlock();

    q_vertex_buffer_.reset(new VertexBuffer());
    q_vertex_buffer_->SetSize(max_quads_in_portion_ * vertices_per_quad_, VertexElements::Position | VertexElements::Color | VertexElements::TexCoord1, true);

    Graphics* graphics = DV_GRAPHICS;

    t_vertex_buffer_.reset(new VertexBuffer());
    t_vertex_buffer_->SetSize(max_triangles_in_portion_ * vertices_per_triangle_, VertexElements::Position | VertexElements::Color, true);
    t_vertex_shader_ = graphics->GetShader(VS, "triangle_batch");
    t_pixel_shader_ = graphics->GetShader(PS, "triangle_batch");
    set_shape_color(Color::WHITE);
}

void SpriteBatchBase::flush()
{
    if (t_num_vertices_ > 0)
    {
        Graphics* graphics = DV_GRAPHICS;

        graphics->ResetRenderTargets();
        graphics->ClearParameterSources();
        graphics->SetCullMode(CULL_NONE);
        graphics->SetDepthWrite(false);
        graphics->SetStencilTest(false);
        graphics->SetScissorTest(false);
        graphics->SetColorWrite(true);
        graphics->SetDepthTest(compare_mode);
        graphics->SetBlendMode(blend_mode);
        graphics->SetViewport(get_viewport_rect());

        graphics->SetIndexBuffer(nullptr);
        graphics->SetVertexBuffer(t_vertex_buffer_.get());
        graphics->SetTexture(0, nullptr);

        // Параметры шейдеров нужно задавать после указания шейдеров
        graphics->SetShaders(t_vertex_shader_, t_pixel_shader_);
        graphics->SetShaderParameter(VSP_MODEL, Matrix3x4::IDENTITY);
        update_view_proj_matrix();

        // Копируем накопленную геометрию в память видеокарты
        TVertex* buffer = (TVertex*)t_vertex_buffer_->Lock(0, t_num_vertices_, true);
        memcpy(buffer, t_vertices_, t_num_vertices_ * sizeof(TVertex));
        t_vertex_buffer_->Unlock();

        // И отрисовываем её
        graphics->Draw(TRIANGLE_LIST, 0, t_num_vertices_);

        // Начинаем новую порцию
        t_num_vertices_ = 0;
    }

    else if (q_num_vertices_ > 0)
    {
        Graphics* graphics = DV_GRAPHICS;

        graphics->ResetRenderTargets();
        graphics->ClearParameterSources();
        graphics->SetCullMode(CULL_NONE);
        graphics->SetDepthWrite(false);
        graphics->SetStencilTest(false);
        graphics->SetScissorTest(false);
        graphics->SetColorWrite(true);
        graphics->SetDepthTest(compare_mode);
        graphics->SetBlendMode(blend_mode);
        graphics->SetViewport(get_viewport_rect());

        graphics->SetIndexBuffer(q_index_buffer_.get());
        graphics->SetVertexBuffer(q_vertex_buffer_.get());
        graphics->SetTexture(0, q_current_texture_);

        // Параметры шейдеров нужно задавать после указания шейдеров
        graphics->SetShaders(q_current_vs_, q_current_ps_);
        graphics->SetShaderParameter(VSP_MODEL, Matrix3x4::IDENTITY);
        update_view_proj_matrix();
        // Мы используем только цвета вершин. Но это значение требует шейдер Basic
        graphics->SetShaderParameter(PSP_MATDIFFCOLOR, Color::WHITE);

        // Копируем накопленную геометрию в память видеокарты
        QVertex* buffer = (QVertex*)q_vertex_buffer_->Lock(0, q_num_vertices_, true);
        memcpy(buffer, q_vertices_, q_num_vertices_ * sizeof(QVertex));
        q_vertex_buffer_->Unlock();

        // И отрисовываем её
        i32 num_quads = q_num_vertices_ / vertices_per_quad_;
        graphics->Draw(TRIANGLE_LIST, 0, num_quads * indices_per_quad_, 0, q_num_vertices_);

        // Начинаем новую порцию
        q_num_vertices_ = 0;
    }
}

} // namespace dviglo
