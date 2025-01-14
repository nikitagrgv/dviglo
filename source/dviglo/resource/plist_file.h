// Copyright (c) 2008-2023 the Urho3D project
// Copyright (c) 2022-2023 the Dviglo project
// License: MIT

/// \file

#pragma once

#include "resource.h"

namespace dviglo
{

class XmlElement;

/// PList value types.
enum PListValueType
{
    PLVT_NONE = 0,
    PLVT_INT,
    PLVT_BOOL,
    PLVT_FLOAT,
    PLVT_STRING,
    PLVT_VALUEMAP,
    PLVT_VALUEVECTOR,
};

class PListValue;

/// PList value map.
using PListValueMap = HashMap<String, PListValue>;

/// Vector of PList value.
using PListValueVector = Vector<PListValue>;

/// PList value.
class DV_API PListValue
{
public:
    /// Construct.
    PListValue();
    /// Construct from int.
    explicit PListValue(int value);
    /// Construct from boolean.
    explicit PListValue(bool value);
    /// Construct from float.
    explicit PListValue(float value);
    /// Construct from string.
    explicit PListValue(const String& value);
    /// Construct from value map.
    explicit PListValue(PListValueMap& valueMap);
    /// Construct from value vector.
    explicit PListValue(PListValueVector& valueVector);
    /// Construct from another value.
    PListValue(const PListValue& value);
    /// Destruct.
    ~PListValue();

    /// Assign operator.
    PListValue& operator =(const PListValue& rhs);

    /// Return true if is valid.
    explicit operator bool() const { return type_ != PLVT_NONE; }

    /// Set int.
    void SetI32(i32 value);
    /// Set boolean.
    void SetBool(bool value);
    /// Set float.
    void SetFloat(float value);
    /// Set string.
    void SetString(const String& value);
    /// Set value map.
    void SetValueMap(const PListValueMap& valueMap);
    /// Set value vector.
    void SetValueVector(const PListValueVector& valueVector);

    /// Return type.
    PListValueType GetType() const { return type_; }

    /// Return int.
    i32 GetI32() const;
    /// Return boolean.
    bool GetBool() const;
    /// Return float.
    float GetFloat() const;
    /// Return string.
    const String& GetString() const;
    /// Return IntRect, for string type.
    IntRect GetIntRect() const;
    /// Return IntVector2, for string type.
    IntVector2 GetIntVector2() const;
    /// Return IntVector3, for string type.
    IntVector3 GetIntVector3() const;
    /// Return value map.
    const PListValueMap& GetValueMap() const;
    /// Return value vector.
    const PListValueVector& GetValueVector() const;

    /// Convert to value map (internal use only).
    PListValueMap& ConvertToValueMap();
    /// Convert to value vector (internal use only).
    PListValueVector& ConvertToValueVector();

private:
    /// Reset.
    void Reset();

    /// Type.
    PListValueType type_;
    /// Values.
    union
    {
        i32 int_;
        bool bool_;
        float float_;
        String* string_;
        PListValueMap* valueMap_;
        PListValueVector* valueVector_;
    };
};

/// Property list (plist).
class DV_API PListFile : public Resource
{
    DV_OBJECT(PListFile);

public:
    /// Construct.
    explicit PListFile();
    /// Destruct.
    ~PListFile() override;
    /// Register object factory.
    static void register_object();

    /// Load resource from stream. May be called from a worker thread. Return true if successful.
    bool begin_load(Deserializer& source) override;

    /// Return root.
    const PListValueMap& GetRoot() const { return root_; }

private:
    /// Load dictionary.
    bool LoadDict(PListValueMap& dict, const XmlElement& dictElem);
    /// Load array.
    bool LoadArray(PListValueVector& array, const XmlElement& arrayElem);
    /// Load value.
    bool LoadValue(PListValue& value, const XmlElement& valueElem);

    /// Root dictionary.
    PListValueMap root_;
};

}
