// Copyright (c) 2008-2023 the Urho3D project
// Copyright (c) 2022-2023 the Dviglo project
// License: MIT

#include "../core/context.h"
#include "../graphics/graphics.h"
#include "../graphics_api/texture_2d.h"
#include "../io/file.h"
#include "../io/file_system.h"
#include "../io/log.h"
#include "../io/memory_buffer.h"
#include "../resource/resource_cache.h"
#include "font.h"
#include "font_face_bitmap.h"
#include "ui.h"

#include "../common/debug_new.h"

using namespace std;

namespace dviglo
{

FontFaceBitmap::FontFaceBitmap(Font* font) :
    FontFace(font)
{
}

FontFaceBitmap::~FontFaceBitmap() = default;

// FIXME: The Load() and Save() should be refactored accordingly after the recent FontGlyph struct changes

bool FontFaceBitmap::Load(const byte* fontData, unsigned fontDataSize, float pointSize)
{
    SharedPtr<XmlFile> xmlReader(new XmlFile());
    MemoryBuffer memoryBuffer(fontData, fontDataSize);
    if (!xmlReader->Load(memoryBuffer))
    {
        DV_LOGERROR("Could not load XML file");
        return false;
    }

    XmlElement root = xmlReader->GetRoot("font");
    if (root.IsNull())
    {
        DV_LOGERROR("Could not find Font element");
        return false;
    }

    XmlElement pagesElem = root.GetChild("pages");
    if (pagesElem.IsNull())
    {
        DV_LOGERROR("Could not find Pages element");
        return false;
    }

    XmlElement infoElem = root.GetChild("info");
    if (!infoElem.IsNull())
        pointSize_ = infoElem.GetI32("size");

    XmlElement commonElem = root.GetChild("common");
    rowHeight_ = commonElem.GetI32("lineHeight");
    unsigned pages = commonElem.GetU32("pages");
    textures_.Reserve(pages);

    ResourceCache* cache = DV_RES_CACHE;
    String fontPath = GetPath(font_->GetName());
    unsigned totalTextureSize = 0;

    XmlElement pageElem = pagesElem.GetChild("page");
    for (unsigned i = 0; i < pages; ++i)
    {
        if (pageElem.IsNull())
        {
            DV_LOGERROR("Could not find Page element for page: " + String(i));
            return false;
        }

        // Assume the font image is in the same directory as the font description file
        String textureFile = fontPath + pageElem.GetAttribute("file");

        // Load texture manually to allow controlling the alpha channel mode
        shared_ptr<File> fontFile = cache->GetFile(textureFile);
        SharedPtr<Image> fontImage(new Image());
        if (!fontFile || !fontImage->Load(*fontFile))
        {
            DV_LOGERROR("Failed to load font image file");
            return false;
        }
        SharedPtr<Texture2D> texture = LoadFaceTexture(fontImage);
        if (!texture)
            return false;

        textures_.Push(texture);

        // Add texture to resource cache
        texture->SetName(fontFile->GetName());
        cache->add_manual_resource(texture);

        totalTextureSize += fontImage->GetWidth() * fontImage->GetHeight() * fontImage->GetComponents();

        pageElem = pageElem.GetNext("page");
    }

    XmlElement charsElem = root.GetChild("chars");
    int count = charsElem.GetI32("count");

    XmlElement charElem = charsElem.GetChild("char");
    while (!charElem.IsNull())
    {
        int id = charElem.GetI32("id");

        FontGlyph glyph;
        glyph.x = (short)charElem.GetI32("x");
        glyph.y = (short)charElem.GetI32("y");
        glyph.width = glyph.tex_width = (short)charElem.GetI32("width");
        glyph.height = glyph.tex_height = (short)charElem.GetI32("height");
        glyph.offset_x = (short)charElem.GetI32("xoffset");
        glyph.offset_y = (short)charElem.GetI32("yoffset");
        glyph.advance_x = (short)charElem.GetI32("xadvance");
        glyph.page = charElem.GetI32("page");
        assert(glyph.page >= 0);

        glyphMapping_[id] = glyph;

        charElem = charElem.GetNext("char");
    }

    XmlElement kerningsElem = root.GetChild("kernings");
    if (kerningsElem.NotNull())
    {
        XmlElement kerningElem = kerningsElem.GetChild("kerning");
        while (!kerningElem.IsNull())
        {
            unsigned first = kerningElem.GetI32("first");
            unsigned second = kerningElem.GetI32("second");
            unsigned value = first << 16u | second;
            kerningMapping_[value] = (short)kerningElem.GetI32("amount");

            kerningElem = kerningElem.GetNext("kerning");
        }
    }

    DV_LOGDEBUGF("Bitmap font face %s has %d glyphs", GetFileName(font_->GetName()).c_str(), count);

    font_->SetMemoryUse(font_->GetMemoryUse() + totalTextureSize);
    return true;
}

bool FontFaceBitmap::Load(FontFace* fontFace, bool usedGlyphs)
{
    if (this == fontFace)
        return true;

    if (!usedGlyphs)
    {
        glyphMapping_ = fontFace->glyphMapping_;
        kerningMapping_ = fontFace->kerningMapping_;
        textures_ = fontFace->textures_;
        pointSize_ = fontFace->pointSize_;
        rowHeight_ = fontFace->rowHeight_;

        return true;
    }

    pointSize_ = fontFace->pointSize_;
    rowHeight_ = fontFace->rowHeight_;

    unsigned numPages = 1;
    int maxTextureSize = DV_UI->max_font_texture_size();
    AreaAllocator allocator(FONT_TEXTURE_MIN_SIZE, FONT_TEXTURE_MIN_SIZE, maxTextureSize, maxTextureSize);

    for (HashMap<c32, FontGlyph>::ConstIterator i = fontFace->glyphMapping_.Begin(); i != fontFace->glyphMapping_.End(); ++i)
    {
        FontGlyph fontGlyph = i->second_;
        if (!fontGlyph.used)
            continue;

        int x, y;
        if (!allocator.Allocate(fontGlyph.width + 1, fontGlyph.height + 1, x, y))
        {
            ++numPages;

            allocator = AreaAllocator(FONT_TEXTURE_MIN_SIZE, FONT_TEXTURE_MIN_SIZE, maxTextureSize, maxTextureSize);
            if (!allocator.Allocate(fontGlyph.width + 1, fontGlyph.height + 1, x, y))
                return false;
        }

        fontGlyph.x = (short)x;
        fontGlyph.y = (short)y;
        fontGlyph.page = numPages - 1;

        glyphMapping_[i->first_] = fontGlyph;
    }

    // Assume that format is the same for all textures and that bitmap font type may have more than one component
    unsigned components = ConvertFormatToNumComponents(fontFace->textures_[0]->GetFormat());

    // Save the existing textures as image resources
    Vector<SharedPtr<Image>> oldImages;
    for (unsigned i = 0; i < fontFace->textures_.Size(); ++i)
        oldImages.Push(SaveFaceTexture(fontFace->textures_[i]));

    Vector<SharedPtr<Image>> newImages(numPages);
    for (unsigned i = 0; i < numPages; ++i)
    {
        SharedPtr<Image> image(new Image());

        int width = maxTextureSize;
        int height = maxTextureSize;
        if (i == numPages - 1)
        {
            width = allocator.GetWidth();
            height = allocator.GetHeight();
        }

        image->SetSize(width, height, components);
        memset(image->GetData(), 0, (size_t)width * height * components);

        newImages[i] = image;
    }

    for (HashMap<c32, FontGlyph>::Iterator i = glyphMapping_.Begin(); i != glyphMapping_.End(); ++i)
    {
        FontGlyph& newGlyph = i->second_;
        const FontGlyph& oldGlyph = fontFace->glyphMapping_[i->first_];
        Blit(newImages[newGlyph.page], newGlyph.x, newGlyph.y, newGlyph.width, newGlyph.height, oldImages[oldGlyph.page],
            oldGlyph.x, oldGlyph.y, components);
    }

    textures_.Resize(newImages.Size());
    for (unsigned i = 0; i < newImages.Size(); ++i)
        textures_[i] = LoadFaceTexture(newImages[i]);

    for (HashMap<u32, float>::ConstIterator i = fontFace->kerningMapping_.Begin(); i != fontFace->kerningMapping_.End(); ++i)
    {
        unsigned first = (i->first_) >> 16u;
        unsigned second = (i->first_) & 0xffffu;
        if (glyphMapping_.Find(first) != glyphMapping_.End() && glyphMapping_.Find(second) != glyphMapping_.End())
            kerningMapping_[i->first_] = i->second_;
    }

    return true;
}

bool FontFaceBitmap::Save(Serializer& dest, int pointSize, const String& indentation)
{
    SharedPtr<XmlFile> xml(new XmlFile());
    XmlElement rootElem = xml->CreateRoot("font");

    // Information
    XmlElement childElem = rootElem.create_child("info");
    String fileName = GetFileName(font_->GetName());
    childElem.SetAttribute("face", fileName);
    childElem.SetAttribute("size", String(pointSize));

    // Common
    childElem = rootElem.create_child("common");
    childElem.SetI32("lineHeight", rowHeight_);
    unsigned pages = textures_.Size();
    childElem.SetU32("pages", pages);

    // Construct the path to store the texture
    String pathName;
    auto* file = dynamic_cast<File*>(&dest);
    if (file)
        // If serialize to file, use the file's path
        pathName = GetPath(file->GetName());
    else
        // Otherwise, use the font resource's path
        pathName = "Data/" + GetPath(font_->GetName());

    // Pages
    childElem = rootElem.create_child("pages");
    for (unsigned i = 0; i < pages; ++i)
    {
        XmlElement pageElem = childElem.create_child("page");
        pageElem.SetI32("id", i);
        String texFileName = fileName + "_" + String(i) + ".png";
        pageElem.SetAttribute("file", texFileName);

        // Save the font face texture to image file
        SaveFaceTexture(textures_[i], pathName + texFileName);
    }

    // Chars and kernings
    XmlElement charsElem = rootElem.create_child("chars");
    unsigned numGlyphs = glyphMapping_.Size();
    charsElem.SetI32("count", numGlyphs);

    for (HashMap<c32, FontGlyph>::ConstIterator i = glyphMapping_.Begin(); i != glyphMapping_.End(); ++i)
    {
        // Char
        XmlElement charElem = charsElem.create_child("char");
        charElem.SetI32("id", i->first_);

        const FontGlyph& glyph = i->second_;
        charElem.SetI32("x", glyph.x);
        charElem.SetI32("y", glyph.y);
        charElem.SetI32("width", glyph.width);
        charElem.SetI32("height", glyph.height);
        charElem.SetI32("xoffset", glyph.offset_x);
        charElem.SetI32("yoffset", glyph.offset_y);
        charElem.SetI32("xadvance", glyph.advance_x);
        charElem.SetI32("page", glyph.page);
    }

    if (!kerningMapping_.Empty())
    {
        XmlElement kerningsElem = rootElem.create_child("kernings");
        for (HashMap<u32, float>::ConstIterator i = kerningMapping_.Begin(); i != kerningMapping_.End(); ++i)
        {
            XmlElement kerningElem = kerningsElem.create_child("kerning");
            kerningElem.SetI32("first", i->first_ >> 16u);
            kerningElem.SetI32("second", i->first_ & 0xffffu);
            kerningElem.SetI32("amount", i->second_);
        }
    }

    return xml->Save(dest, indentation);
}

unsigned FontFaceBitmap::ConvertFormatToNumComponents(unsigned format)
{
    if (format == Graphics::GetRGBAFormat())
        return 4;
    else if (format == Graphics::GetRGBFormat())
        return 3;
    else if (format == Graphics::GetLuminanceAlphaFormat())
        return 2;
    else
        return 1;
}

SharedPtr<Image> FontFaceBitmap::SaveFaceTexture(Texture2D* texture)
{
    SharedPtr<Image> image(new Image());
    image->SetSize(texture->GetWidth(), texture->GetHeight(), ConvertFormatToNumComponents(texture->GetFormat()));
    if (!texture->GetData(0, image->GetData()))
    {
        DV_LOGERROR("Could not save texture to image resource");
        return SharedPtr<Image>();
    }
    return image;
}

bool FontFaceBitmap::SaveFaceTexture(Texture2D* texture, const String& fileName)
{
    SharedPtr<Image> image = SaveFaceTexture(texture);
    return image ? image->SavePNG(fileName) : false;
}

void FontFaceBitmap::Blit(Image* dest, int x, int y, int width, int height, Image* source, int sourceX, int sourceY, int components)
{
    unsigned char* destData = dest->GetData() + (y * dest->GetWidth() + x) * components;
    unsigned char* sourceData = source->GetData() + (sourceY * source->GetWidth() + sourceX) * components;
    for (int i = 0; i < height; ++i)
    {
        memcpy(destData, sourceData, (size_t)width * components);
        destData += dest->GetWidth() * components;
        sourceData += source->GetWidth() * components;
    }
}

}
