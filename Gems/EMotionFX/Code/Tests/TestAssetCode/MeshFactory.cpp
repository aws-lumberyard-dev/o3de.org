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

#include <Tests/TestAssetCode/MeshFactory.h>

#include <EMotionFX/Source/Mesh.h>
#include <EMotionFX/Source/SubMesh.h>
#include <EMotionFX/Source/SkinningInfoVertexAttributeLayer.h>
#include <EMotionFX/Source/VertexAttributeLayerAbstractData.h>

namespace EMotionFX
{
    EMotionFX::Mesh* MeshFactory::Create(
        const AZStd::vector<AZ::u32>& indices,
        const AZStd::vector<AZ::Vector3>& vertices,
        const AZStd::vector<AZ::Vector3>& normals,
        const AZStd::vector<AZ::Vector2>& uvs,
        const AZStd::vector<AZ::Color>& clothData,
        const AZStd::vector<VertexSkinInfluences>& skinningInfo
    ) {
        const AZ::u32 vertCount = aznumeric_cast<AZ::u32>(vertices.size());
        const AZ::u32 faceCount = aznumeric_cast<AZ::u32>(indices.size()) / 3;

        auto* mesh = EMotionFX::Mesh::Create(vertCount, aznumeric_caster(indices.size()), faceCount, vertCount, false);

        // Skinning info
        if (!skinningInfo.empty() && skinningInfo.size() == vertices.size())
        {
            auto* skinningLayer = EMotionFX::SkinningInfoVertexAttributeLayer::Create(vertCount);
            for (size_t vertex = 0; vertex < skinningInfo.size(); ++vertex)
            {
                for (const auto& [nodeNr, weight] : skinningInfo[vertex])
                {
                    skinningLayer->AddInfluence(static_cast<AZ::u32>(vertex), aznumeric_caster(nodeNr), weight, 0);
                }
            }

            mesh->AddSharedVertexAttributeLayer(skinningLayer);
        }

        // Original vertex numbers
        auto* orgVtxLayer = EMotionFX::VertexAttributeLayerAbstractData::Create(
            vertCount,
            EMotionFX::Mesh::ATTRIB_ORGVTXNUMBERS,
            sizeof(AZ::u32),
            true
        );
        mesh->AddVertexAttributeLayer(orgVtxLayer);
        AZStd::copy(indices.begin(), indices.end(), static_cast<AZ::u32*>(orgVtxLayer->GetOriginalData()));
        orgVtxLayer->ResetToOriginalData();

        // The positions layer
        auto* posLayer = EMotionFX::VertexAttributeLayerAbstractData::Create(
            vertCount,
            EMotionFX::Mesh::ATTRIB_POSITIONS,
            sizeof(AZ::Vector3),
            true
        );
        mesh->AddVertexAttributeLayer(posLayer);
        AZStd::copy(vertices.begin(), vertices.end(), static_cast<AZ::Vector3*>(posLayer->GetOriginalData()));
        posLayer->ResetToOriginalData();

        // The normals layer
        auto* normalsLayer = EMotionFX::VertexAttributeLayerAbstractData::Create(
            vertCount,
            EMotionFX::Mesh::ATTRIB_NORMALS,
            sizeof(AZ::Vector3),
            true
        );
        mesh->AddVertexAttributeLayer(normalsLayer);
        AZStd::copy(normals.begin(), normals.end(), static_cast<AZ::Vector3*>(normalsLayer->GetOriginalData()));
        normalsLayer->ResetToOriginalData();

        // The UVs layer.
        EMotionFX::VertexAttributeLayerAbstractData* uvsLayer = nullptr;
        if (!uvs.empty() && uvs.size() == vertices.size())
        {
            uvsLayer = EMotionFX::VertexAttributeLayerAbstractData::Create(vertCount, EMotionFX::Mesh::ATTRIB_UVCOORDS, sizeof(AZ::Vector2), true);
            mesh->AddVertexAttributeLayer(uvsLayer);
            AZStd::copy(uvs.begin(), uvs.end(), static_cast<AZ::Vector2*>(uvsLayer->GetOriginalData()));
            uvsLayer->ResetToOriginalData();
        }

        // The cloth layer.
        EMotionFX::VertexAttributeLayerAbstractData* clothLayer = nullptr;
        if (!clothData.empty() && clothData.size() == vertices.size())
        {
            clothLayer = EMotionFX::VertexAttributeLayerAbstractData::Create(vertCount, EMotionFX::Mesh::ATTRIB_CLOTH_DATA, sizeof(AZ::u32), false);
            mesh->AddVertexAttributeLayer(clothLayer);
            AZStd::transform(clothData.begin(), clothData.end(), static_cast<AZ::u32*>(clothLayer->GetOriginalData()), [](const AZ::Color& color) { return color.ToU32(); });
            clothLayer->ResetToOriginalData();
        }

        auto* subMesh = EMotionFX::SubMesh::Create(
            /*parentMesh=*/    mesh,
            /*startVertex=*/   0,
            /*startIndex=*/    0,
            /*startPolygon=*/  0,
            /*numVerts=*/      mesh->GetNumVertices(),
            /*numIndices=*/    mesh->GetNumIndices(),
            /*numPolygons=*/   mesh->GetNumPolygons(),
            /*materialIndex=*/ 0,
            /*numBones=*/      aznumeric_caster(skinningInfo.size())
        );
        mesh->AddSubMesh(subMesh);
        if (!skinningInfo.empty() && skinningInfo.size() == vertices.size())
        {
            for (size_t vertex = 0; vertex < skinningInfo.size(); ++vertex)
            {
                subMesh->SetBone(aznumeric_caster(vertex), aznumeric_caster(vertex));
            }
        }

        AZStd::fill(mesh->GetPolygonVertexCounts(), mesh->GetPolygonVertexCounts() + faceCount, 3);
        AZStd::copy(indices.begin(), indices.end(), mesh->GetIndices());

        return mesh;
    }
} // namespace EMotionFX
