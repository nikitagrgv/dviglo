// Copyright (c) 2008-2023 the Urho3D project
// Copyright (c) 2022-2023 the Dviglo project
// License: MIT

#include "../core/context.h"
#include "../input/input_events.h"
#include "../io/log.h"
#include "progress_bar.h"
#include "ui_events.h"

namespace dviglo
{

extern const char* orientations[];
extern const char* UI_CATEGORY;

ProgressBar::ProgressBar() :
        orientation_(O_HORIZONTAL),
        loadingPercentStyle_("Text"),
        range_(1.0f),
        value_(0.0f),
        showPercentText_(true)
{
    SetEnabled(false);
    SetEditable(false);
    SetFocus(false);
    knob_ = create_child<BorderImage>("S_Knob");
    knob_->SetInternal(true);

    loadingText_ = create_child<Text>("S_Text");
    loadingText_->SetInternal(true);

    UpdateProgressBar();
}

ProgressBar::~ProgressBar() = default;

void ProgressBar::register_object()
{
    DV_CONTEXT->RegisterFactory<ProgressBar>(UI_CATEGORY);

    DV_COPY_BASE_ATTRIBUTES(BorderImage);
    DV_UPDATE_ATTRIBUTE_DEFAULT_VALUE("Is Enabled", true);
    DV_ENUM_ACCESSOR_ATTRIBUTE("Orientation", GetOrientation, SetOrientation, orientations, O_HORIZONTAL, AM_FILE);
    DV_ACCESSOR_ATTRIBUTE("Range", GetRange, SetRange, 1.0f, AM_FILE);
    DV_ACCESSOR_ATTRIBUTE("Value", GetValue, SetValue, 0.0f, AM_FILE);
    DV_ACCESSOR_ATTRIBUTE("Show Percent Text", GetShowPercentText, SetShowPercentText, true, AM_FILE);
}

void ProgressBar::OnResize(const IntVector2& /*newSize*/, const IntVector2& /*delta*/)
{
    UpdateProgressBar();
}

void ProgressBar::SetOrientation(Orientation orientation)
{
    orientation_ = orientation;
    UpdateProgressBar();
}

void ProgressBar::SetRange(float range)
{
    range = Max(range, 0.0f);
    if (range != range_)
    {
        range_ = range;
        UpdateProgressBar();
    }
}

void ProgressBar::SetValue(float value)
{
    value = Clamp(value, 0.0f, range_);
    if (value != value_)
    {
        value_ = value;
        UpdateProgressBar();

        using namespace ProgressBarChanged;

        VariantMap& eventData = GetEventDataMap();
        eventData[P_ELEMENT] = this;
        eventData[P_VALUE] = value_;
        SendEvent(E_PROGRESSBARCHANGED, eventData);
    }
}

void ProgressBar::ChangeValue(float delta)
{
    SetValue(value_ + delta);
}

void ProgressBar::SetShowPercentText(bool enable)
{
    showPercentText_ = enable;
    loadingText_->SetVisible(showPercentText_);
}

bool ProgressBar::FilterImplicitAttributes(XmlElement &dest) const
{
    if (!BorderImage::FilterImplicitAttributes(dest))
        return false;

    XmlElement childElem = dest.GetChild("element");
    if (!childElem)
        return false;
    if (!RemoveChildXML(childElem, "Name", "S_Knob"))
        return false;
    if (!RemoveChildXML(childElem, "Name", "S_Text"))
        return false;
    if (!RemoveChildXML(childElem, "Position"))
        return false;
    if (!RemoveChildXML(childElem, "Size"))
        return false;

    return true;
}

void ProgressBar::UpdateProgressBar()
{
    const IntRect &border = knob_->GetBorder();

    if (range_ > 0.0f)
    {
        if (orientation_ == O_HORIZONTAL)
        {
            auto loadingBarLength = (int) Max((float) GetWidth() * value_ / range_,
                                             (float) (border.left_ + border.right_));
            knob_->SetSize(loadingBarLength, GetHeight());
            knob_->SetPosition(Clamp(0, 0, GetWidth() - knob_->GetWidth()), 0);
        }
        else
        {
            auto loadingBarLength = (int) Max((float) GetHeight() * value_ / range_,
                                             (float) (border.top_ + border.bottom_));
            knob_->SetSize(GetWidth(), loadingBarLength);
            knob_->SetPosition(0, Clamp(GetHeight() - knob_->GetHeight(), 0, GetHeight() - knob_->GetHeight()));
        }
    }
    else
    {
        knob_->SetSize(GetSize());
        knob_->SetPosition(0, 0);
    }

    // Update the text.
    loadingText_->SetStyle(loadingPercentStyle_);
    loadingText_->SetAlignment(HA_CENTER, VA_CENTER);
    loadingText_->SetText(dviglo::ToString("%d %%", RoundToInt((value_ / range_) * 100.0f)));
}

}
