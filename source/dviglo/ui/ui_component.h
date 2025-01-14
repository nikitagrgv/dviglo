// Copyright (c) 2008-2023 the Urho3D project
// Copyright (c) 2022-2023 the Dviglo project
// License: MIT

#pragma once

#include "../scene/component.h"
#include "ui_element.h"

namespace dviglo
{

class Material;
class Texture2D;
class StaticModel;
class Viewport;
class UiElement;
class UIBatch;
class VertexBuffer;
class UiElement3d;

class DV_API UiComponent : public Component
{
    DV_OBJECT(UiComponent);

public:
    /// Construct.
    explicit UiComponent();
    /// Destruct.
    ~UiComponent() override;
    /// Register object factory.
    static void register_object();

    /// Return UiElement.
    UiElement* GetRoot() const;
    /// Return material which will be used for rendering UI texture.
    Material* GetMaterial() const;
    /// Return texture which will be used for rendering UI to.
    Texture2D* GetTexture() const;
    /// Set index of viewport to be used for screen coordinate translation.
    void SetViewportIndex(unsigned index);

protected:
    /// Handle component being added to Node or removed from it.
    void OnNodeSet(Node* node) override;
    /// Handle resizing of element. Setting size of element will automatically resize texture. UiElement size matches size of texture.
    void OnElementResized(StringHash eventType, VariantMap& args);

    /// Material that is set to the model.
    SharedPtr<Material> material_;
    /// Texture that UiElement will be rendered into.
    SharedPtr<Texture2D> texture_;
    /// Model created by this component. If node already has StaticModel then this will be null.
    SharedPtr<StaticModel> model_;
    /// UiElement to be rendered into texture. It also handles screen to UI coordinate translation.
    SharedPtr<UiElement3d> rootElement_;
    /// Viewport index to be set when component is added to a node.
    unsigned viewportIndex_;
};

}
