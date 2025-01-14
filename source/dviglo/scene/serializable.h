// Copyright (c) 2008-2023 the Urho3D project
// Copyright (c) 2022-2023 the Dviglo project
// License: MIT

#pragma once

#include "../core/attribute.h"
#include "../core/object.h"

#include <cstddef>
#include <memory>
#include <type_traits>

namespace dviglo
{

class Connection;
class Deserializer;
class Serializer;
class XmlElement;
class JSONValue;

struct DirtyBits;
struct NetworkState;
struct ReplicationState;

/// Base class for objects with automatic serialization through attributes.
class DV_API Serializable : public Object
{
    DV_OBJECT(Serializable);

public:
    /// Construct.
    explicit Serializable();
    /// Destruct.
    ~Serializable() override;

    /// Handle attribute write access. Default implementation writes to the variable at offset, or invokes the set accessor.
    virtual void OnSetAttribute(const AttributeInfo& attr, const Variant& src);
    /// Handle attribute read access. Default implementation reads the variable at offset, or invokes the get accessor.
    virtual void OnGetAttribute(const AttributeInfo& attr, Variant& dest) const;
    /// Return attribute descriptions, or null if none defined.
    virtual const Vector<AttributeInfo>* GetAttributes() const;
    /// Return network replication attribute descriptions, or null if none defined.
    virtual const Vector<AttributeInfo>* GetNetworkAttributes() const;
    /// Load from binary data. Return true if successful.
    virtual bool Load(Deserializer& source);
    /// Save as binary data. Return true if successful.
    virtual bool Save(Serializer& dest) const;
    /// Load from XML data. Return true if successful.
    virtual bool load_xml(const XmlElement& source);
    /// Save as XML data. Return true if successful.
    virtual bool save_xml(XmlElement& dest) const;
    /// Load from JSON data. Return true if successful.
    virtual bool load_json(const JSONValue& source);
    /// Save as JSON data. Return true if successful.
    virtual bool save_json(JSONValue& dest) const;

    /// Apply attribute changes that can not be applied immediately. Called after scene load or a network update.
    virtual void apply_attributes() { }

    /// Return whether should save default-valued attributes into XML. Default false.
    virtual bool SaveDefaultAttributes() const { return false; }

    /// Mark for attribute check on the next network update.
    virtual void MarkNetworkUpdate() { }

    /// Set attribute by index. Return true if successfully set.
    bool SetAttribute(unsigned index, const Variant& value);
    /// Set attribute by name. Return true if successfully set.
    bool SetAttribute(const String& name, const Variant& value);
    /// Set instance-level default flag.
    void SetInstanceDefault(bool enable) { setInstanceDefault_ = enable; }
    /// Reset all editable attributes to their default values.
    void ResetToDefault();
    /// Remove instance's default values if they are set previously.
    void RemoveInstanceDefault();
    /// Set temporary flag. Temporary objects will not be saved.
    void SetTemporary(bool enable);
    /// Enable interception of an attribute from network updates. Intercepted attributes are sent as events instead of applying directly. This can be used to implement client side prediction.
    void SetInterceptNetworkUpdate(const String& attributeName, bool enable);
    /// Allocate network attribute state.
    void AllocateNetworkState();
    /// Write initial delta network update.
    void WriteInitialDeltaUpdate(Serializer& dest, unsigned char timeStamp);
    /// Write a delta network update according to dirty attribute bits.
    void WriteDeltaUpdate(Serializer& dest, const DirtyBits& attributeBits, unsigned char timeStamp);
    /// Write a latest data network update.
    void WriteLatestDataUpdate(Serializer& dest, unsigned char timeStamp);
    /// Read and apply a network delta update. Return true if attributes were changed.
    bool ReadDeltaUpdate(Deserializer& source);
    /// Read and apply a network latest data update. Return true if attributes were changed.
    bool ReadLatestDataUpdate(Deserializer& source);

    /// Return attribute value by index. Return empty if illegal index.
    Variant GetAttribute(unsigned index) const;
    /// Return attribute value by name. Return empty if not found.
    Variant GetAttribute(const String& name) const;
    /// Return attribute default value by index. Return empty if illegal index.
    Variant GetAttributeDefault(unsigned index) const;
    /// Return attribute default value by name. Return empty if not found.
    Variant GetAttributeDefault(const String& name) const;
    /// Return number of attributes.
    unsigned GetNumAttributes() const;
    /// Return number of network replication attributes.
    unsigned GetNumNetworkAttributes() const;

    /// Return whether is temporary.
    bool IsTemporary() const { return temporary_; }

    /// Return whether an attribute's network updates are being intercepted.
    bool GetInterceptNetworkUpdate(const String& attributeName) const;

    /// Return the network attribute state, if allocated.
    NetworkState* GetNetworkState() const { return networkState_.get(); }

protected:
    /// Network attribute state.
    std::unique_ptr<NetworkState> networkState_;

private:
    /// Set instance-level default value. Allocate the internal data structure as necessary.
    void SetInstanceDefault(const String& name, const Variant& defaultValue);
    /// Get instance-level default value.
    Variant GetInstanceDefault(const String& name) const;

    /// Attribute default value at each instance level.
    std::unique_ptr<VariantMap> instanceDefaultValues_;

    /// When true, store the attribute value as instance's default value (internal use only).
    bool setInstanceDefault_;

    /// Temporary flag.
    bool temporary_;
};

/// Template implementation of the variant attribute accessor.
template <class TClassType, class TGetFunction, class TSetFunction>
class VariantAttributeAccessorImpl : public AttributeAccessor
{
public:
    /// Construct.
    VariantAttributeAccessorImpl(TGetFunction getFunction, TSetFunction setFunction) : getFunction_(getFunction), setFunction_(setFunction) { }

    /// Invoke getter function.
    void Get(const Serializable* ptr, Variant& value) const override
    {
        assert(ptr);
        const auto classPtr = static_cast<const TClassType*>(ptr);
        getFunction_(*classPtr, value);
    }

    /// Invoke setter function.
    void Set(Serializable* ptr, const Variant& value) override
    {
        assert(ptr);
        auto classPtr = static_cast<TClassType*>(ptr);
        setFunction_(*classPtr, value);
    }

private:
    /// Get functor.
    TGetFunction getFunction_;
    /// Set functor.
    TSetFunction setFunction_;
};

/// Make variant attribute accessor implementation.
/// \tparam TClassType Serializable class type.
/// \tparam TGetFunction Functional object with call signature `void getFunction(const TClassType& self, Variant& value)`
/// \tparam TSetFunction Functional object with call signature `void setFunction(TClassType& self, const Variant& value)`
template <class TClassType, class TGetFunction, class TSetFunction>
SharedPtr<AttributeAccessor> MakeVariantAttributeAccessor(TGetFunction getFunction, TSetFunction setFunction)
{
    return SharedPtr<AttributeAccessor>(new VariantAttributeAccessorImpl<TClassType, TGetFunction, TSetFunction>(getFunction, setFunction));
}

/// Make member attribute accessor.
#define DV_MAKE_MEMBER_ATTRIBUTE_ACCESSOR(typeName, variable) dviglo::MakeVariantAttributeAccessor<ClassName>( \
    [](const ClassName& self, dviglo::Variant& value) { value = self.variable; }, \
    [](ClassName& self, const dviglo::Variant& value) { self.variable = value.Get<typeName>(); })

/// Make member attribute accessor with custom post-set callback.
#define DV_MAKE_MEMBER_ATTRIBUTE_ACCESSOR_EX(typeName, variable, postSetCallback) dviglo::MakeVariantAttributeAccessor<ClassName>( \
    [](const ClassName& self, dviglo::Variant& value) { value = self.variable; }, \
    [](ClassName& self, const dviglo::Variant& value) { self.variable = value.Get<typeName>(); self.postSetCallback(); })

/// Make get/set attribute accessor.
#define DV_MAKE_GET_SET_ATTRIBUTE_ACCESSOR(getFunction, setFunction, typeName) dviglo::MakeVariantAttributeAccessor<ClassName>( \
    [](const ClassName& self, dviglo::Variant& value) { value = self.getFunction(); }, \
    [](ClassName& self, const dviglo::Variant& value) { self.setFunction(value.Get<typeName>()); })

/// Make member enum attribute accessor.
#define DV_MAKE_MEMBER_ENUM_ATTRIBUTE_ACCESSOR(variable) dviglo::MakeVariantAttributeAccessor<ClassName>( \
    [](const ClassName& self, dviglo::Variant& value) { value = static_cast<int>(self.variable); }, \
    [](ClassName& self, const dviglo::Variant& value) { self.variable = static_cast<decltype(self.variable)>(value.Get<int>()); })

/// Make member enum attribute accessor with custom post-set callback.
#define DV_MAKE_MEMBER_ENUM_ATTRIBUTE_ACCESSOR_EX(variable, postSetCallback) dviglo::MakeVariantAttributeAccessor<ClassName>( \
    [](const ClassName& self, dviglo::Variant& value) { value = static_cast<int>(self.variable); }, \
    [](ClassName& self, const dviglo::Variant& value) { self.variable = static_cast<decltype(self.variable)>(value.Get<int>()); self.postSetCallback(); })

/// Make get/set enum attribute accessor.
#define DV_MAKE_GET_SET_ENUM_ATTRIBUTE_ACCESSOR(getFunction, setFunction, typeName) dviglo::MakeVariantAttributeAccessor<ClassName>( \
    [](const ClassName& self, dviglo::Variant& value) { value = static_cast<int>(self.getFunction()); }, \
    [](ClassName& self, const dviglo::Variant& value) { self.setFunction(static_cast<typeName>(value.Get<int>())); })

/// Attribute metadata.
namespace AttributeMetadata
{
    /// Names of vector struct elements. StringVector.
    static const StringHash P_VECTOR_STRUCT_ELEMENTS("VectorStructElements");
}

/// Get result type of a class member function with zero args.
#define DV_GETTER_RETURN_TYPE(getFunction) \
    std::remove_cv_t<std::remove_reference_t<std::invoke_result_t<decltype(&ClassName::getFunction), ClassName>>>

// The following macros need to be used within a class member function such as ClassName::register_object()

/// Copy attributes from a base class.
#define DV_COPY_BASE_ATTRIBUTES(sourceClassName) DV_CONTEXT->CopyBaseAttributes<sourceClassName, ClassName>()
/// Update the default value of an already registered attribute.
#define DV_UPDATE_ATTRIBUTE_DEFAULT_VALUE(name, defaultValue) DV_CONTEXT->UpdateAttributeDefaultValue<ClassName>(name, defaultValue)
/// Remove attribute by name.
#define DV_REMOVE_ATTRIBUTE(name) DV_CONTEXT->RemoveAttribute<ClassName>(name)

/// Define an object member attribute.
#define DV_ATTRIBUTE(name, variable, defaultValue, mode) DV_CONTEXT->RegisterAttribute<ClassName>(dviglo::AttributeInfo( \
    dviglo::GetVariantType<std::remove_reference_t<decltype(variable)>>(), name, DV_MAKE_MEMBER_ATTRIBUTE_ACCESSOR(std::remove_reference_t<decltype(variable)>, variable), nullptr, defaultValue, mode))

/// Define an object member attribute with forced type. Allows use custom type convertible to variant type (e.g. serialize u8 as int).
#define DV_ATTRIBUTE_FORCE_TYPE(name, typeName, variable, defaultValue, mode) DV_CONTEXT->RegisterAttribute<ClassName>(dviglo::AttributeInfo( \
    dviglo::GetVariantType<typeName>(), name, DV_MAKE_MEMBER_ATTRIBUTE_ACCESSOR(typeName, variable), nullptr, defaultValue, mode))

/// Define an object member attribute. Post-set member function callback is called when attribute set.
#define DV_ATTRIBUTE_EX(name, variable, postSetCallback, defaultValue, mode) DV_CONTEXT->RegisterAttribute<ClassName>(dviglo::AttributeInfo( \
    dviglo::GetVariantType<std::remove_reference_t<decltype(variable)>>(), name, DV_MAKE_MEMBER_ATTRIBUTE_ACCESSOR_EX(std::remove_reference_t<decltype(variable)>, variable, postSetCallback), nullptr, defaultValue, mode))

/// Define an object member attribute with forced type. Allows use custom type convertible to variant type (e.g. serialize u8 as int). Post-set member function callback is called when attribute set.
#define DV_ATTRIBUTE_FORCE_TYPE_EX(name, typeName, variable, postSetCallback, defaultValue, mode) DV_CONTEXT->RegisterAttribute<ClassName>(dviglo::AttributeInfo( \
    dviglo::GetVariantType<typeName>(), name, DV_MAKE_MEMBER_ATTRIBUTE_ACCESSOR_EX(typeName, variable, postSetCallback), nullptr, defaultValue, mode))

/// Define an attribute that uses get and set functions.
#define DV_ACCESSOR_ATTRIBUTE(name, getFunction, setFunction, defaultValue, mode) DV_CONTEXT->RegisterAttribute<ClassName>(dviglo::AttributeInfo( \
    dviglo::GetVariantType<DV_GETTER_RETURN_TYPE(getFunction)>(), name, DV_MAKE_GET_SET_ATTRIBUTE_ACCESSOR(getFunction, setFunction, DV_GETTER_RETURN_TYPE(getFunction)), nullptr, defaultValue, mode))

/// Define an attribute that uses get and set functions with forced type.
#define DV_ACCESSOR_ATTRIBUTE_FORCE_TYPE(name, getFunction, setFunction, typeName, defaultValue, mode) DV_CONTEXT->RegisterAttribute<ClassName>(dviglo::AttributeInfo( \
    dviglo::GetVariantType<typeName>(), name, DV_MAKE_GET_SET_ATTRIBUTE_ACCESSOR(getFunction, setFunction, typeName), nullptr, defaultValue, mode))

/// Define an object member attribute. Zero-based enum values are mapped to names through an array of C string pointers.
#define DV_ENUM_ATTRIBUTE(name, variable, enumNames, defaultValue, mode) DV_CONTEXT->RegisterAttribute<ClassName>(dviglo::AttributeInfo( \
    dviglo::VAR_INT, name, DV_MAKE_MEMBER_ENUM_ATTRIBUTE_ACCESSOR(variable), enumNames, static_cast<int>(defaultValue), mode))
/// Define an object member attribute. Zero-based enum values are mapped to names through an array of C string pointers. Post-set member function callback is called when attribute set.
#define DV_ENUM_ATTRIBUTE_EX(name, variable, postSetCallback, enumNames, defaultValue, mode) DV_CONTEXT->RegisterAttribute<ClassName>(dviglo::AttributeInfo( \
    dviglo::VAR_INT, name, DV_MAKE_MEMBER_ENUM_ATTRIBUTE_ACCESSOR_EX(variable, postSetCallback), enumNames, static_cast<int>(defaultValue), mode))

/// Define an attribute that uses get and set functions. Zero-based enum values are mapped to names through an array of C string pointers.
#define DV_ENUM_ACCESSOR_ATTRIBUTE(name, getFunction, setFunction, enumNames, defaultValue, mode) DV_CONTEXT->RegisterAttribute<ClassName>(dviglo::AttributeInfo( \
    dviglo::VAR_INT, name, DV_MAKE_GET_SET_ENUM_ATTRIBUTE_ACCESSOR(getFunction, setFunction, DV_GETTER_RETURN_TYPE(getFunction)), enumNames, static_cast<int>(defaultValue), mode))

/// Define an attribute that uses get and set functions with forced type. Zero-based enum values are mapped to names through an array of C string pointers.
#define DV_ENUM_ACCESSOR_ATTRIBUTE_FORCE_TYPE(name, getFunction, setFunction, typeName, enumNames, defaultValue, mode) DV_CONTEXT->RegisterAttribute<ClassName>(dviglo::AttributeInfo( \
    dviglo::VAR_INT, name, DV_MAKE_GET_SET_ENUM_ATTRIBUTE_ACCESSOR(getFunction, setFunction, typeName), enumNames, static_cast<int>(defaultValue), mode))

/// Define an attribute with custom setter and getter.
#define DV_CUSTOM_ATTRIBUTE(name, getFunction, setFunction, typeName, defaultValue, mode) DV_CONTEXT->RegisterAttribute<ClassName>(dviglo::AttributeInfo( \
    dviglo::GetVariantType<typeName>(), name, dviglo::MakeVariantAttributeAccessor<ClassName>(getFunction, setFunction), nullptr, defaultValue, mode))
/// Define an enum attribute with custom setter and getter. Zero-based enum values are mapped to names through an array of C string pointers.
#define DV_CUSTOM_ENUM_ATTRIBUTE(name, getFunction, setFunction, enumNames, defaultValue, mode) DV_CONTEXT->RegisterAttribute<ClassName>(dviglo::AttributeInfo( \
    dviglo::VAR_INT, name, dviglo::MakeVariantAttributeAccessor<ClassName>(getFunction, setFunction), enumNames, static_cast<int>(defaultValue), mode))

}
