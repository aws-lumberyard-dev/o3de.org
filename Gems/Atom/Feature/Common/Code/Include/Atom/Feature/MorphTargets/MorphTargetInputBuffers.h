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

#pragma once

#include <Atom/Feature/SkinnedMesh/SkinnedMeshVertexStreams.h>
#include <Atom/Feature/SkinnedMesh/SkinnedMeshInstance.h>
#include <Atom/RPI.Reflect/Model/ModelLodAsset.h>
#include <Atom/RPI.Reflect/Material/MaterialAsset.h>
#include <Atom/RHI.Reflect/Base.h>
#include <AtomCore/Instance/InstanceData.h>

#include <AzCore/Math/Aabb.h>
#include <AzCore/Asset/AssetCommon.h>

namespace AZ
{
    namespace RHI
    {
        class BufferView;
        class IndexBufferView;
    }
    
    namespace RPI
    {
        class Model;
        class ShaderResourceGroup;
    }
    
    namespace Render
    {
        //! The input to the morph target pass, including the delta values for a fully morphed pose
        //! and the index of the target vertex that is going to be modified
        //! The morph target pass will read these values, apply a weight, and write the accumulated deltas
        //! to an intermediate buffer that will be consumed by the skinning pass
        class MorphTargetInputBuffers
            : public AZStd::intrusive_base
        {
        public:
            AZ_CLASS_ALLOCATOR(MorphTargetInputBuffers, AZ::SystemAllocator, 0);
            MorphTargetInputBuffers(uint32_t vertexCount, const AZStd::vector<uint32_t>& vertexDeltas, const AZStd::string& bufferNamePrefix);

            //! Set the buffer views and vertex count on the given SRG
            void SetBufferViewsOnShaderResourceGroup(const Data::Instance<RPI::ShaderResourceGroup>& perInstanceSRG);
        private:
            Data::Instance<RPI::Buffer> m_vertexDeltas;
        };

        struct MorphTargetMetaData
        {
            float m_minWeight;
            float m_maxWeight;
            float m_minDelta;
            float m_maxDelta;
            uint32_t m_vertexCount;
            uint32_t m_positionOffset;
        };

        namespace MorphTargetConstants
        {
            // Morph targets output deltas with three 32-bit components
            static constexpr uint32_t s_unpackedMorphTargetDeltaSizeInBytes = 12;
            // Position, normal, tangent, and bitangent is output for each morph
            static constexpr uint32_t s_morphTargetDeltaTypeCount = 4;
            static constexpr uint32_t s_invalidDeltaOffset = std::numeric_limits<uint32_t>::max();
        }

        //! Unlike MorphTargetMetaData which is the same for every instance of a given skinned mesh,
        //! this data varies between instances
        struct MorphTargetInstanceMetaData
        {
            uint32_t m_accumulatedPositionDeltaOffsetInBytes;
            uint32_t m_accumulatedNormalDeltaOffsetInBytes;
            uint32_t m_accumulatedTangentDeltaOffsetInBytes;
            uint32_t m_accumulatedBitangentDeltaOffsetInBytes;
        };

    }// Render
}// AZ
