// Copyright (c) 2008-2023 the Urho3D project
// Copyright (c) 2022-2023 the Dviglo project
// License: MIT

/// \file

#pragma once

#include "../math/bounding_box.h"
#include "../math/quaternion.h"
#include "../scene/component.h"

#include <memory>

class btBvhTriangleMeshShape;
class btCollisionShape;
class btCompoundShape;
class btGImpactMeshShape;
class btTriangleMesh;

struct btTriangleInfoMap;

namespace dviglo
{

class CustomGeometry;
class Geometry;
class Model;
class PhysicsWorld;
class RigidBody;
class Terrain;
class TriangleMeshInterface;

/// Collision shape type.
enum ShapeType
{
    SHAPE_BOX = 0,
    SHAPE_SPHERE,
    SHAPE_STATICPLANE,
    SHAPE_CYLINDER,
    SHAPE_CAPSULE,
    SHAPE_CONE,
    SHAPE_TRIANGLEMESH,
    SHAPE_CONVEXHULL,
    SHAPE_TERRAIN,
    SHAPE_GIMPACTMESH
};

/// Base class for collision shape geometry data.
struct CollisionGeometryData : public RefCounted
{
};

/// Cache of collision geometry data.
/// \todo Remove duplicate declaration
using CollisionGeometryDataCache = HashMap<Pair<Model*, i32>, SharedPtr<CollisionGeometryData>>;

/// Triangle mesh geometry data.
struct TriangleMeshData : public CollisionGeometryData
{
    /// Construct from a model.
    TriangleMeshData(Model* model, i32 lodLevel);
    /// Construct from a custom geometry.
    explicit TriangleMeshData(CustomGeometry* custom);

    /// Bullet triangle mesh interface.
    std::unique_ptr<TriangleMeshInterface> meshInterface_;

    /// Bullet triangle mesh collision shape.
    std::unique_ptr<btBvhTriangleMeshShape> shape_;

    /// Bullet triangle info map.
    std::unique_ptr<btTriangleInfoMap> infoMap_;
};

/// Triangle mesh geometry data.
struct GImpactMeshData : public CollisionGeometryData
{
    /// Construct from a model.
    GImpactMeshData(Model* model, i32 lodLevel);
    /// Construct from a custom geometry.
    explicit GImpactMeshData(CustomGeometry* custom);

    /// Bullet triangle mesh interface.
    std::unique_ptr<TriangleMeshInterface> meshInterface_;
};

/// Convex hull geometry data.
struct ConvexData : public CollisionGeometryData
{
    /// Construct from a model.
    ConvexData(Model* model, i32 lodLevel);
    /// Construct from a custom geometry.
    explicit ConvexData(CustomGeometry* custom);

    /// Build the convex hull from vertices.
    void BuildHull(const Vector<Vector3>& vertices);

    /// Vertex data.
    std::unique_ptr<Vector3[]> vertexData_;
    /// Number of vertices.
    unsigned vertexCount_{};
    /// Index data.
    std::unique_ptr<unsigned[]> indexData_;
    /// Number of indices.
    unsigned indexCount_{};
};

/// Heightfield geometry data.
struct HeightfieldData : public CollisionGeometryData
{
    /// Construct from a terrain.
    HeightfieldData(Terrain* terrain, i32 lodLevel);

    /// Height data. On LOD level 0 the original height data will be used.
    std::shared_ptr<float[]> heightData_;
    /// Vertex spacing.
    Vector3 spacing_;
    /// Heightmap size.
    IntVector2 size_;
    /// Minimum height.
    float minHeight_;
    /// Maximum height.
    float maxHeight_;
};

/// Physics collision shape component.
class DV_API CollisionShape : public Component
{
    DV_OBJECT(CollisionShape);

public:
    /// Construct.
    explicit CollisionShape();
    /// Destruct. Free the geometry data and clean up unused data from the geometry data cache.
    ~CollisionShape() override;
    /// Register object factory.
    static void register_object();

    /// Apply attribute changes that can not be applied immediately. Called after scene load or a network update.
    void apply_attributes() override;
    /// Handle enabled/disabled state change.
    void OnSetEnabled() override;
    /// Visualize the component as debug geometry.
    void draw_debug_geometry(DebugRenderer* debug, bool depthTest) override;

    /// Set as a box.
    void SetBox(const Vector3& size, const Vector3& position = Vector3::ZERO, const Quaternion& rotation = Quaternion::IDENTITY);
    /// Set as a sphere.
    void SetSphere(float diameter, const Vector3& position = Vector3::ZERO, const Quaternion& rotation = Quaternion::IDENTITY);
    /// Set as a static plane.
    void SetStaticPlane(const Vector3& position = Vector3::ZERO, const Quaternion& rotation = Quaternion::IDENTITY);
    /// Set as a cylinder.
    void SetCylinder(float diameter, float height, const Vector3& position = Vector3::ZERO, const Quaternion& rotation = Quaternion::IDENTITY);
    /// Set as a capsule.
    void SetCapsule(float diameter, float height, const Vector3& position = Vector3::ZERO, const Quaternion& rotation = Quaternion::IDENTITY);
    /// Set as a cone.
    void SetCone(float diameter, float height, const Vector3& position = Vector3::ZERO, const Quaternion& rotation = Quaternion::IDENTITY);
    /// Set as a triangle mesh from Model. If you update a model's geometry and want to reapply the shape, call physicsWorld->RemoveCachedGeometry(model) first.
    void SetTriangleMesh(Model* model, i32 lodLevel = 0, const Vector3& scale = Vector3::ONE, const Vector3& position = Vector3::ZERO,
        const Quaternion& rotation = Quaternion::IDENTITY);
    /// Set as a triangle mesh from CustomGeometry.
    void SetCustomTriangleMesh(CustomGeometry* custom, const Vector3& scale = Vector3::ONE, const Vector3& position = Vector3::ZERO,
        const Quaternion& rotation = Quaternion::IDENTITY);
    /// Set as a convex hull from Model.
    void SetConvexHull(Model* model, i32 lodLevel = 0, const Vector3& scale = Vector3::ONE, const Vector3& position = Vector3::ZERO,
        const Quaternion& rotation = Quaternion::IDENTITY);
    /// Set as a convex hull from CustomGeometry.
    void SetCustomConvexHull(CustomGeometry* custom, const Vector3& scale = Vector3::ONE, const Vector3& position = Vector3::ZERO,
        const Quaternion& rotation = Quaternion::IDENTITY);
    /// Set as a triangle mesh from Model. If you update a model's geometry and want to reapply the shape, call physicsWorld->RemoveCachedGeometry(model) first.
    void SetGImpactMesh(Model* model, i32 lodLevel = 0, const Vector3& scale = Vector3::ONE, const Vector3& position = Vector3::ZERO,
        const Quaternion& rotation = Quaternion::IDENTITY);
    /// Set as a triangle mesh from CustomGeometry.
    void SetCustomGImpactMesh(CustomGeometry* custom, const Vector3& scale = Vector3::ONE, const Vector3& position = Vector3::ZERO,
        const Quaternion& rotation = Quaternion::IDENTITY);
    /// Set as a terrain. Only works if the same scene node contains a Terrain component.
    void SetTerrain(i32 lodLevel = 0);
    /// Set shape type.
    void SetShapeType(ShapeType type);
    /// Set shape size.
    void SetSize(const Vector3& size);
    /// Set offset position.
    void SetPosition(const Vector3& position);
    /// Set offset rotation.
    void SetRotation(const Quaternion& rotation);
    /// Set offset transform.
    void SetTransform(const Vector3& position, const Quaternion& rotation);
    /// Set collision margin.
    void SetMargin(float margin);
    /// Set triangle mesh / convex hull model.
    void SetModel(Model* model);
    /// Set model LOD level.
    void SetLodLevel(i32 lodLevel);

    /// Return Bullet collision shape.
    btCollisionShape* GetCollisionShape() const { return shape_.get(); }

    /// Return the shared geometry data.
    CollisionGeometryData* GetGeometryData() const { return geometry_; }

    /// Return physics world.
    PhysicsWorld* GetPhysicsWorld() const { return physicsWorld_; }

    /// Return shape type.
    ShapeType GetShapeType() const { return shapeType_; }

    /// Return shape size.
    const Vector3& GetSize() const { return size_; }

    /// Return offset position.
    const Vector3& GetPosition() const { return position_; }

    /// Return offset rotation.
    const Quaternion& GetRotation() const { return rotation_; }

    /// Return collision margin.
    float GetMargin() const { return margin_; }

    /// Return triangle mesh / convex hull model.
    Model* GetModel() const { return model_; }

    /// Return model LOD level.
    i32 GetLodLevel() const { return lodLevel_; }

    /// Return world-space bounding box.
    BoundingBox GetWorldBoundingBox() const;

    /// Update the new collision shape to the RigidBody.
    void NotifyRigidBody(bool updateMass = true);
    /// Set model attribute.
    void SetModelAttr(const ResourceRef& value);
    /// Return model attribute.
    ResourceRef GetModelAttr() const;
    /// Release the collision shape.
    void ReleaseShape();

protected:
    /// Handle node being assigned.
    void OnNodeSet(Node* node) override;
    /// Handle scene being assigned.
    void OnSceneSet(Scene* scene) override;
    /// Handle node transform being dirtied.
    void OnMarkedDirty(Node* node) override;
    /**
     * Called when instantiating a collision shape that is not one of ShapeType (default no-op).
     *
     * Useful for custom shape types that subclass CollisionShape and use a non-standard underlying
     * btCollisionShape. UpdateDerivedShape can then be overridden to create the required
     * btCollisionShape subclass.
     */
    virtual btCollisionShape* UpdateDerivedShape(int shapeType, const Vector3& newWorldScale);

private:
    /// Find the parent rigid body component and return its compound collision shape.
    btCompoundShape* GetParentCompoundShape();
    /// Update the collision shape after attribute changes.
    void UpdateShape();
    /// Update cached geometry collision shape.
    void UpdateCachedGeometryShape(CollisionGeometryDataCache& cache);
    /// Set as specified shape type using model and LOD.
    void SetModelShape(ShapeType shapeType, Model* model, i32 lodLevel,
        const Vector3& scale, const Vector3& position, const Quaternion& rotation);
    /// Set as specified shape type using CustomGeometry.
    void SetCustomShape(ShapeType shapeType, CustomGeometry* custom,
        const Vector3& scale, const Vector3& position, const Quaternion& rotation);
    /// Update terrain collision shape from the terrain component.
    void HandleTerrainCreated(StringHash eventType, VariantMap& eventData);
    /// Update trimesh or convex shape after a model has reloaded itself.
    void HandleModelReloadFinished(StringHash eventType, VariantMap& eventData);
    /// Mark shape dirty.
    void MarkShapeDirty() { recreateShape_ = true; }

    /// Physics world.
    WeakPtr<PhysicsWorld> physicsWorld_;

    /// Rigid body.
    WeakPtr<RigidBody> rigidBody_;

    /// Model.
    SharedPtr<Model> model_;

    /// Shared geometry data.
    SharedPtr<CollisionGeometryData> geometry_;

    /// Bullet collision shape.
    std::unique_ptr<btCollisionShape> shape_;

    /// Collision shape type.
    ShapeType shapeType_;

    /// Offset position.
    Vector3 position_;
    /// Offset rotation.
    Quaternion rotation_;
    /// Shape size.
    Vector3 size_;
    /// Cached world scale for determining if the collision shape needs update.
    Vector3 cachedWorldScale_;
    /// Model LOD level.
    i32 lodLevel_;
    /// CustomGeometry component ID. 0 if not creating the convex hull / triangle mesh from a CustomGeometry.
    unsigned customGeometryID_;
    /// Collision margin.
    float margin_;
    /// Recreate collision shape flag.
    bool recreateShape_;
    /// Shape creation retry flag if attributes initially set without scene.
    bool retryCreation_;
};

}
