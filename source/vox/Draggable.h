#pragma once

#include <dviglo/math/math_defs.h>
#include <dviglo/ui/ui_element.h>

template <class T = dviglo::UiElement> class Draggable : public T
{
public:
    void OnDragBegin(const dviglo::IntVector2& pos, const dviglo::IntVector2& screen_pos,
                     dviglo::MouseButtonFlags buttons, dviglo::QualifierFlags qualifiers,
                     dviglo::Cursor* cursor) override
    {
        T::OnDragBegin(pos, screen_pos, buttons, qualifiers, cursor);

        drag_start_pos_ = screen_pos;
        widget_start_pos_ = T::GetPosition();
    }

    void OnDragMove(const dviglo::IntVector2& pos, const dviglo::IntVector2& screen_pos,
                    const dviglo::IntVector2& delta, dviglo::MouseButtonFlags buttons,
                    dviglo::QualifierFlags qualifiers, dviglo::Cursor* cursor) override
    {
        T::OnDragMove(pos, screen_pos, delta, buttons, qualifiers, cursor);

        const dviglo::IntVector2 moving = screen_pos - drag_start_pos_;
        T::SetPosition(widget_start_pos_ + moving);
    }
    void OnDragCancel(const dviglo::IntVector2& pos, const dviglo::IntVector2& screen_pos,
                      dviglo::MouseButtonFlags drag_buttons,
                      dviglo::MouseButtonFlags cancel_buttons, dviglo::Cursor* cursor) override
    {
        T::OnDragCancel(pos, screen_pos, drag_buttons, cancel_buttons, cursor);

        T::SetPosition(widget_start_pos_);
    }

private:
    dviglo::IntVector2 widget_start_pos_;
    dviglo::IntVector2 drag_start_pos_;
};
