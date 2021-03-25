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

#include <SceneAPI/SceneData/GraphData/MeshVertexBitangentData.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/RTTI/BehaviorContext.h>

namespace AZ
{
    namespace SceneData
    {
        namespace GraphData
        {
            void MeshVertexBitangentData::Reflect(ReflectContext* context)
            {
                SerializeContext* serializeContext = azrtti_cast<SerializeContext*>(context);
                if (serializeContext)
                {
                    serializeContext->Class<MeshVertexBitangentData>()->Version(1);
                }

                BehaviorContext* behaviorContext = azrtti_cast<BehaviorContext*>(context);
                if (behaviorContext)
                {
                    behaviorContext->Class<MeshVertexBitangentData>()
                        ->Attribute(AZ::Script::Attributes::Scope, AZ::Script::Attributes::ScopeFlags::Common)
                        ->Attribute(AZ::Script::Attributes::Module, "scene")
                        ->Method("GetCount", &MeshVertexBitangentData::GetCount)
                        ->Method("GetBitangent", &MeshVertexBitangentData::GetBitangent)
                        ->Method("GetBitangentSetIndex", &MeshVertexBitangentData::GetBitangentSetIndex)
                        ->Method("GetTangentSpace", &MeshVertexBitangentData::GetTangentSpace)
                        ->Enum<(int)SceneAPI::DataTypes::TangentSpace::EMotionFX>("EMotionFX")
                        ->Enum<(int)SceneAPI::DataTypes::TangentSpace::FromFbx>("FromFbx")
                        ->Enum<(int)SceneAPI::DataTypes::TangentSpace::MikkT>("MikkT");
                }
            }

            size_t MeshVertexBitangentData::GetCount() const
            {
                return m_bitangents.size();
            }


            const AZ::Vector3& MeshVertexBitangentData::GetBitangent(size_t index) const
            {
                AZ_Assert(index < m_bitangents.size(), "Invalid index %i for mesh bitangents.", index);
                return m_bitangents[index];
            }


            void MeshVertexBitangentData::ReserveContainerSpace(size_t numVerts)
            {
                m_bitangents.reserve(numVerts);
            }


            void MeshVertexBitangentData::Resize(size_t numVerts)
            {
                m_bitangents.resize(numVerts);
            }


            void MeshVertexBitangentData::AppendBitangent(const AZ::Vector3& bitangent)
            {
                m_bitangents.push_back(bitangent);
            }


            void MeshVertexBitangentData::SetBitangent(size_t vertexIndex, const AZ::Vector3& bitangent)
            {
                m_bitangents[vertexIndex] = bitangent;
            }


            void MeshVertexBitangentData::SetBitangentSetIndex(size_t setIndex)
            {
                m_setIndex = setIndex;
            }


            size_t MeshVertexBitangentData::GetBitangentSetIndex() const
            {
                return m_setIndex;
            }


            AZ::SceneAPI::DataTypes::TangentSpace MeshVertexBitangentData::GetTangentSpace() const
            { 
                return m_tangentSpace;
            }


            void MeshVertexBitangentData::SetTangentSpace(AZ::SceneAPI::DataTypes::TangentSpace space)
            {
                m_tangentSpace = space;
            }

            void MeshVertexBitangentData::GetDebugOutput(AZ::SceneAPI::Utilities::DebugOutput& output) const
            {
                output.Write("Bitangents", m_bitangents);
                output.Write("TangentSpace", aznumeric_cast<int64_t>(m_tangentSpace));
            }
        } // GraphData
    } // SceneData
} // AZ
