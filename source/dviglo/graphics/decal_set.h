// Copyright (c) 2008-2023 the Urho3D project
// Copyright (c) 2022-2023 the Dviglo project
// License: MIT

#pragma once

#include "../containers/list.h"
#include "drawable.h"
#include "skeleton.h"
#include "../math/frustum.h"

namespace dviglo
{

class IndexBuffer;
class VertexBuffer;

/// %Decal vertex.
struct DecalVertex
{
    /// Construct with defaults.
    DecalVertex() = default;

    /// Construct with position and normal.
    DecalVertex(const Vector3& position, const Vector3& normal) :
        position_(position),
        normal_(normal)
    {
    }

    /// Construct with position, normal and skinning information.
    DecalVertex(const Vector3& position, const Vector3& normal, const float* blendWeights, const unsigned char* blendIndices) :
        position_(position),
        normal_(normal)
    {
        for (unsigned i = 0; i < 4; ++i)
        {
            blendWeights_[i] = blendWeights[i];
            blendIndices_[i] = blendIndices[i];
        }
    }

    /// Position.
    Vector3 position_;
    /// Normal.
    Vector3 normal_;
    /// Texture coordinates.
    Vector2 texCoord_;
    /// Tangent.
    Vector4 tangent_;
    /// Blend weights.
    float blendWeights_[4]{};
    /// Blend indices.
    unsigned char blendIndices_[4]{};
};

/// One decal in a decal set.
struct Decal
{
    /// Construct with defaults.
    Decal() :
        timer_(0.0f),
        timeToLive_(0.0f)
    {
    }

    /// Add a vertex.
    void AddVertex(const DecalVertex& vertex);
    /// Calculate local-space bounding box.
    void CalculateBoundingBox();

    /// Decal age timer.
    float timer_;
    /// Maximum time to live in seconds (0 = infinite).
    float timeToLive_;
    /// Local-space bounding box.
    BoundingBox boundingBox_;
    /// Decal vertices.
    Vector<DecalVertex> vertices_;
    /// Decal indices.
    Vector<unsigned short> indices_;
};

/// %Decal renderer component.
class DV_API DecalSet : public Drawable
{
    DV_OBJECT(DecalSet);

public:
    /// Construct.
    explicit DecalSet();
    /// Destruct.
    ~DecalSet() override;
    /// Register object factory.
    static void register_object();

    /// Apply attribute changes that can not be applied immediately. Called after scene load or a network update.
    void apply_attributes() override;
    /// Handle enabled/disabled state change.
    void OnSetEnabled() override;
    /// Process octree raycast. May be called from a worker thread.
    void ProcessRayQuery(const RayOctreeQuery& query, Vector<RayQueryResult>& results) override;
    /// Calculate distance and prepare batches for rendering. May be called from worker thread(s), possibly re-entrantly.
    void update_batches(const FrameInfo& frame) override;
    /// Prepare geometry for rendering. Called from a worker thread if possible (no GPU update).
    void UpdateGeometry(const FrameInfo& frame) override;
    /// Return whether a geometry update is necessary, and if it can happen in a worker thread.
    UpdateGeometryType GetUpdateGeometryType() override;

    /// Set material. The material should use a small negative depth bias to avoid Z-fighting.
    void SetMaterial(Material* material);
    /// Set maximum number of decal vertices.
    void SetMaxVertices(unsigned num);
    /// Set maximum number of decal vertex indices.
    void SetMaxIndices(unsigned num);
    /// Set whether to optimize GPU buffer sizes according to current amount of decals. Default false, which will size the buffers according to the maximum vertices/indices. When true, buffers will be reallocated whenever decals are added/removed, which can be worse for performance.
    void SetOptimizeBufferSize(bool enable);
    /// Add a decal at world coordinates, using a target drawable's geometry for reference. If the decal needs to move with the target, the decal component should be created to the target's node. Return true if successful.
    bool AddDecal(Drawable* target, const Vector3& worldPosition, const Quaternion& worldRotation, float size, float aspectRatio,
        float depth, const Vector2& topLeftUV, const Vector2& bottomRightUV, float timeToLive = 0.0f, float normalCutoff = 0.1f,
        unsigned subGeometry = M_MAX_UNSIGNED);
    /// Remove n oldest decals.
    void RemoveDecals(unsigned num);
    /// Remove all decals.
    void RemoveAllDecals();

    /// Return material.
    Material* GetMaterial() const;

    /// Return number of decals.
    unsigned GetNumDecals() const { return decals_.Size(); }

    /// Retur number of vertices in the decals.
    unsigned GetNumVertices() const { return numVertices_; }

    /// Retur number of vertex indices in the decals.
    unsigned GetNumIndices() const { return numIndices_; }

    /// Return maximum number of decal vertices.
    unsigned GetMaxVertices() const { return maxVertices_; }

    /// Return maximum number of decal vertex indices.
    unsigned GetMaxIndices() const { return maxIndices_; }

    /// Return whether is optimizing GPU buffer sizes according to current amount of decals.
    bool GetOptimizeBufferSize() const { return optimizeBufferSize_; }

    /// Set material attribute.
    void SetMaterialAttr(const ResourceRef& value);
    /// Set decals attribute.
    void SetDecalsAttr(const Vector<byte>& value);
    /// Return material attribute.
    ResourceRef GetMaterialAttr() const;
    /// Return decals attribute.
    Vector<byte> GetDecalsAttr() const;

protected:
    /// Recalculate the world-space bounding box.
    void OnWorldBoundingBoxUpdate() override;
    /// Handle node transform being dirtied.
    void OnMarkedDirty(Node* node) override;

private:
    /// Get triangle faces from the target geometry.
    void GetFaces(Vector<Vector<DecalVertex>>& faces, Drawable* target, unsigned batchIndex, const Frustum& frustum,
        const Vector3& decalNormal, float normalCutoff);
    /// Get triangle face from the target geometry.
    void GetFace
        (Vector<Vector<DecalVertex>>& faces, Drawable* target, unsigned batchIndex, unsigned i0, unsigned i1, unsigned i2,
            const byte* positionData, const byte* normalData, const byte* skinningData,
            unsigned positionStride, unsigned normalStride, unsigned skinningStride, const Frustum& frustum,
            const Vector3& decalNormal, float normalCutoff);
    /// Get bones referenced by skinning data and remap the skinning indices. Return true if successful.
    bool GetBones(Drawable* target, unsigned batchIndex, const float* blendWeights, const unsigned char* blendIndices,
        unsigned char* newBlendIndices);
    /// Calculate UV coordinates for the decal.
    void CalculateUVs
        (Decal& decal, const Matrix3x4& view, const Matrix4& projection, const Vector2& topLeftUV, const Vector2& bottomRightUV);
    /// Transform decal's vertices from the target geometry to the decal set local space.
    void TransformVertices(Decal& decal, const Matrix3x4& transform);
    /// Remove a decal by iterator and return iterator to the next decal.
    List<Decal>::Iterator RemoveDecal(List<Decal>::Iterator i);
    /// Mark decals and the bounding box dirty.
    void MarkDecalsDirty();
    /// Recalculate the local-space bounding box.
    void CalculateBoundingBox();
    /// Rewrite decal vertex and index buffers.
    void UpdateBuffers();
    /// Recalculate skinning.
    void UpdateSkinning();
    /// Update the batch (geometry type, shader data).
    void UpdateBatch();
    /// Find bones after loading.
    void AssignBoneNodes();
    /// Subscribe/unsubscribe from scene post-update as necessary.
    void UpdateEventSubscription(bool checkAllDecals);
    /// Handle scene post-update event.
    void HandleScenePostUpdate(StringHash eventType, VariantMap& eventData);

    /// Geometry.
    SharedPtr<Geometry> geometry_;
    /// Vertex buffer.
    std::shared_ptr<VertexBuffer> vertexBuffer_;
    /// Index buffer.
    std::shared_ptr<IndexBuffer> indexBuffer_;
    /// Decals.
    List<Decal> decals_;
    /// Bones used for skinned decals.
    Vector<Bone> bones_;
    /// Skinning matrices.
    Vector<Matrix3x4> skinMatrices_;
    /// Vertices in the current decals.
    unsigned numVertices_;
    /// Indices in the current decals.
    unsigned numIndices_;
    /// Maximum vertices.
    unsigned maxVertices_;
    /// Maximum indices.
    unsigned maxIndices_;
    /// Optimize buffer sizes flag.
    bool optimizeBufferSize_;
    /// Skinned mode flag.
    bool skinned_;
    /// Vertex buffer needs rewrite / resizing flag.
    bool bufferDirty_;
    /// Bounding box needs update flag.
    bool boundingBoxDirty_;
    /// Skinning dirty flag.
    bool skinningDirty_;
    /// Bone nodes assignment pending flag.
    bool assignBonesPending_;
    /// Subscribed to scene post update event flag.
    bool subscribed_;
};

}
