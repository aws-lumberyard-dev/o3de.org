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

#include <ActorAsset.h>
#include <AtomActorInstance.h>

#include <EMotionFX/Source/TransformData.h>
#include <EMotionFX/Source/Actor.h>
#include <EMotionFX/Source/Mesh.h>
#include <EMotionFX/Source/MorphSetup.h>
#include <EMotionFX/Source/MorphTargetStandard.h>
#include <EMotionFX/Source/SubMesh.h>
#include <EMotionFX/Source/SkinningInfoVertexAttributeLayer.h>
#include <MCore/Source/DualQuaternion.h>

// For creating a skinned mesh from an actor
#include <Atom/Feature/SkinnedMesh/SkinnedMeshInputBuffers.h>
#include <Atom/RPI.Reflect/ResourcePoolAssetCreator.h>
#include <Atom/RPI.Reflect/Buffer/BufferAssetCreator.h>
#include <Atom/RPI.Reflect/Material/MaterialAsset.h>
#include <Atom/RPI.Reflect/Model/ModelAssetCreator.h>
#include <Atom/RPI.Reflect/Model/ModelLodAssetCreator.h>
#include <Atom/RPI.Public/Model/Model.h>

#include <AzCore/Asset/AssetManager.h>
#include <AzCore/base.h>
#include <AzCore/Math/Aabb.h>
#include <AzCore/Math/PackedVector3.h>
#include <AzCore/Math/Transform.h>
#include <AzCore/Math/Matrix3x4.h>
#include <AzCore/Math/MathUtils.h>
#include <AzCore/Component/Entity.h>

// Copied from ModelAssetBuilderComponent.cpp
namespace
{
    const AZ::u32 IndicesPerFace = 3;
    const AZ::RHI::Format IndicesFormat = AZ::RHI::Format::R32_UINT;

    const AZ::u32 PositionFloatsPerVert = 3;
    const AZ::u32 NormalFloatsPerVert = 3;
    const AZ::u32 UVFloatsPerVert = 2;
    const AZ::u32 ColorFloatsPerVert = 4;
    const AZ::u32 TangentFloatsPerVert = 4;
    const AZ::u32 BitangentFloatsPerVert = 3;

    const AZ::RHI::Format PositionFormat = AZ::RHI::Format::R32G32B32_FLOAT;
    const AZ::RHI::Format NormalFormat = AZ::RHI::Format::R32G32B32_FLOAT;
    const AZ::RHI::Format UVFormat = AZ::RHI::Format::R32G32_FLOAT;
    const AZ::RHI::Format ColorFormat = AZ::RHI::Format::R32G32B32A32_FLOAT;
    const AZ::RHI::Format TangentFormat = AZ::RHI::Format::R32G32B32A32_FLOAT;
    const AZ::RHI::Format BitangentFormat = AZ::RHI::Format::R32G32B32_FLOAT;
    const AZ::RHI::Format BoneIndexFormat = AZ::RHI::Format::R32G32B32A32_UINT;
    const AZ::RHI::Format BoneWeightFormat = AZ::RHI::Format::R32G32B32A32_FLOAT;

    const size_t LinearSkinningFloatsPerBone = 12;
    const size_t DualQuaternionSkinningFloatsPerBone = 8;
    const uint32_t MaxSupportedSkinInfluences = 4;
}

namespace AZ
{
    namespace Render
    {
        static bool IsVertexCountWithinSupportedRange(size_t vertexOffset, size_t vertexCount)
        {
            return vertexOffset + vertexCount <= aznumeric_cast<size_t>(SkinnedMeshVertexStreamPropertyInterface::Get()->GetMaxSupportedVertexCount());
        }

        static void CalculateSubmeshPropertiesForLod(const Data::AssetId& actorAssetId, const EMotionFX::Actor* actor, size_t lodIndex, AZStd::vector<SkinnedSubMeshProperties>& subMeshes, uint32_t& lodIndexCount, uint32_t& lodVertexCount)
        {
            lodIndexCount = 0;
            lodVertexCount = 0;

            uint32_t subMeshIndexOffset = 0;
            const Data::Asset<RPI::ModelLodAsset>& lodAsset = actor->GetMeshAsset()->GetLodAssets()[lodIndex];
            const AZStd::array_view<RPI::ModelLodAsset::Mesh> modelMeshes = lodAsset->GetMeshes();
            for (const RPI::ModelLodAsset::Mesh& modelMesh : modelMeshes)
            {
                const size_t subMeshIndexCount = modelMesh.GetIndexCount();
                const size_t subMeshVertexCount = modelMesh.GetVertexCount();
                if (subMeshVertexCount > 0)
                {
                    if (IsVertexCountWithinSupportedRange(lodVertexCount, subMeshVertexCount))
                    {
                        SkinnedSubMeshProperties skinnedSubMesh{};
                        skinnedSubMesh.m_indexOffset = lodIndexCount;
                        skinnedSubMesh.m_indexCount = aznumeric_cast<uint32_t>(subMeshIndexCount);
                        lodIndexCount += aznumeric_cast<uint32_t>(subMeshIndexCount);

                        skinnedSubMesh.m_vertexOffset = lodVertexCount;
                        skinnedSubMesh.m_vertexCount = aznumeric_cast<uint32_t>(subMeshVertexCount);
                        lodVertexCount += aznumeric_cast<uint32_t>(subMeshVertexCount);

                        // The default material id used by a sub-mesh is the guid of the source .fbx plus the subId which is a unique material ID from the scene API
                        AZ::u32 subId = modelMesh.GetMaterialAsset().GetId().m_subId;
                        AZ::Data::AssetId materialId{ actorAssetId.m_guid, subId };

                        // Queue the material asset - the ModelLod seems to handle delayed material loads
                        skinnedSubMesh.m_material = Data::AssetManager::Instance().GetAsset(materialId, azrtti_typeid<RPI::MaterialAsset>(), skinnedSubMesh.m_material.GetAutoLoadBehavior());
                        subMeshes.push_back(skinnedSubMesh);
                    }
                    else
                    {
                        AZStd::string assetPath;
                        Data::AssetCatalogRequestBus::BroadcastResult(assetPath, &Data::AssetCatalogRequests::GetAssetPathById, actorAssetId);
                        AZ_Error("ActorAsset", false, "Lod '%d' for actor '%s' has greater than %d, the maximum supported number of vertices for a skinned sub-mesh. Sub-mesh will be ignored and not all vertices will be rendered.", lodIndex, assetPath.c_str(), SkinnedMeshVertexStreamPropertyInterface::Get()->GetMaxSupportedVertexCount());
                    }
                }
            }
        }

        static void ProcessIndicesForSubmesh(size_t indexCount, size_t atomIndexBufferOffset, size_t emfxSourceVertexStart, const uint32_t* emfxSubMeshIndices, AZStd::vector<uint32_t>& indexBufferData)
        {
            for (size_t index = 0; index < indexCount; ++index)
            {
                // The emfxSubMeshIndices is a pointer to the start of the indices for a particular sub-mesh, so we need to copy the indices from 0-indexCount instead of offsetting the start by emfxSourceVertexStart like we do with the other buffers
                // Also, the emfxSubMeshIndices are relative to the start vertex of the sub-mesh, so we need to subtract emfxSourceVertexStart to get the actual index of the vertex within the lod's vertex buffer
                indexBufferData[atomIndexBufferOffset + index] = emfxSubMeshIndices[index] - aznumeric_cast<uint32_t>(emfxSourceVertexStart);
            }
        }

        static void ProcessPositionsForSubmesh(size_t vertexCount, size_t atomVertexBufferOffset, size_t emfxSourceVertexStart, const AZ::Vector3* emfxSourcePositions, AZStd::vector<PackedVector3f>& positionBufferData, SkinnedSubMeshProperties& submesh)
        {
            // Pack the source Vector3 positions (which have 4 components under the hood) into a PackedVector3f buffer for Atom, and build an Aabb along the way
            // ATOM-3898 Investigate buffer format and alignment performance to compare current packed R32G32B32 buffer with R32G32B32A32 buffer
            Aabb localAabb = Aabb::CreateNull();
            for (size_t vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex)
            {
                const Vector3& sourcePosition = emfxSourcePositions[emfxSourceVertexStart + vertexIndex];
                localAabb.AddPoint(sourcePosition);
                positionBufferData[atomVertexBufferOffset + vertexIndex] = PackedVector3f(sourcePosition);
            }

            submesh.m_aabb = localAabb;
        }

        static void ProcessNormalsForSubmesh(size_t vertexCount, size_t atomVertexBufferOffset, size_t emfxSourceVertexStart, const AZ::Vector3* emfxSourceNormals, AZStd::vector<PackedVector3f>& normalBufferData)
        {
            // Pack the source Vector3 normals (which have 4 components under the hood) into a PackedVector3f buffer for Atom
            // ATOM-3898 Investigate buffer format and alignment performance to compare current packed R32G32B32 buffer with R32G32B32A32 buffer
            for (size_t vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex)
            {
                const Vector3& sourceNormal = emfxSourceNormals[emfxSourceVertexStart + vertexIndex];
                normalBufferData[atomVertexBufferOffset + vertexIndex] = PackedVector3f(sourceNormal);
            }
        }

        static void ProcessUVsForSubmesh(size_t vertexCount, size_t atomVertexBufferOffset, [[maybe_unused]] size_t emfxSourceVertexStart, const AZ::Vector2* emfxSourceUVs, AZStd::vector<float[2]>& uvBufferData)
        {
            for (size_t vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex)
            {
                emfxSourceUVs[vertexIndex].StoreToFloat2(uvBufferData[atomVertexBufferOffset + vertexIndex]);
            }
        }

        static void ProcessTangentsForSubmesh(size_t vertexCount, size_t atomVertexBufferOffset, size_t emfxSourceVertexStart, const AZ::Vector4* emfxSourceTangents, AZStd::vector<Vector4>& tangentBufferData)
        {
            AZStd::copy(&emfxSourceTangents[emfxSourceVertexStart], &emfxSourceTangents[emfxSourceVertexStart + vertexCount], tangentBufferData.data() + atomVertexBufferOffset);
        }

        static void ProcessBitangentsForSubmesh(size_t vertexCount, size_t atomVertexBufferOffset, size_t emfxSourceVertexStart, const AZ::Vector3* emfxSourceBitangents, AZStd::vector<PackedVector3f>& bitangentBufferData)
        {
            AZ_Assert(emfxSourceBitangents, "GenerateBitangentsForSubmesh called with null source normals.");

            // Pack the source Vector3 bitangents (which have 4 components under the hood) into a PackedVector3f buffer for Atom
            // ATOM-3898 Investigate buffer format and alignment performance to compare current packed R32G32B32 buffer with R32G32B32A32 buffer
            for (size_t i = 0; i < vertexCount; ++i)
            {
                const Vector3& sourceBitangent = emfxSourceBitangents[emfxSourceVertexStart + i];
                bitangentBufferData[atomVertexBufferOffset + i] = PackedVector3f(sourceBitangent);
            }
        }

        static void GenerateBitangentsForSubmesh(size_t vertexCount, size_t atomVertexBufferOffset, size_t emfxSourceVertexStart, const AZ::Vector3* emfxSourceNormals, const AZ::Vector4* emfxSourceTangents, AZStd::vector<PackedVector3f>& bitangentBufferData)
        {
            AZ_Assert(emfxSourceNormals, "GenerateBitangentsForSubmesh called with null source normals.");
            AZ_Assert(emfxSourceTangents, "GenerateBitangentsForSubmesh called with null source tangents.");

            // Compute bitangent from tangent and normal.
            for (size_t i = 0; i < vertexCount; ++i)
            {
                const Vector4& sourceTangent = emfxSourceTangents[emfxSourceVertexStart + i];
                const Vector3& sourceNormal = emfxSourceNormals[emfxSourceVertexStart + i];
                const Vector3 bitangent = sourceNormal.Cross(sourceTangent.GetAsVector3()) * sourceTangent.GetW();
                bitangentBufferData[atomVertexBufferOffset + i] = PackedVector3f(bitangent);
            }
        }

        static void ProcessSkinInfluences(
            const EMotionFX::Mesh* mesh,
            const EMotionFX::SubMesh* subMesh,
            size_t atomVertexBufferOffset,
            AZStd::vector<uint32_t[MaxSupportedSkinInfluences / 2]>& blendIndexBufferData,
            AZStd::vector<AZStd::array<float, MaxSupportedSkinInfluences>>& blendWeightBufferData,
            bool hasClothData)
        {
            EMotionFX::SkinningInfoVertexAttributeLayer* sourceSkinningInfo = static_cast<EMotionFX::SkinningInfoVertexAttributeLayer*>(mesh->FindSharedVertexAttributeLayer(EMotionFX::SkinningInfoVertexAttributeLayer::TYPE_ID));
            
            // EMotionFX source gives 16 bit indices and 32 bit float weights
            // Atom consumes 32 bit uint indices and 32 bit float weights (range 0-1)
            // Up to MaxSupportedSkinInfluences influences per vertex are supported
            const uint32_t* sourceOriginalVertex = static_cast<uint32_t*>(mesh->FindOriginalVertexData(EMotionFX::Mesh::ATTRIB_ORGVTXNUMBERS));
            const uint32_t vertexCount = subMesh->GetNumVertices();
            const uint32_t vertexStart = subMesh->GetStartVertex();
            for (uint32_t vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex)
            {
                const uint32_t originalVertex = sourceOriginalVertex[vertexIndex + vertexStart];
                const uint32_t influenceCount = AZStd::GetMin<uint32_t>(MaxSupportedSkinInfluences, sourceSkinningInfo->GetNumInfluences(originalVertex));
                uint32_t influenceIndex = 0;
                float weightError = 1.0f;

                AZStd::vector<uint32_t> localIndices;
                for (; influenceIndex < influenceCount; ++influenceIndex)
                {
                    EMotionFX::SkinInfluence* influence = sourceSkinningInfo->GetInfluence(originalVertex, influenceIndex);
                    localIndices.push_back(static_cast<uint32_t>(influence->GetNodeNr()));
                    blendWeightBufferData[atomVertexBufferOffset + vertexIndex][influenceIndex] = influence->GetWeight();
                    weightError -= blendWeightBufferData[atomVertexBufferOffset + vertexIndex][influenceIndex];
                }

                // Zero out any unused ids/weights
                for (; influenceIndex < MaxSupportedSkinInfluences; ++influenceIndex)
                {
                    localIndices.push_back(0);
                    blendWeightBufferData[atomVertexBufferOffset + vertexIndex][influenceIndex] = 0.0f;
                }

                // Now that we have the 16-bit indices, pack them into 32-bit uints
                for (size_t i = 0; i < localIndices.size(); ++i)
                {
                    if (i % 2 == 0)
                    {
                        // Put the first/even ids in the most significant bits
                        blendIndexBufferData[atomVertexBufferOffset + vertexIndex][i/2] = localIndices[i] << 16;
                    }
                    else
                    {
                        // Put the next/odd ids in the least significant bits
                        blendIndexBufferData[atomVertexBufferOffset + vertexIndex][i / 2] |= localIndices[i];
                    }
                }
            }

            // If there is cloth data, set all the blend weights to zero to indicate
            // the vertices will be updated by cpu.
            //
            // [TODO ATOM-14478]
            // At the moment blend weights is a shared buffer and therefore all
            // instances of the actor asset will be affected by it. In the future
            // this buffer will be unique per instance and modified by cloth component
            // when necessary.
            //
            // [TODO LYN-1890]
            // At the moment, if there is cloth data it is assumed that every vertex in the
            // submesh will be simulated by cloth in cpu, so all the weights are set to zero.
            // But once the blend weights buffer can be modified per instance, it will be set by
            // the cloth component, which decides whether to control the whole submesh or
            // to apply an additional simplification pass to remove static triangles from simulation.
            // Static triangles are the ones that all its vertices won't move during simulation and
            // therefore its weights won't be altered so they are controlled by GPU.
            // This additional simplification has been disabled in ClothComponentMesh.cpp for now.
            if (hasClothData)
            {
                for (uint32_t vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex)
                {
                    for (uint32_t influenceIndex = 0; influenceIndex < MaxSupportedSkinInfluences; ++influenceIndex)
                    {
                        blendWeightBufferData[atomVertexBufferOffset + vertexIndex][influenceIndex] = 0.0f;
                    }
                }
            }
        }
        
        void ProcessMorphsForLod(const EMotionFX::Actor* actor, uint32_t lodIndex, const AZStd::string& fullFileName, SkinnedMeshInputLod& skinnedMeshLod)
        {
            EMotionFX::MorphSetup* morphSetup = actor->GetMorphSetup(lodIndex);
            if (morphSetup)
            {
                uint32_t morphTargetCount = morphSetup->GetNumMorphTargets();

                // We're going to split the data into separate streams with 4byte elements,
                // which allows for a coalesced read in the morph target compute shader when each thread is loading 4 adjacent bytes at the same time

                // Packed, interleaved vertex deltas
                AZStd::vector<uint32_t> deltaData;

                uint32_t totalDeformDataCount = 0;

                for (uint32_t morphTargetIndex = 0; morphTargetIndex < morphTargetCount; ++morphTargetIndex)
                {
                    EMotionFX::MorphTarget* morphTarget = morphSetup->GetMorphTarget(morphTargetIndex);
                    // check if we are dealing with a standard morph target
                    if (morphTarget->GetType() != EMotionFX::MorphTargetStandard::TYPE_ID)
                    {
                        continue;
                    }

                    // down cast the morph target
                    EMotionFX::MorphTargetStandard* morphTargetStandard = static_cast<EMotionFX::MorphTargetStandard*>(morphTarget);
                    uint32_t deformDataCount = morphTargetStandard->GetNumDeformDatas();

                    // Get the min/max weight across the entire morph
                    float minWeight = morphTargetStandard->GetRangeMin();
                    float maxWeight = morphTargetStandard->GetRangeMax();

                    // There are multiple deforms for a single morph. Combine them all into a single morph to be processed at once
                    for (uint32_t deformDataIndex = 0; deformDataIndex < deformDataCount; ++deformDataIndex)
                    {
                        EMotionFX::MorphTargetStandard::DeformData* deformData = morphTargetStandard->GetDeformData(deformDataIndex);

                        // Vertex data
                        for (uint32_t vertexIndex = 0; vertexIndex < deformData->mNumVerts; ++vertexIndex)
                        {
                            const EMotionFX::MorphTargetStandard::DeformData::VertexDelta& delta = deformData->mDeltas[vertexIndex];

                            // TODO: Modify the builder to pack data the way it's expected to be read on the GPU instead of packing it here [ATOM-14698]

                            // Add the target vertex index
                            deltaData.push_back(delta.mVertexNr);

                            // Combine the position x in the most-significant 16 bits and position y in the least significant 16 bits
                            uint32_t xy = 0;
                            xy |= (static_cast<uint32_t>(delta.mPosition.mX) << 16);
                            xy |= static_cast<uint32_t>(delta.mPosition.mY);
                            deltaData.push_back(xy);

                            // Combine the position z in the most significant 16 bits with the normal x and y in the least significant 16 bits
                            uint32_t positionZnormalXY = 0;
                            positionZnormalXY |= static_cast<uint32_t>(delta.mPosition.mZ) << 16;
                            positionZnormalXY |= static_cast<uint32_t>(delta.mNormal.mX) << 8;
                            positionZnormalXY |= static_cast<uint32_t>(delta.mNormal.mY);
                            deltaData.push_back(positionZnormalXY);

                            // Combine the normal z in the most significant 8 bits with the tangent in the least significant 24 bits
                            uint32_t normalZTangent = 0;
                            normalZTangent |= (static_cast<uint32_t>(delta.mNormal.mZ) << 24);
                            normalZTangent |= (static_cast<uint32_t>(delta.mTangent.mX) << 16);
                            normalZTangent |= (static_cast<uint32_t>(delta.mTangent.mY) << 8);
                            normalZTangent |= static_cast<uint32_t>(delta.mTangent.mZ);
                            deltaData.push_back(normalZTangent);

                            // Combine padding in most significant 8 bits with the bitangent in the least significant 24 bits
                            uint32_t padBitangent = 0;
                            padBitangent |= static_cast<uint32_t>(delta.mBitangent.mX) << 16;
                            padBitangent |= (static_cast<uint32_t>(delta.mBitangent.mY) << 8);
                            padBitangent |= static_cast<uint32_t>(delta.mBitangent.mZ);
                            deltaData.push_back(padBitangent);

                            // Extra padding so each structured element of the buffer is 16 byte aligned
                            deltaData.push_back(0);
                            deltaData.push_back(0);
                            deltaData.push_back(0);
                        }

                        if (deformData->mNumVerts > 0)
                        {
                            // The skinned mesh lod gets a unique morph for each deform data, since each one has unique min/max delta values to use for decompression
                            AZStd::string morphString = AZStd::string::format("_Lod%u_Morph%u", lodIndex, totalDeformDataCount);
                            skinnedMeshLod.AddMorphTarget(minWeight, maxWeight, deformData->mMinValue, deformData->mMaxValue, deformData->mNumVerts, deltaData, fullFileName + morphString);
                            totalDeformDataCount++;
                        }
                        else
                        {
                            AZ_Warning("ProcessMorphsForLod", false, "EMotionFX deform data '%u' in morph target '%u' for lod '%u' in '%s' modifies zero vertices and will be skipped.", deformDataIndex, morphTargetIndex, lodIndex, fullFileName.c_str());
                        }
                        deltaData.clear();
                    }
                }
            }
        }

        AZStd::intrusive_ptr<SkinnedMeshInputBuffers> CreateSkinnedMeshInputFromActor(const Data::AssetId& actorAssetId, const EMotionFX::Actor* actor)
        {

            Data::Asset<RPI::ModelAsset> modelAsset = actor->GetMeshAsset();
            if (!modelAsset.IsReady())
            {
                AZ_Error("CreateSkinnedMeshInputFromActor", false, "Attempting to create skinned mesh input buffers for an actor that doesn't have a loaded model.");
                return nullptr;
            }

            AZStd::intrusive_ptr<SkinnedMeshInputBuffers> skinnedMeshInputBuffers = aznew SkinnedMeshInputBuffers;

            skinnedMeshInputBuffers->SetAssetId(actorAssetId);

            // Get the fileName, which will be used to label the buffers
            AZStd::string assetPath;
            Data::AssetCatalogRequestBus::BroadcastResult(assetPath, &Data::AssetCatalogRequests::GetAssetPathById, actorAssetId);
            AZStd::string fullFileName;
            AzFramework::StringFunc::Path::GetFullFileName(assetPath.c_str(), fullFileName);

            // GetNumNodes returns the number of 'joints' or 'bones' in the skeleton
            const size_t numJoints = actor->GetNumNodes();
            const size_t numLODs = actor->GetNumLODLevels();

            // Create the containers to hold the data for all the combined sub-meshes
            AZStd::vector<uint32_t> indexBufferData;
            AZStd::vector<PackedVector3f> positionBufferData;
            AZStd::vector<PackedVector3f> normalBufferData;
            AZStd::vector<Vector4> tangentBufferData;
            AZStd::vector<PackedVector3f> bitangentBufferData;
            AZStd::vector<uint32_t[MaxSupportedSkinInfluences / 2]> blendIndexBufferData;
            AZStd::vector<AZStd::array<float, MaxSupportedSkinInfluences>> blendWeightBufferData;
            AZStd::vector<float[2]> uvBufferData;

            //
            // Process all LODs from the EMotionFX actor data.
            //

            skinnedMeshInputBuffers->SetLodCount(numLODs);
            AZ_Assert(numLODs == modelAsset->GetLodCount(), "The lod count of the EMotionFX mesh and Atom model are out of sync for '%s'", fullFileName.c_str());
            for (size_t lodIndex = 0; lodIndex < numLODs; ++lodIndex)
            {
                // Create a single LOD
                SkinnedMeshInputLod skinnedMeshLod;

                Data::Asset<RPI::ModelLodAsset> modelLodAsset = modelAsset->GetLodAssets()[lodIndex];

                // Each mesh vertex stream is packed into a single buffer for the whole lod. Get the first mesh, which can be used to retrieve the underlying buffer assets
                AZ_Assert(modelLodAsset->GetMeshes().size() > 0, "ModelLod '%d' for model '%s' has 0 meshes", lodIndex, fullFileName.c_str());
                const RPI::ModelLodAsset::Mesh& mesh0 = modelLodAsset->GetMeshes()[0];

                // Get the amount of vertices and indices
                // Get the meshes to process
                bool hasUVs = false;
                bool hasUVs2 = false;
                bool hasTangents = false;
                bool hasBitangents = false;
                bool hasClothData = false;

                // Do a pass over the lod to find the number of sub-meshes, the offset and size of each sub-mesh, and total number of vertices in the lod.
                // These will be combined into one input buffer for the source actor, but these offsets and sizes will be used to create multiple sub-meshes for the target skinned actor
                uint32_t lodVertexCount = 0;
                uint32_t lodIndexCount = 0;
                AZStd::vector<SkinnedSubMeshProperties> subMeshes;
                CalculateSubmeshPropertiesForLod(actorAssetId, actor, lodIndex, subMeshes, lodIndexCount, lodVertexCount);

                skinnedMeshLod.SetIndexCount(lodIndexCount);
                skinnedMeshLod.SetVertexCount(lodVertexCount);

                // We'll be overwriting all the elements, so no need to construct them when resizing
                indexBufferData.resize_no_construct(lodIndexCount);
                positionBufferData.resize_no_construct(lodVertexCount);
                normalBufferData.resize_no_construct(lodVertexCount);
                tangentBufferData.resize_no_construct(lodVertexCount);
                bitangentBufferData.resize_no_construct(lodVertexCount);
                blendIndexBufferData.resize_no_construct(lodVertexCount);
                blendWeightBufferData.resize_no_construct(lodVertexCount);
                uvBufferData.resize_no_construct(lodVertexCount);

                // Now iterate over the actual data and populate the data for the per-actor buffers
                size_t lodVertexStart = 0;
                size_t indexBufferOffset = 0;
                size_t vertexBufferOffset = 0;
                size_t skinnedMeshSubmeshIndex = 0;
                for (size_t jointIndex = 0; jointIndex < numJoints; ++jointIndex)
                {
                    const EMotionFX::Mesh* mesh = actor->GetMesh(lodIndex, jointIndex);
                    if (!mesh || mesh->GetIsCollisionMesh())
                    {
                        continue;
                    }

                    // Each of these is one long buffer containing the data for all sub-meshes in the joint
                    const AZ::Vector3* sourcePositions = static_cast<const AZ::Vector3*>(mesh->FindOriginalVertexData(EMotionFX::Mesh::ATTRIB_POSITIONS));
                    const AZ::Vector3* sourceNormals = static_cast<const AZ::Vector3*>(mesh->FindOriginalVertexData(EMotionFX::Mesh::ATTRIB_NORMALS));
                    const uint32_t* sourceOriginalVertex = static_cast<const uint32_t*>(mesh->FindOriginalVertexData(EMotionFX::Mesh::ATTRIB_ORGVTXNUMBERS));
                    const AZ::Vector4* sourceTangents = static_cast<const AZ::Vector4*>(mesh->FindOriginalVertexData(EMotionFX::Mesh::ATTRIB_TANGENTS));
                    const AZ::Vector3* sourceBitangents = static_cast<const AZ::Vector3*>(mesh->FindOriginalVertexData(EMotionFX::Mesh::ATTRIB_BITANGENTS));
                    const AZ::Vector2* sourceUVs = static_cast<const AZ::Vector2*>(mesh->FindOriginalVertexData(EMotionFX::Mesh::ATTRIB_UVCOORDS, 0));
                    const AZ::Vector2* sourceUVs2 = static_cast<const AZ::Vector2*>(mesh->FindOriginalVertexData(EMotionFX::Mesh::ATTRIB_UVCOORDS, 1));
                    const uint32_t* sourceClothData = static_cast<uint32_t*>(mesh->FindOriginalVertexData(EMotionFX::Mesh::ATTRIB_CLOTH_DATA));

                    hasUVs = (sourceUVs != nullptr);
                    hasUVs2 = (sourceUVs2 != nullptr);
                    hasTangents = (sourceTangents != nullptr);
                    hasBitangents = (sourceBitangents != nullptr);
                    hasClothData = (sourceClothData != nullptr);

                    // For each sub-mesh within each mesh, we want to create a separate sub-piece.
                    const size_t numSubMeshes = mesh->GetNumSubMeshes();

                    for (size_t subMeshIndex = 0; subMeshIndex < numSubMeshes; ++subMeshIndex)
                    {
                        const EMotionFX::SubMesh* subMesh = mesh->GetSubMesh(subMeshIndex);
                        const size_t vertexCount = subMesh->GetNumVertices();

                        // Skip empty sub-meshes and sub-meshes that would put the total vertex count beyond the supported range
                        if (vertexCount > 0 && IsVertexCountWithinSupportedRange(vertexBufferOffset, vertexCount))
                        {
                            const size_t indexCount = subMesh->GetNumIndices();
                            const uint32_t* indices = subMesh->GetIndices();
                            const size_t vertexStart = subMesh->GetStartVertex();

                            ProcessIndicesForSubmesh(indexCount, indexBufferOffset, vertexStart, indices, indexBufferData);
                            ProcessPositionsForSubmesh(vertexCount, vertexBufferOffset, vertexStart, sourcePositions, positionBufferData, subMeshes[skinnedMeshSubmeshIndex]);
                            ProcessNormalsForSubmesh(vertexCount, vertexBufferOffset, vertexStart, sourceNormals, normalBufferData);

                            AZ_Assert(hasUVs, "ActorAsset '%s' lod '%d' missing uvs. Downstream code is assuming all actors have uvs", fullFileName.c_str(), lodIndex);
                            if (hasUVs)
                            {
                                ProcessUVsForSubmesh(vertexCount, vertexBufferOffset, vertexStart, sourceUVs, uvBufferData);
                            }
                            // ATOM-3623 Support multiple UV sets in actors

                            // ATOM-3972 Support actors that don't have tangents
                            AZ_Assert(hasTangents, "ActorAsset '%s' lod '%d'  missing tangents. Downstream code is assuming all actors have tangents", fullFileName.c_str(), lodIndex);
                            if (hasTangents)
                            {
                                ProcessTangentsForSubmesh(vertexCount, vertexBufferOffset, vertexStart, sourceTangents, tangentBufferData);
                                if (hasBitangents)
                                {
                                    ProcessBitangentsForSubmesh(vertexCount, vertexBufferOffset, vertexStart, sourceBitangents, bitangentBufferData);
                                }
                                else
                                {
                                    GenerateBitangentsForSubmesh(vertexCount, vertexBufferOffset, vertexStart, sourceNormals, sourceTangents, bitangentBufferData);
                                }
                            }

                            ProcessSkinInfluences(mesh, subMesh, vertexBufferOffset, blendIndexBufferData, blendWeightBufferData, hasClothData);

                            // Increment offsets so that the next sub-mesh can start at the right place
                            indexBufferOffset += indexCount;
                            vertexBufferOffset += vertexCount;
                            skinnedMeshSubmeshIndex++;
                        }
                    }   // for all submeshes
                } // for all meshes

                // Now that the data has been prepped, create the actual buffers

                // Create read-only buffers and views for input buffers that are shared across all instances
                AZStd::string lodString = AZStd::string::format("_Lod%zu", lodIndex);
                skinnedMeshLod.SetSkinningInputBufferAsset(mesh0.GetSemanticBufferAssetView(Name{ "POSITION" })->GetBufferAsset(), SkinnedMeshInputVertexStreams::Position);
                skinnedMeshLod.SetSkinningInputBufferAsset(mesh0.GetSemanticBufferAssetView(Name{ "NORMAL" })->GetBufferAsset(), SkinnedMeshInputVertexStreams::Normal);
                skinnedMeshLod.SetSkinningInputBufferAsset(mesh0.GetSemanticBufferAssetView(Name{ "TANGENT" })->GetBufferAsset(), SkinnedMeshInputVertexStreams::Tangent);
                skinnedMeshLod.SetSkinningInputBufferAsset(mesh0.GetSemanticBufferAssetView(Name{ "BITANGENT" })->GetBufferAsset(), SkinnedMeshInputVertexStreams::BiTangent);
                
                Data::Asset<RPI::BufferAsset> jointIndicesBufferAsset = mesh0.GetSemanticBufferAssetView(Name{ "SKIN_JOINTINDICES" })->GetBufferAsset();
                skinnedMeshLod.SetSkinningInputBufferAsset(jointIndicesBufferAsset, SkinnedMeshInputVertexStreams::BlendIndices);
                Data::Asset<RPI::BufferAsset> skinWeightsBufferAsset = mesh0.GetSemanticBufferAssetView(Name{ "SKIN_WEIGHTS" })->GetBufferAsset();
                skinnedMeshLod.SetSkinningInputBufferAsset(skinWeightsBufferAsset, SkinnedMeshInputVertexStreams::BlendWeights);

                // We're using the indices/weights buffers directly from the model.
                // However, EMFX has done some re-mapping of the id's, so we need to update the GPU buffer for it to have the correct data.
                size_t remappedJointIndexBufferSizeInBytes = blendIndexBufferData.size() * sizeof(blendIndexBufferData[0]);
                size_t remappedSkinWeightsBufferSizeInBytes = blendWeightBufferData.size() * sizeof(blendWeightBufferData[0]);

                AZ_Assert(jointIndicesBufferAsset->GetBufferDescriptor().m_byteCount == remappedJointIndexBufferSizeInBytes, "Joint indices data from EMotionFX is not the same size as the buffer from the model in '%s', lod '%d'", fullFileName.c_str(), lodIndex);
                AZ_Assert(skinWeightsBufferAsset->GetBufferDescriptor().m_byteCount == remappedSkinWeightsBufferSizeInBytes, "Skin weights data from EMotionFX is not the same size as the buffer from the model in '%s', lod '%d'", fullFileName.c_str(), lodIndex);

                Data::Instance<RPI::Buffer> jointIndicesBuffer = RPI::Buffer::FindOrCreate(jointIndicesBufferAsset);
                jointIndicesBuffer->UpdateData(blendIndexBufferData.data(), remappedJointIndexBufferSizeInBytes);
                Data::Instance<RPI::Buffer> skinWeightsBuffer = RPI::Buffer::FindOrCreate(skinWeightsBufferAsset);
                skinWeightsBuffer->UpdateData(blendWeightBufferData.data(), remappedSkinWeightsBufferSizeInBytes);
                

                // Create read-only input assembly buffers that are not modified during skinning and shared across all instances
                skinnedMeshLod.SetIndexBuffer(mesh0.GetIndexBufferAssetView().GetBufferAsset());
                skinnedMeshLod.CreateStaticBuffer(uvBufferData.data(), SkinnedMeshStaticVertexStreams::UV_0, fullFileName + lodString + "_SkinnedMeshStaticUVs");

                // Set the data that needs to be tracked on a per-sub-mesh basis
                // and create the common, shared sub-mesh buffer views
                skinnedMeshLod.SetSubMeshProperties(subMeshes);

                ProcessMorphsForLod(actor, lodIndex, fullFileName, skinnedMeshLod);

                skinnedMeshInputBuffers->SetLod(lodIndex, skinnedMeshLod);

            } // for all lods

            return skinnedMeshInputBuffers;
        }

        void GetBoneTransformsFromActorInstance(const EMotionFX::ActorInstance* actorInstance, AZStd::vector<float>& boneTransforms, EMotionFX::Integration::SkinningMethod skinningMethod)
        {
            const EMotionFX::TransformData* transforms = actorInstance->GetTransformData();
            const AZ::Matrix3x4* skinningMatrices = transforms->GetSkinningMatrices();

            // For linear skinning, we need a 3x4 row-major float matrix for each transform
            const size_t numBoneTransforms = transforms->GetNumTransforms();

            if (skinningMethod == EMotionFX::Integration::SkinningMethod::Linear)
            {
                boneTransforms.resize_no_construct(numBoneTransforms * LinearSkinningFloatsPerBone);
                for (size_t i = 0; i < numBoneTransforms; ++i)
                {
                    skinningMatrices[i].StoreToRowMajorFloat12(&boneTransforms[i * LinearSkinningFloatsPerBone]);
                }
            }
            else if(skinningMethod == EMotionFX::Integration::SkinningMethod::DualQuat)
            {
                boneTransforms.resize_no_construct(numBoneTransforms * DualQuaternionSkinningFloatsPerBone);
                for (size_t i = 0; i < numBoneTransforms; ++i)
                {
                    MCore::DualQuaternion dualQuat = MCore::DualQuaternion::ConvertFromTransform(AZ::Transform::CreateFromMatrix3x4(skinningMatrices[i]));
                    dualQuat.mReal.StoreToFloat4(&boneTransforms[i * DualQuaternionSkinningFloatsPerBone]);
                    dualQuat.mDual.StoreToFloat4(&boneTransforms[i * DualQuaternionSkinningFloatsPerBone + 4]);
                }
            }
        }

        Data::Instance<RPI::Buffer> CreateBoneTransformBufferFromActorInstance(const EMotionFX::ActorInstance* actorInstance, EMotionFX::Integration::SkinningMethod skinningMethod)
        {
            // Get the actual transforms
            AZStd::vector<float> boneTransforms;
            GetBoneTransformsFromActorInstance(actorInstance, boneTransforms, skinningMethod);

            size_t floatsPerBone = 0;
            if (skinningMethod == EMotionFX::Integration::SkinningMethod::Linear)
            {
                floatsPerBone = LinearSkinningFloatsPerBone;
            }
            else if (skinningMethod == EMotionFX::Integration::SkinningMethod::DualQuat)
            {
                floatsPerBone = DualQuaternionSkinningFloatsPerBone;
            }
            else
            {
                AZ_Error("ActorAsset", false, "Unsupported EMotionFX skinning method.");
            }

            // Create a buffer and populate it with the transforms
            RPI::CommonBufferDescriptor descriptor;
            descriptor.m_bufferData = boneTransforms.data();
            descriptor.m_bufferName = AZStd::string::format("BoneTransformBuffer_%s_%s", actorInstance->GetActor()->GetName(), Uuid::CreateRandom().ToString<AZStd::string>().c_str());
            descriptor.m_byteCount = boneTransforms.size() * sizeof(float);
            descriptor.m_elementSize = floatsPerBone * sizeof(float);
            descriptor.m_poolType = RPI::CommonBufferPoolType::ReadOnly;
            return RPI::BufferSystemInterface::Get()->CreateBufferFromCommonPool(descriptor);
        }

    } //namespace Render
} // namespace AZ
