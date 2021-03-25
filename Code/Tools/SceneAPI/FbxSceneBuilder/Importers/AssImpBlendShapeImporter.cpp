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

#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/std/smart_ptr/make_shared.h>
#include <AzCore/std/string/conversions.h>
#include <AzToolsFramework/Debug/TraceContext.h>
#include <SceneAPI/FbxSceneBuilder/Importers/AssImpBlendShapeImporter.h>
#include <SceneAPI/FbxSceneBuilder/Importers/Utilities/RenamedNodesMap.h>
#include <SceneAPI/FbxSceneBuilder/Importers/FbxImporterUtilities.h>
#include <SceneAPI/FbxSceneBuilder/FbxSceneSystem.h>
#include <SceneAPI/SDKWrapper/AssImpNodeWrapper.h>
#include <SceneAPI/SDKWrapper/AssImpSceneWrapper.h>
#include <SceneAPI/SDKWrapper/AssImpTypeConverter.h>
#include <SceneAPI/SceneCore/Utilities/Reporting.h>
#include <SceneAPI/SceneData/GraphData/MeshData.h>
#include <SceneAPI/SceneData/GraphData/SkinMeshData.h>
#include <SceneAPI/SceneData/GraphData/BlendShapeData.h>
#include <assimp/scene.h>

namespace AZ
{
    namespace SceneAPI
    {
        namespace FbxSceneBuilder
        {
            AssImpBlendShapeImporter::AssImpBlendShapeImporter()
            {
                BindToCall(&AssImpBlendShapeImporter::ImportBlendShapes);
            }

            void AssImpBlendShapeImporter::Reflect(ReflectContext* context)
            {
                SerializeContext* serializeContext = azrtti_cast<SerializeContext*>(context);
                if (serializeContext)
                {
                    serializeContext->Class<AssImpBlendShapeImporter, SceneCore::LoadingComponent>()->Version(1);
                }
            }

            Events::ProcessingResult AssImpBlendShapeImporter::ImportBlendShapes(AssImpSceneNodeAppendedContext& context)
            {
                AZ_TraceContext("Importer", "Blend Shapes");
                int numMesh = context.m_sourceNode.GetAssImpNode()->mNumMeshes;
                bool animMeshExists = false;
                for (int idx = 0; idx < numMesh; idx++)
                {
                    int meshId = context.m_sourceNode.GetAssImpNode()->mMeshes[idx];
                    aiMesh* aiMesh = context.m_sourceScene.GetAssImpScene()->mMeshes[meshId];
                    if (aiMesh->mNumAnimMeshes)
                    {
                        animMeshExists = true;
                        break;
                    }
                }

                if (!animMeshExists)
                {
                    return Events::ProcessingResult::Ignored;
                }

                Events::ProcessingResultCombiner combinedBlendShapeResult;

                for (int meshIdx = 0; meshIdx < numMesh; meshIdx++)
                {
                    int meshId = context.m_sourceNode.GetAssImpNode()->mMeshes[meshIdx];
                    aiMesh* aiMesh = context.m_sourceScene.GetAssImpScene()->mMeshes[meshId];
                    if (!aiMesh->mNumAnimMeshes)
                    {
                        continue;
                    }

                    for (int animIdx = 0; animIdx < aiMesh->mNumAnimMeshes; animIdx++)
                    {
                        AZStd::shared_ptr<SceneData::GraphData::BlendShapeData> blendShapeData =
                            AZStd::make_shared<SceneData::GraphData::BlendShapeData>();

                        aiAnimMesh* aiAnimMesh = aiMesh->mAnimMeshes[animIdx];
                        AZStd::string nodeName(aiAnimMesh->mName.C_Str());
                        size_t dotIndex = nodeName.rfind('.');
                        if (dotIndex != AZStd::string::npos)
                        {
                            nodeName.erase(0, dotIndex + 1);
                        }
                        RenamedNodesMap::SanitizeNodeName(nodeName, context.m_scene.GetGraph(), context.m_currentGraphPosition, "BlendShape");
                        AZ_TraceContext("Blend shape name", nodeName);

                        int firstMeshVertexIndex = -1;
                        int previousMeshVertexIndex = -1;

                        int verticesInMeshFace = 0;

                        for (int vertIdx = 0; vertIdx < aiAnimMesh->mNumVertices; ++vertIdx)
                        {
                            AZ::Vector3 vertex(AssImpSDKWrapper::AssImpTypeConverter::ToVector3(aiAnimMesh->mVertices[vertIdx]));
                   
                            context.m_sourceSceneSystem.SwapVec3ForUpAxis(vertex);
                            context.m_sourceSceneSystem.ConvertUnit(vertex);


                            AZ::Vector3 normal;
                            if (aiAnimMesh->HasNormals())
                            {
                                normal = AssImpSDKWrapper::AssImpTypeConverter::ToVector3(aiAnimMesh->mNormals[vertIdx]);
                                context.m_sourceSceneSystem.SwapVec3ForUpAxis(normal);
                                normal.NormalizeSafe();
                            }
                            blendShapeData->AddVertex(vertex, normal);
                            blendShapeData->SetVertexIndexToControlPointIndexMap(vertIdx, vertIdx);
                        }

                        // aiAnimMesh just has a list of positions for vertices. The face indices are on the original mesh.
                        for (int faceIdx = 0; faceIdx < aiMesh->mNumFaces; ++faceIdx)
                        {
                            aiFace face = aiMesh->mFaces[faceIdx];
                            DataTypes::IBlendShapeData::Face blendFace;
                            if (face.mNumIndices != 3)
                            {
                                // AssImp should have triangulated everything, so if this happens then someone has
                                // probably changed AssImp's import settings. The engine only supports triangles.
                                AZ_Error(
                                    Utilities::ErrorWindow, false,
                                    "Mesh for node %s has a face with %d vertices, only 3 vertices are supported per face.",
                                    nodeName.c_str(),
                                    face.mNumIndices);
                                continue;
                            }
                            for (int idx = 0; idx < face.mNumIndices; ++idx)
                            {
                                blendFace.vertexIndex[idx] = face.mIndices[idx];
                            }

                            blendShapeData->AddFace(blendFace);
                        }

                        // Report problem if no vertex or face converted to MeshData
                        if (blendShapeData->GetVertexCount() <= 0 || blendShapeData->GetFaceCount() <= 0)
                        {
                            AZ_Error(Utilities::ErrorWindow, false, "Missing geometry data in blendshape node %s.", nodeName.c_str());
                            return Events::ProcessingResult::Failure;
                        }

                        Containers::SceneGraph::NodeIndex newIndex =
                            context.m_scene.GetGraph().AddChild(context.m_currentGraphPosition, nodeName.c_str());

                        Events::ProcessingResult blendShapeResult;
                        AssImpSceneAttributeDataPopulatedContext dataPopulated(context, blendShapeData, newIndex, nodeName);
                        blendShapeResult = Events::Process(dataPopulated);

                        if (blendShapeResult != Events::ProcessingResult::Failure)
                        {
                            blendShapeResult = AddAttributeDataNodeWithContexts(dataPopulated);
                        }
                        combinedBlendShapeResult += blendShapeResult;
                    }

                }

                return combinedBlendShapeResult.GetResult();
            }
        } // namespace FbxSceneBuilder
    } // namespace SceneAPI
} // namespace AZ
