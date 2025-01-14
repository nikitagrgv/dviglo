// Copyright (c) 2008-2023 the Urho3D project
// Copyright (c) 2022-2023 the Dviglo project
// License: MIT

#include "../core/context.h"
#include "camera.h"
#include "debug_renderer.h"
#include "geometry.h"
#include "material.h"
#include "occlusion_buffer.h"
#include "octree_query.h"
#include "terrain.h"
#include "terrain_patch.h"
#include "../graphics_api/index_buffer.h"
#include "../graphics_api/vertex_buffer.h"
#include "../io/log.h"
#include "../scene/node.h"

#include "../common/debug_new.h"

using namespace std;

namespace dviglo
{

static const float LOD_CONSTANT = 1.0f / 150.0f;

extern const char* GEOMETRY_CATEGORY;

TerrainPatch::TerrainPatch() :
    Drawable(DrawableTypes::Geometry),
    geometry_(new Geometry()),
    maxLodGeometry_(new Geometry()),
    occlusionGeometry_(new Geometry()),
    vertexBuffer_(make_shared<VertexBuffer>()),
    coordinates_(IntVector2::ZERO),
    lodLevel_(0)
{
    geometry_->SetVertexBuffer(0, vertexBuffer_);
    maxLodGeometry_->SetVertexBuffer(0, vertexBuffer_);
    occlusionGeometry_->SetVertexBuffer(0, vertexBuffer_);

    batches_.Resize(1);
    batches_[0].geometry_ = geometry_;
    batches_[0].geometryType_ = GEOM_STATIC_NOINSTANCING;
}

TerrainPatch::~TerrainPatch() = default;

void TerrainPatch::register_object()
{
    DV_CONTEXT->RegisterFactory<TerrainPatch>();
}

void TerrainPatch::ProcessRayQuery(const RayOctreeQuery& query, Vector<RayQueryResult>& results)
{
    RayQueryLevel level = query.level_;

    switch (level)
    {
    case RAY_AABB:
        Drawable::ProcessRayQuery(query, results);
        break;

    case RAY_OBB:
    case RAY_TRIANGLE:
        {
            Matrix3x4 inverse(node_->GetWorldTransform().Inverse());
            Ray localRay = query.ray_.Transformed(inverse);
            float distance = localRay.HitDistance(boundingBox_);
            Vector3 normal = -query.ray_.direction_;

            if (level == RAY_TRIANGLE && distance < query.maxDistance_)
            {
                Vector3 geometryNormal;
                distance = geometry_->GetHitDistance(localRay, &geometryNormal);
                normal = (node_->GetWorldTransform() * Vector4(geometryNormal, 0.0f)).normalized();
            }

            if (distance < query.maxDistance_)
            {
                RayQueryResult result;
                result.position_ = query.ray_.origin_ + distance * query.ray_.direction_;
                result.normal_ = normal;
                result.distance_ = distance;
                result.drawable_ = this;
                result.node_ = node_;
                result.subObject_ = NINDEX;
                results.Push(result);
            }
        }
        break;

    case RAY_TRIANGLE_UV:
        DV_LOGWARNING("RAY_TRIANGLE_UV query level is not supported for TerrainPatch component");
        break;
    }
}

void TerrainPatch::update_batches(const FrameInfo& frame)
{
    const Matrix3x4& worldTransform = node_->GetWorldTransform();
    distance_ = frame.camera_->GetDistance(GetWorldBoundingBox().Center());

    float scale = worldTransform.Scale().DotProduct(DOT_SCALE);
    lodDistance_ = frame.camera_->GetLodDistance(distance_, scale, lodBias_);

    batches_[0].distance_ = distance_;
    batches_[0].worldTransform_ = &worldTransform;

    i32 newLodLevel = 0;
    for (i32 i = 0; i < lodErrors_.Size(); ++i)
    {
        if (lodErrors_[i] / lodDistance_ > LOD_CONSTANT)
            break;
        else
            newLodLevel = i;
    }

    lodLevel_ = GetCorrectedLodLevel(newLodLevel);
}

void TerrainPatch::UpdateGeometry(const FrameInfo& frame)
{
    if (vertexBuffer_->IsDataLost())
    {
        if (owner_)
            owner_->CreatePatchGeometry(this);
        else
            vertexBuffer_->ClearDataLost();
    }

    if (owner_)
        owner_->UpdatePatchLod(this);
}

UpdateGeometryType TerrainPatch::GetUpdateGeometryType()
{
    // Because there is a latency in starting worker thread updates, and the update of terrain patch LOD should not take
    // much time, always update in the main thread
    return UPDATE_MAIN_THREAD;
}

Geometry* TerrainPatch::GetLodGeometry(i32 batchIndex, i32 level)
{
    assert(batchIndex >= 0);
    assert(level >= 0 || level == NINDEX);

    if (!level)
        return maxLodGeometry_;
    else
        return geometry_;
}

i32 TerrainPatch::GetNumOccluderTriangles()
{
    // Check that the material is suitable for occlusion (default material always is)
    Material* mat = batches_[0].material_;
    if (mat && !mat->GetOcclusion())
        return 0;
    else
        return occlusionGeometry_->GetIndexCount() / 3;
}

bool TerrainPatch::DrawOcclusion(OcclusionBuffer* buffer)
{
    // Check that the material is suitable for occlusion (default material always is) and set culling mode
    Material* material = batches_[0].material_;
    if (material)
    {
        if (!material->GetOcclusion())
            return true;
        buffer->SetCullMode(material->GetCullMode());
    }
    else
        buffer->SetCullMode(CULL_CCW);

    const byte* vertexData;
    i32 vertexSize;
    const byte* indexData;
    i32 indexSize;
    const Vector<VertexElement>* elements;

    occlusionGeometry_->GetRawData(vertexData, vertexSize, indexData, indexSize, elements);
    // Check for valid geometry data
    if (!vertexData || !indexData || !elements || VertexBuffer::GetElementOffset(*elements, TYPE_VECTOR3, SEM_POSITION) != 0)
        return false;

    // Draw and check for running out of triangles
    return buffer->AddTriangles(node_->GetWorldTransform(), vertexData, vertexSize, indexData, indexSize, occlusionGeometry_->GetIndexStart(),
        occlusionGeometry_->GetIndexCount());
}

void TerrainPatch::draw_debug_geometry(DebugRenderer* debug, bool depthTest)
{
    // Intentionally no operation
}

void TerrainPatch::SetOwner(Terrain* terrain)
{
    owner_ = terrain;
}

void TerrainPatch::SetNeighbors(TerrainPatch* north, TerrainPatch* south, TerrainPatch* west, TerrainPatch* east)
{
    north_ = north;
    south_ = south;
    west_ = west;
    east_ = east;
}

void TerrainPatch::SetMaterial(Material* material)
{
    batches_[0].material_ = material;
}

void TerrainPatch::SetBoundingBox(const BoundingBox& box)
{
    boundingBox_ = box;
    OnMarkedDirty(node_);
}

void TerrainPatch::SetCoordinates(const IntVector2& coordinates)
{
    coordinates_ = coordinates;
}

void TerrainPatch::ResetLod()
{
    lodLevel_ = 0;
}

Geometry* TerrainPatch::GetGeometry() const
{
    return geometry_;
}

Geometry* TerrainPatch::GetMaxLodGeometry() const
{
    return maxLodGeometry_;
}

Geometry* TerrainPatch::GetOcclusionGeometry() const
{
    return occlusionGeometry_;
}

shared_ptr<VertexBuffer> TerrainPatch::GetVertexBuffer() const
{
    return vertexBuffer_;
}

Terrain* TerrainPatch::GetOwner() const
{
    return owner_;
}

void TerrainPatch::OnWorldBoundingBoxUpdate()
{
    worldBoundingBox_ = boundingBox_.Transformed(node_->GetWorldTransform());
}

i32 TerrainPatch::GetCorrectedLodLevel(i32 lodLevel)
{
    assert(lodLevel >= 0);

    if (north_)
        lodLevel = Min(lodLevel, north_->GetLodLevel() + 1);
    if (south_)
        lodLevel = Min(lodLevel, south_->GetLodLevel() + 1);
    if (west_)
        lodLevel = Min(lodLevel, west_->GetLodLevel() + 1);
    if (east_)
        lodLevel = Min(lodLevel, east_->GetLodLevel() + 1);

    return lodLevel;
}

}
