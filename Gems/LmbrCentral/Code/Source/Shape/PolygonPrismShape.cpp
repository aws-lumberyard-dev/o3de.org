/*
* All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
* its licensors.
*
* For complete copyright and license terms please see the LICENSE at the root of this
* distribution (the "License"). All use of this software is governed by the License,
* or, if provided, by the license below or the license accompanying this file. Do not
* remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*
*/

#include "LmbrCentral_precompiled.h"
#include "PolygonPrismShape.h"

#include <AzCore/Math/Aabb.h>
#include <AzCore/Math/IntersectSegment.h>
#include <AzCore/Math/Transform.h>
#include <AzCore/Math/VectorConversions.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/std/smart_ptr/make_shared.h>
#include <AzFramework/Entity/EntityDebugDisplayBus.h>
#include <MathConversion.h>
#include <Shape/ShapeGeometryUtil.h>
#include <Shape/ShapeDisplay.h>
#include <ISystem.h>
#include <IRenderAuxGeom.h>

namespace LmbrCentral
{
    /// Generates solid polygon prism mesh.
    /// Applies non-uniform scale, but does not apply any scale from the transform, which is assumed to be applied separately elsewhere.
    static void GenerateSolidPolygonPrismMesh(
        const AZStd::vector<AZ::Vector2>& vertices,
        const float height,
        const AZ::Vector3& nonUniformScale,
        AZStd::vector<AZ::Vector3>& meshTriangles)
    {
        // must have at least one triangle
        if (vertices.size() < 3)
        {
            meshTriangles.clear();
            return;
        }

        // deal with the possibility that the scaled height is negative
        const float scaledHeight = height * nonUniformScale.GetZ();
        const float top = AZ::GetMax(0.0f, scaledHeight);
        const float bottom = AZ::GetMin(0.0f, scaledHeight);
        const AZ::Vector3 topVector = AZ::Vector3::CreateAxisZ(top);
        const AZ::Vector3 bottomVector = AZ::Vector3::CreateAxisZ(bottom);

        // generate triangles for one face of polygon prism
        const AZStd::vector<AZ::Vector3> faceTriangles = GenerateTriangles(vertices);

        const size_t halfTriangleCount = faceTriangles.size();
        const size_t fullTriangleCount = faceTriangles.size() * 2;
        const size_t wallTriangleCount = vertices.size() * 2 * 3;

        // allocate space for both faces (polygons) and walls
        meshTriangles.resize_no_construct(fullTriangleCount + wallTriangleCount);

        // copy vertices into triangle list
        const typename AZStd::vector<AZ::Vector3>::iterator midFace = meshTriangles.begin() + halfTriangleCount;

        AZStd::transform(faceTriangles.cbegin(), faceTriangles.cend(), meshTriangles.begin(),
            [&nonUniformScale, &topVector](AZ::Vector3 vertex) { return nonUniformScale * vertex + topVector; });
        // due to winding order, reverse copy triangles for other face/polygon
        AZStd::transform(faceTriangles.crbegin(), faceTriangles.crend(), midFace,
            [&nonUniformScale, &bottomVector](AZ::Vector3 vertex) { return nonUniformScale * vertex + bottomVector; });

        // end of face/polygon vertices is start of side/wall vertices
        const typename AZStd::vector<AZ::Vector3>::iterator endFaceIt = meshTriangles.begin() + fullTriangleCount;
        typename AZStd::vector<AZ::Vector3>::iterator sideIt = endFaceIt;

        // build quad triangles from vertices util
        const auto quadTriangles =
            [](const AZ::Vector3& a, const AZ::Vector3& b, const AZ::Vector3& c, const AZ::Vector3& d, typename AZStd::vector<AZ::Vector3>::iterator& tri)
        {
            *tri = a; ++tri;
            *tri = b; ++tri;
            *tri = c; ++tri;
            *tri = c; ++tri;
            *tri = b; ++tri;
            *tri = d; ++tri;
        };

        // generate walls
        const bool clockwise = ClockwiseOrder(vertices);
        const size_t vertexCount = vertices.size();
        for (size_t i = 0; i < vertexCount; ++i)
        {
            // local vertex positions
            const AZ::Vector3 currentPoint = nonUniformScale * AZ::Vector2ToVector3(vertices[i]);
            const AZ::Vector3 nextPoint = nonUniformScale * AZ::Vector2ToVector3(vertices[(i + 1) % vertexCount]);
            const AZ::Vector3 p1 = currentPoint + bottomVector;
            const AZ::Vector3 p2 = nextPoint + bottomVector;
            const AZ::Vector3 p3 = currentPoint + topVector;
            const AZ::Vector3 p4 = nextPoint + topVector;

            // generate triangles for wall quad
            if (clockwise)
            {
                quadTriangles(p1, p3, p2, p4, sideIt);
            }
            else
            {
                quadTriangles(p1, p2, p3, p4, sideIt);
            }
        }
    }

    static void GenerateWirePolygonPrismMesh(
        const AZStd::vector<AZ::Vector2>& vertices,
        const float height,
        const AZ::Vector3& nonUniformScale,
        AZStd::vector<AZ::Vector3>& lines)
    {
        const size_t vertexCount = vertices.size();
        const size_t verticalLineCount = vertexCount;
        const size_t horizontalLineCount = vertexCount > 2
            ? vertexCount
            : vertexCount > 1
            ? 1
            : 0;

        lines.resize((verticalLineCount + (horizontalLineCount * 2)) * 2);

        size_t lineVertIndex = 0;
        for (size_t i = 0; i < verticalLineCount; ++i)
        {
            // vertical line
            lines[lineVertIndex++] = nonUniformScale * AZ::Vector2ToVector3(vertices[i]);
            lines[lineVertIndex++] = nonUniformScale * AZ::Vector2ToVector3(vertices[i], height);
        }

        for (size_t i = 0; i < horizontalLineCount; ++i)
        {
            // bottom line
            lines[lineVertIndex++] = nonUniformScale * AZ::Vector2ToVector3(vertices[i]);
            lines[lineVertIndex++] = nonUniformScale * AZ::Vector2ToVector3(vertices[(i + 1) % vertexCount]);
        }

        for (size_t i = 0; i < horizontalLineCount; ++i)
        {
            // top line
            lines[lineVertIndex++] = nonUniformScale * AZ::Vector2ToVector3(vertices[i], height);
            lines[lineVertIndex++] = nonUniformScale * AZ::Vector2ToVector3(vertices[(i + 1) % vertexCount], height);
        }
    }

    void GeneratePolygonPrismMesh(
        const AZStd::vector<AZ::Vector2>& vertices, const float height, const AZ::Vector3& nonUniformScale,
        PolygonPrismMesh& polygonPrismMeshOut)
    {
        GenerateSolidPolygonPrismMesh(
            vertices, height, nonUniformScale, polygonPrismMeshOut.m_triangles);
        GenerateWirePolygonPrismMesh(
            vertices, height, nonUniformScale, polygonPrismMeshOut.m_lines);
    }

    PolygonPrismShape::PolygonPrismShape()
        : m_polygonPrism(AZStd::make_shared<AZ::PolygonPrism>())
        , m_nonUniformScaleChangedHandler([this](const AZ::Vector3& scale) {this->OnNonUniformScaleChanged(scale); })
    {
    }

    PolygonPrismShape::PolygonPrismShape(const PolygonPrismShape& other)
        : m_polygonPrism(other.m_polygonPrism)
        , m_intersectionDataCache(other.m_intersectionDataCache)
        , m_currentTransform(other.m_currentTransform)
        , m_entityId(other.m_entityId)
    {

    }

    PolygonPrismShape& PolygonPrismShape::operator=(const PolygonPrismShape& other)
    {
        m_polygonPrism = other.m_polygonPrism;
        m_intersectionDataCache = other.m_intersectionDataCache;
        m_currentTransform = other.m_currentTransform;
        m_entityId = other.m_entityId;
        return *this;
    }

    void PolygonPrismShape::Reflect(AZ::ReflectContext* context)
    {
        PolygonPrismShapeConfig::Reflect(context);

        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<PolygonPrismShape>()
                ->Version(1)
                ->Field("PolygonPrism", &PolygonPrismShape::m_polygonPrism)
                ;

            if (AZ::EditContext* editContext = serializeContext->GetEditContext())
            {
                editContext->Class<PolygonPrismShape>("Configuration", "Polygon Prism configuration parameters")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)
                        ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &PolygonPrismShape::m_polygonPrism,
                        "Polygon Prism", "Data representing the shape in the entity's local coordinate space.")
                        ->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)
                        ->Attribute(AZ::Edit::Attributes::ContainerCanBeModified, false)
                        ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                        ;
            }
        }
    }

    void PolygonPrismShape::Activate(AZ::EntityId entityId)
    {
        m_entityId = entityId;
        m_currentTransform = AZ::Transform::CreateIdentity();
        AZ::TransformBus::EventResult(m_currentTransform, entityId, &AZ::TransformBus::Events::GetWorldTM);
        m_currentNonUniformScale = AZ::Vector3::CreateOne();
        AZ::NonUniformScaleRequestBus::EventResult(m_currentNonUniformScale, m_entityId, &AZ::NonUniformScaleRequests::GetScale);
        m_polygonPrism->SetNonUniformScale(m_currentNonUniformScale);
        m_intersectionDataCache.InvalidateCache(InvalidateShapeCacheReason::ShapeChange);

        AZ::TransformNotificationBus::Handler::BusConnect(entityId);
        ShapeComponentRequestsBus::Handler::BusConnect(entityId);
        PolygonPrismShapeComponentRequestBus::Handler::BusConnect(entityId);
        AZ::VariableVerticesRequestBus<AZ::Vector2>::Handler::BusConnect(entityId);
        AZ::FixedVerticesRequestBus<AZ::Vector2>::Handler::BusConnect(entityId);

        AZ::NonUniformScaleRequestBus::Event(m_entityId, &AZ::NonUniformScaleRequests::RegisterScaleChangedEvent,
            m_nonUniformScaleChangedHandler);

        const auto polygonPrismChanged = [this]()
        {
            ShapeChanged();
        };

        m_polygonPrism->SetCallbacks(
            polygonPrismChanged,
            polygonPrismChanged,
            polygonPrismChanged,
            polygonPrismChanged);
    }

    void PolygonPrismShape::Deactivate()
    {
        m_nonUniformScaleChangedHandler.Disconnect();
        PolygonPrismShapeComponentRequestBus::Handler::BusDisconnect();
        AZ::FixedVerticesRequestBus<AZ::Vector2>::Handler::BusDisconnect();
        AZ::VariableVerticesRequestBus<AZ::Vector2>::Handler::BusDisconnect();
        ShapeComponentRequestsBus::Handler::BusDisconnect();
        AZ::TransformNotificationBus::Handler::BusDisconnect();
    }

    void PolygonPrismShape::InvalidateCache(InvalidateShapeCacheReason reason)
    {
        m_intersectionDataCache.InvalidateCache(reason);
    }

    void PolygonPrismShape::OnTransformChanged(const AZ::Transform& /*local*/, const AZ::Transform& world)
    {
        m_currentTransform = world;
        m_intersectionDataCache.InvalidateCache(InvalidateShapeCacheReason::TransformChange);
        ShapeComponentNotificationsBus::Event(
            m_entityId, &ShapeComponentNotificationsBus::Events::OnShapeChanged,
            ShapeComponentNotifications::ShapeChangeReasons::TransformChanged);
    }

    void PolygonPrismShape::OnNonUniformScaleChanged(const AZ::Vector3& scale)
    {
        m_currentNonUniformScale = scale;
        m_polygonPrism->SetNonUniformScale(scale);
        m_intersectionDataCache.InvalidateCache(InvalidateShapeCacheReason::ShapeChange);
        ShapeComponentNotificationsBus::Event(
            m_entityId, &ShapeComponentNotificationsBus::Events::OnShapeChanged,
            ShapeComponentNotifications::ShapeChangeReasons::ShapeChanged);
    }

    void PolygonPrismShape::ShapeChanged()
    {
        ShapeComponentNotificationsBus::Event(
            m_entityId, &ShapeComponentNotificationsBus::Events::OnShapeChanged,
            ShapeComponentNotifications::ShapeChangeReasons::ShapeChanged);

        m_intersectionDataCache.InvalidateCache(
            InvalidateShapeCacheReason::ShapeChange);
    }

    AZ::PolygonPrismPtr PolygonPrismShape::GetPolygonPrism()
    {
        return m_polygonPrism;
    }

    bool PolygonPrismShape::GetVertex(const size_t index, AZ::Vector2& vertex) const
    {
        return m_polygonPrism->m_vertexContainer.GetVertex(index, vertex);
    }

    void PolygonPrismShape::AddVertex(const AZ::Vector2& vertex)
    {
        m_polygonPrism->m_vertexContainer.AddVertex(vertex);
    }

    bool PolygonPrismShape::UpdateVertex(const size_t index, const AZ::Vector2& vertex)
    {
        return m_polygonPrism->m_vertexContainer.UpdateVertex(index, vertex);
    }

    bool PolygonPrismShape::InsertVertex(const size_t index, const AZ::Vector2& vertex)
    {
        return m_polygonPrism->m_vertexContainer.InsertVertex(index, vertex);
    }

    bool PolygonPrismShape::RemoveVertex(const size_t index)
    {
        return m_polygonPrism->m_vertexContainer.RemoveVertex(index);
    }

    void PolygonPrismShape::SetVertices(const AZStd::vector<AZ::Vector2>& vertices)
    {
        m_polygonPrism->m_vertexContainer.SetVertices(vertices);
    }

    void PolygonPrismShape::ClearVertices()
    {
        m_polygonPrism->m_vertexContainer.Clear();
    }

    size_t PolygonPrismShape::Size() const
    {
        return m_polygonPrism->m_vertexContainer.Size();
    }

    bool PolygonPrismShape::Empty() const
    {
        return m_polygonPrism->m_vertexContainer.Empty();
    }

    void PolygonPrismShape::SetHeight(const float height)
    {
        m_polygonPrism->SetHeight(height);
        m_intersectionDataCache.InvalidateCache(InvalidateShapeCacheReason::ShapeChange);
    }

    AZ::Aabb PolygonPrismShape::GetEncompassingAabb()
    {
        m_intersectionDataCache.UpdateIntersectionParams(m_currentTransform, *m_polygonPrism, m_currentNonUniformScale);

        return m_intersectionDataCache.m_aabb;
    }

    void PolygonPrismShape::GetTransformAndLocalBounds(AZ::Transform& transform, AZ::Aabb& bounds)
    {
        bounds = PolygonPrismUtil::CalculateAabb(*m_polygonPrism, AZ::Transform::Identity());
        transform = m_currentTransform;
    }

    /// Return if the point is inside of the polygon prism volume or not.
    /// Use 'Crossings Test' to determine if point lies in or out of the polygon.
    /// @param point Position in world space to test against.
    bool PolygonPrismShape::IsPointInside(const AZ::Vector3& point)
    {
        m_intersectionDataCache.UpdateIntersectionParams(m_currentTransform, *m_polygonPrism, m_currentNonUniformScale);

        // initial early aabb rejection test
        // note: will implicitly do height test too
        if (!GetEncompassingAabb().Contains(point))
        {
            return false;
        }

        return PolygonPrismUtil::IsPointInside(*m_polygonPrism, point, m_currentTransform);
    }

    float PolygonPrismShape::DistanceSquaredFromPoint(const AZ::Vector3& point)
    {
        m_intersectionDataCache.UpdateIntersectionParams(m_currentTransform, *m_polygonPrism, m_currentNonUniformScale);

        return PolygonPrismUtil::DistanceSquaredFromPoint(*m_polygonPrism, point, m_currentTransform);;
    }

    bool PolygonPrismShape::IntersectRay(const AZ::Vector3& src, const AZ::Vector3& dir, float& distance)
    {
        m_intersectionDataCache.UpdateIntersectionParams(m_currentTransform, *m_polygonPrism, m_currentNonUniformScale);

        return PolygonPrismUtil::IntersectRay(m_intersectionDataCache.m_triangles, m_currentTransform, src, dir, distance);
    }

    void PolygonPrismShape::PolygonPrismIntersectionDataCache::UpdateIntersectionParamsImpl(
        const AZ::Transform& currentTransform, const AZ::PolygonPrism& polygonPrism,
        const AZ::Vector3& currentNonUniformScale)
    {
        m_aabb = PolygonPrismUtil::CalculateAabb(polygonPrism, currentTransform);
        GenerateSolidPolygonPrismMesh(
            polygonPrism.m_vertexContainer.GetVertices(),
            polygonPrism.GetHeight(), currentNonUniformScale, m_triangles);
    }

    void DrawPolygonPrismShape(
        const ShapeDrawParams& shapeDrawParams, const PolygonPrismMesh& polygonPrismMesh,
        AzFramework::DebugDisplayRequests& debugDisplay)
    {
        if (!gEnv->pRenderer)
        {
            return;
        }

        auto geomRenderer = gEnv->pRenderer->GetIRenderAuxGeom();

        const SAuxGeomRenderFlags oldFlags = geomRenderer->GetRenderFlags();
        SAuxGeomRenderFlags newFlags = oldFlags;

        // ensure render state is configured correctly - we want to read the depth
        // buffer but do not want to write to it (ensure objects inside the volume are not obscured)
        newFlags.SetAlphaBlendMode(e_AlphaBlended);
        newFlags.SetDepthWriteFlag(e_DepthWriteOff);
        newFlags.SetDepthTestFlag(e_DepthTestOn);

        geomRenderer->SetRenderFlags(newFlags);

        if (shapeDrawParams.m_filled)
        {
            if (!polygonPrismMesh.m_triangles.empty())
            {
                debugDisplay.DrawTriangles(polygonPrismMesh.m_triangles, shapeDrawParams.m_shapeColor);
            }
        }

        geomRenderer->SetRenderFlags(oldFlags);

        if (!polygonPrismMesh.m_lines.empty())
        {
            debugDisplay.DrawLines(polygonPrismMesh.m_lines, shapeDrawParams.m_wireColor);
        }
    }

    namespace PolygonPrismUtil
    {
        AZ::Aabb CalculateAabb(const AZ::PolygonPrism& polygonPrism, const AZ::Transform& worldFromLocal)
        {
            const AZ::VertexContainer<AZ::Vector2>& vertexContainer = polygonPrism.m_vertexContainer;

            const float height = polygonPrism.GetHeight();
            const AZ::Vector3& nonUniformScale = polygonPrism.GetNonUniformScale();

            AZ::Transform worldFromLocalUniformScale = worldFromLocal;
            const float entityScale = worldFromLocalUniformScale.ExtractScale().GetMaxElement();
            worldFromLocalUniformScale *= AZ::Transform::CreateScale(AZ::Vector3(entityScale));

            AZ::Aabb aabb = AZ::Aabb::CreateNull();
            // check base of prism
            for (const AZ::Vector2& vertex : vertexContainer.GetVertices())
            {
                aabb.AddPoint(worldFromLocalUniformScale.TransformPoint(nonUniformScale * AZ::Vector3(vertex.GetX(), vertex.GetY(), 0.0f)));
            }

            // check top of prism
            // set aabb to be height of prism - ensure entire polygon prism shape is enclosed in aabb
            for (const AZ::Vector2& vertex : vertexContainer.GetVertices())
            {
                aabb.AddPoint(worldFromLocalUniformScale.TransformPoint(nonUniformScale * AZ::Vector3(vertex.GetX(), vertex.GetY(), height)));
            }

            return aabb;
        }

        bool IsPointInside(const AZ::PolygonPrism& polygonPrism, const AZ::Vector3& point, const AZ::Transform& worldFromLocal)
        {
            using namespace PolygonPrismUtil;

            const float epsilon = 0.0001f;
            const float projectRayLength = 1000.0f;

            const AZStd::vector<AZ::Vector2>& vertices = polygonPrism.m_vertexContainer.GetVertices();
            const size_t vertexCount = vertices.size();

            AZ::Transform worldFromLocalWithUniformScale = worldFromLocal;
            const float transformScale = worldFromLocalWithUniformScale.ExtractScale().GetMaxElement();
            worldFromLocalWithUniformScale *= AZ::Transform::CreateScale(AZ::Vector3(transformScale));

            // transform point to local space
            // it's fine to invert the transform including scale here, because it won't affect whether the point is inside the prism
            const AZ::Vector3 localPoint =
                worldFromLocalWithUniformScale.GetInverse().TransformPoint(point) / polygonPrism.GetNonUniformScale();

            // ensure the point is not above or below the prism (in its local space)
            if (localPoint.GetZ() < 0.0f || localPoint.GetZ() > polygonPrism.GetHeight())
            {
                return false;
            }

            const AZ::Vector3 localPointFlattened = AZ::Vector3(localPoint.GetX(), localPoint.GetY(), 0.0f);
            const AZ::Vector3 localEndFlattened = localPointFlattened + AZ::Vector3::CreateAxisX() * projectRayLength;

            size_t intersections = 0;
            // use 'crossing test' algorithm to decide if the point lies within the volume or not
            // (odd number of intersections - inside, even number of intersections - outside)
            for (size_t i = 0; i < vertexCount; ++i)
            {
                const AZ::Vector3 segmentStart = AZ::Vector2ToVector3(vertices[i]);
                const AZ::Vector3 segmentEnd = AZ::Vector2ToVector3(vertices[(i + 1) % vertexCount]);

                AZ::Vector3 closestPosRay, closestPosSegment;
                float rayProportion, segmentProportion;
                AZ::Intersect::ClosestSegmentSegment(localPointFlattened, localEndFlattened, segmentStart, segmentEnd, rayProportion, segmentProportion, closestPosRay, closestPosSegment);
                const float delta = (closestPosRay - closestPosSegment).GetLengthSq();

                // have we crossed/touched a line on the polygon
                if (delta < epsilon)
                {
                    const AZ::Vector3 highestVertex = segmentStart.GetY() > segmentEnd.GetY() ? segmentStart : segmentEnd;

                    const float threshold = (highestVertex - point).Dot(AZ::Vector3::CreateAxisY());
                    if (AZ::IsClose(segmentProportion, 0.0f, AZ::Constants::FloatEpsilon))
                    {
                        // if at beginning of segment, only count intersection if segment is going up (y-axis)
                        // (prevent counting segments twice when intersecting at vertex)
                        if (threshold > 0.0f)
                        {
                            intersections++;
                        }
                    }
                    else
                    {
                        intersections++;
                    }
                }
            }

            // odd inside, even outside - bitwise AND to convert to bool
            return intersections & 1;
        }

        float DistanceSquaredFromPoint(const AZ::PolygonPrism& polygonPrism, const AZ::Vector3& point, const AZ::Transform& worldFromLocal)
        {
            const float height = polygonPrism.GetHeight();
            const AZ::Vector3& nonUniformScale = polygonPrism.GetNonUniformScale();

            // we want to invert the rotation and translation from the transform to get the point into the local space of the prism
            // but inverting any scale in the transform would mess up the distance, so extract that first and apply scale separately to the
            // prism
            AZ::Transform worldFromLocalNoScale = worldFromLocal;
            const float transformScale = worldFromLocalNoScale.ExtractScale().GetMaxElement();
            const AZ::Vector3 combinedScale = transformScale * nonUniformScale;
            const float scaledHeight = height * combinedScale.GetZ();

            // find the bottom and top which may be reversed from the usual order if the height or Z component of the scale is negative
            const float bottom = AZ::GetMin(scaledHeight, 0.0f);
            const float top = AZ::GetMax(scaledHeight, 0.0f);

            // translate and rotate (but don't scale) the point into the local space of the prism
            const AZ::Vector3 localPoint = worldFromLocalNoScale.GetInverse().TransformPoint(point);
            const AZ::Vector3 localPointFlattened = AZ::Vector3(localPoint.GetX(), localPoint.GetY(), 0.5f * (bottom + top));
            const AZ::Vector3 worldPointFlattened = worldFromLocalNoScale.TransformPoint(localPointFlattened);

            // first test if the point is contained within the polygon (flatten)
            if (IsPointInside(polygonPrism, worldPointFlattened, worldFromLocal))
            {
                if (localPoint.GetZ() < bottom)
                {
                    // if it's inside the 2d polygon but below the volume
                    const float distance = bottom - localPoint.GetZ();
                    return distance * distance;
                }

                if (localPoint.GetZ() > top)
                {
                    // if it's inside the 2d polygon but above the volume
                    const float distance = localPoint.GetZ() - top;
                    return distance * distance;
                }

                // if it's fully contained, return 0
                return 0.0f;
            }

            const AZStd::vector<AZ::Vector2>& vertices = polygonPrism.m_vertexContainer.GetVertices();
            const size_t vertexCount = vertices.size();

            // find closest segment
            AZ::Vector3 closestPos;
            float minDistanceSq = std::numeric_limits<float>::max();
            for (size_t i = 0; i < vertexCount; ++i)
            {
                const AZ::Vector3 segmentStart = combinedScale * AZ::Vector2ToVector3(vertices[i]);
                const AZ::Vector3 segmentEnd = combinedScale * AZ::Vector2ToVector3(vertices[(i + 1) % vertexCount]);

                AZ::Vector3 position;
                float proportion;
                AZ::Intersect::ClosestPointSegment(localPointFlattened, segmentStart, segmentEnd, proportion, position);

                const float distanceSq = (position - localPointFlattened).GetLengthSq();
                if (distanceSq < minDistanceSq)
                {
                    minDistanceSq = distanceSq;
                    closestPos = position;
                }
            }

            // constrain closest pos to [bottom, top] of volume
            closestPos += AZ::Vector3(0.0f, 0.0f, AZ::GetClamp<float>(localPoint.GetZ(), bottom, top));

            // return distanceSq from closest pos on prism
            return (closestPos - localPoint).GetLengthSq();
        }

        bool IntersectRay(
            AZStd::vector<AZ::Vector3> triangles, const AZ::Transform& worldFromLocal,
            const AZ::Vector3& src, const AZ::Vector3& dir, float& distance)
        {
            // must have at least one triangle
            if (triangles.size() < 3)
            {
                distance = std::numeric_limits<float>::max();
                return false;
            }

            // transform ray into local space
            AZ::Transform worldFromLocalNomalized = worldFromLocal;
            const float entityScale = worldFromLocalNomalized.ExtractScale().GetMaxElement();
            const AZ::Transform localFromWorldNormalized = worldFromLocalNomalized.GetInverse();
            const float rayLength = 1000.0f;
            const AZ::Vector3 localSrc = localFromWorldNormalized.TransformPoint(src);
            const AZ::Vector3 localDir = localFromWorldNormalized.TransformVector(dir);
            const AZ::Vector3 localEnd = localSrc + localDir * rayLength;

            // iterate over all triangles in polygon prism and
            // test ray against each in turn
            bool intersection = false;
            distance = std::numeric_limits<float>::max();
            for (size_t i = 0; i < triangles.size(); i += 3)
            {
                float t;
                AZ::Vector3 normal;
                if (AZ::Intersect::IntersectSegmentTriangle(
                    localSrc, localEnd,
                    triangles[i] * entityScale,
                    triangles[i + 1] * entityScale,
                    triangles[i + 2] * entityScale, normal, t))
                {
                    intersection |= true;

                    const float dist = t * rayLength;
                    if (dist < distance)
                    {
                        distance = dist;
                    }
                }
            }

            return intersection;
        }
    }
} // namespace LmbrCentral
