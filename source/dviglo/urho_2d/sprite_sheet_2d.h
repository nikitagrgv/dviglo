// Copyright (c) 2008-2023 the Urho3D project
// Copyright (c) 2022-2023 the Dviglo project
// License: MIT

#pragma once

#include "../resource/resource.h"

namespace dviglo
{

class PListFile;
class Sprite2D;
class Texture2D;
class XmlFile;
class JSONFile;

/// Sprite sheet.
class DV_API SpriteSheet2D : public Resource
{
    DV_OBJECT(SpriteSheet2D);

public:
    /// Construct.
    explicit SpriteSheet2D();
    /// Destruct.
    ~SpriteSheet2D() override;
    /// Register object factory.
    static void register_object();

    /// Load resource from stream. May be called from a worker thread. Return true if successful.
    bool begin_load(Deserializer& source) override;
    /// Finish resource loading. Always called from the main thread. Return true if successful.
    bool end_load() override;

    /// Set texture.
    void SetTexture(Texture2D* texture);
    /// Define sprite.
    void DefineSprite(const String& name, const IntRect& rectangle, const Vector2& hotSpot = Vector2(0.5f, 0.5f),
        const IntVector2& offset = IntVector2::ZERO);

    /// Return texture.
    Texture2D* GetTexture() const { return texture_; }
    /// Return sprite.
    Sprite2D* GetSprite(const String& name) const;

    /// Return sprite mapping.
    const HashMap<String, SharedPtr<Sprite2D>>& GetSpriteMapping() const { return spriteMapping_; }

private:
    /// Begin load from PList file.
    bool BeginLoadFromPListFile(Deserializer& source);
    /// End load from PList file.
    bool EndLoadFromPListFile();

    /// Begin load from XML file.
    bool BeginLoadFromXMLFile(Deserializer& source);
    /// End load from XML file.
    bool EndLoadFromXMLFile();
    /// Begin load from JSON file.
    bool BeginLoadFromJSONFile(Deserializer& source);
    /// End load from JSON file.
    bool EndLoadFromJSONFile();

    /// Texture.
    SharedPtr<Texture2D> texture_;
    /// Sprite mapping.
    HashMap<String, SharedPtr<Sprite2D>> spriteMapping_;
    /// PList file used while loading.
    SharedPtr<PListFile> loadPListFile_;
    /// XML file used while loading.
    SharedPtr<XmlFile> loadXMLFile_;
    /// JSON file used while loading.
    SharedPtr<JSONFile> loadJSONFile_;
    /// Texture name used while loading.
    String loadTextureName_;
};

}
