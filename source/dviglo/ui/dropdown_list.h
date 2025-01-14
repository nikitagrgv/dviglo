// Copyright (c) 2008-2023 the Urho3D project
// Copyright (c) 2022-2023 the Dviglo project
// License: MIT

#pragma once

#include "menu.h"

namespace dviglo
{

class ListView;

/// %Menu %UI element that displays a popup list view.
class DV_API DropDownList : public Menu
{
    DV_OBJECT(DropDownList);

public:
    /// Construct.
    explicit DropDownList();
    /// Destruct.
    ~DropDownList() override;
    /// Register object factory.
    static void register_object();

    /// Apply attribute changes that can not be applied immediately.
    void apply_attributes() override;
    /// Return UI rendering batches.
    void GetBatches(Vector<UIBatch>& batches, Vector<float>& vertexData, const IntRect& currentScissor) override;
    /// React to the popup being shown.
    void OnShowPopup() override;
    /// React to the popup being hidden.
    void OnHidePopup() override;
    /// React to editable status change.
    void OnSetEditable() override;

    /// Add item to the end of the list.
    void AddItem(UiElement* item);
    /// Insert item to a specific position. index can be ENDPOS.
    void InsertItem(i32 index, UiElement* item);
    /// Remove specific item.
    void RemoveItem(UiElement* item);
    /// Remove item at index.
    void RemoveItem(i32 index);
    /// Remove all items.
    void RemoveAllItems();
    /// Set selection.
    void SetSelection(i32 index);
    /// Set place holder text. This is the text shown when there is no selection (-1) in drop down list. Note that if the list has items, the default is to show the first item, so the "no selection" state has to be set explicitly.
    void SetPlaceholderText(const String& text);
    /// Set whether popup should be automatically resized to match the dropdown button width.
    void SetResizePopup(bool enable);

    /// Return number of items.
    i32 GetNumItems() const;
    /// Return item at index.
    UiElement* GetItem(i32 index) const;
    /// Return all items.
    Vector<UiElement*> GetItems() const;
    /// Return selection index, or NINDEX if none selected.
    i32 GetSelection() const;
    /// Return selected item, or null if none selected.
    UiElement* GetSelectedItem() const;

    /// Return listview element.
    ListView* GetListView() const { return listView_; }

    /// Return selected item placeholder element.
    UiElement* GetPlaceholder() const { return placeholder_; }

    /// Return place holder text.
    const String& GetPlaceholderText() const;

    /// Return whether popup should be automatically resized.
    bool GetResizePopup() const { return resizePopup_; }

    /// Set selection attribute.
    void SetSelectionAttr(i32 index);

protected:
    /// Filter implicit attributes in serialization process.
    bool FilterImplicitAttributes(XmlElement& dest) const override;
    /// Filter implicit attributes in serialization process.
    bool FilterPopupImplicitAttributes(XmlElement& dest) const override;

    /// Listview element.
    SharedPtr<ListView> listView_;
    /// Selected item placeholder element.
    SharedPtr<UiElement> placeholder_;
    /// Resize popup flag.
    bool resizePopup_;

private:
    /// Handle listview item click event.
    void HandleItemClicked(StringHash eventType, VariantMap& eventData);
    /// Handle a key press from the listview.
    void HandleListViewKey(StringHash eventType, VariantMap& eventData);
    /// Handle the listview selection change. Set placeholder text hidden/visible as necessary.
    void HandleSelectionChanged(StringHash eventType, VariantMap& eventData);

    /// Selected item index attribute.
    i32 selectionAttr_;
};

}
