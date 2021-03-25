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

#include <SceneAPI/SceneCore/Components/GenerationComponent.h>
#include <SceneAPI/SceneCore/Containers/Scene.h>
#include <AzCore/RTTI/RTTI.h>

namespace AZ::SceneAPI::DataTypes { class IMeshData; }
namespace AZ::SceneAPI::DataTypes { class IMeshVertexUVData; }
namespace AZ::SceneAPI::DataTypes { class IMeshVertexTangentData; }
namespace AZ::SceneAPI::DataTypes { class IMeshVertexBitangentData; }
namespace AZ::SceneAPI::DataTypes { enum class TangentSpace; }

namespace AZ::SceneGenerationComponents
{
    struct TangentGenerateContext
        : public AZ::SceneAPI::Events::ICallContext
    {
        AZ_RTTI(TangentGenerateContext, "{E836F8F8-5A66-497C-89CC-2D37D741CCAA}", AZ::SceneAPI::Events::ICallContext)

        TangentGenerateContext(AZ::SceneAPI::Containers::Scene& scene)
            : m_scene(scene) {}
        TangentGenerateContext& operator=(const TangentGenerateContext& other) = delete;

        AZ::SceneAPI::Containers::Scene& GetScene() { return m_scene; }
        const AZ::SceneAPI::Containers::Scene& GetScene() const { return m_scene; }

    private:
        AZ::SceneAPI::Containers::Scene& m_scene;
    };


    class TangentGenerateComponent
        : public AZ::SceneAPI::SceneCore::GenerationComponent
    {
    public:
        AZ_COMPONENT(TangentGenerateComponent, "{57743E6F-8718-491C-8A82-24A6763904F5}", AZ::SceneAPI::SceneCore::GenerationComponent);

        TangentGenerateComponent();

        static void Reflect(AZ::ReflectContext* context);
        static bool CreateTangentBitangentLayers(AZ::SceneAPI::Containers::SceneManifest& manifest, const AZ::SceneAPI::Containers::SceneGraph::NodeIndex& nodeIndex, size_t numVerts, size_t uvSetIndex, AZ::SceneAPI::DataTypes::TangentSpace tangentSpace,
            const char* spaceName, AZ::SceneAPI::Containers::SceneGraph& graph, AZ::SceneAPI::DataTypes::IMeshVertexTangentData** outTangentData, AZ::SceneAPI::DataTypes::IMeshVertexBitangentData** outBitangentData);

        AZ::SceneAPI::Events::ProcessingResult GenerateTangentData(TangentGenerateContext& context);

    private:
        bool GenerateTangentsForMesh(AZ::SceneAPI::Containers::Scene& scene, const AZ::SceneAPI::Containers::SceneGraph::NodeIndex& nodeIndex, AZ::SceneAPI::DataTypes::IMeshData* meshData);
        void UpdateFbxTangentWValues(AZ::SceneAPI::Containers::SceneGraph& graph, const AZ::SceneAPI::Containers::SceneGraph::NodeIndex& nodeIndex, const AZ::SceneAPI::DataTypes::IMeshData* meshData);
        AZStd::vector<AZ::SceneAPI::DataTypes::TangentSpace> CollectRequiredTangentSpaces(const AZ::SceneAPI::Containers::Scene& scene) const;
    };
} // namespace AZ::SceneGenerationComponents
