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

#include <SceneAPI/FbxSceneBuilder/Importers/AssImpMaterialImporter.h>
#include <AzCore/IO/FileIO.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/std/smart_ptr/make_shared.h>
#include <AzCore/std/containers/unordered_map.h>
#include <AzCore/StringFunc/StringFunc.h>
#include <AzToolsFramework/Debug/TraceContext.h>
#include <SceneAPI/FbxSceneBuilder/Importers/FbxImporterUtilities.h>
#include <SceneAPI/FbxSceneBuilder/Importers/Utilities/RenamedNodesMap.h>
#include <SceneAPI/SDKWrapper/AssImpNodeWrapper.h>
#include <SceneAPI/SDKWrapper/AssImpSceneWrapper.h>
#include <SceneAPI/SDKWrapper/AssImpMaterialWrapper.h>
#include <SceneAPI/SceneData/GraphData/MaterialData.h>
#include <SceneAPI/SceneCore/Utilities/Reporting.h>
#include <SceneAPI/SceneCore/Events/ProcessingResult.h>
#include <assimp/scene.h>

namespace AZ
{
    namespace SceneAPI
    {
        namespace FbxSceneBuilder
        {
            AssImpMaterialImporter::AssImpMaterialImporter()
            {
                BindToCall(&AssImpMaterialImporter::ImportMaterials);
            }

            void AssImpMaterialImporter::Reflect(ReflectContext* context)
            {
                SerializeContext* serializeContext = azrtti_cast<SerializeContext*>(context);
                if (serializeContext)
                {
                    serializeContext->Class<AssImpMaterialImporter, SceneCore::LoadingComponent>()->Version(1);
                }
            }

            Events::ProcessingResult AssImpMaterialImporter::ImportMaterials(AssImpSceneNodeAppendedContext& context)
            {
                AZ_TraceContext("Importer", "Material");
                if (!context.m_sourceNode.ContainsMesh())
                {
                    return Events::ProcessingResult::Ignored;
                }

                Events::ProcessingResultCombiner combinedMaterialImportResults;

                AZStd::unordered_map<int, AZStd::shared_ptr<SceneData::GraphData::MaterialData>> materialMap;
                for (int idx = 0; idx < context.m_sourceNode.m_assImpNode->mNumMeshes; ++idx)
                {
                    int meshIndex = context.m_sourceNode.m_assImpNode->mMeshes[idx];
                    aiMesh* assImpMesh = context.m_sourceScene.GetAssImpScene()->mMeshes[meshIndex];
                    AZ_Assert(assImpMesh, "Asset Importer Mesh should not be null.");
                    int materialIndex = assImpMesh->mMaterialIndex;
                    AZ_TraceContext("Material Index", materialIndex);

                    auto matFound = materialMap.find(materialIndex);

                    AZStd::shared_ptr<SceneData::GraphData::MaterialData> material;
                    AZStd::string materialName;

                    if (matFound == materialMap.end())
                    {
                        std::shared_ptr<AssImpSDKWrapper::AssImpMaterialWrapper> assImpMaterial =
                            std::shared_ptr<AssImpSDKWrapper::AssImpMaterialWrapper>(new AssImpSDKWrapper::AssImpMaterialWrapper(context.m_sourceScene.GetAssImpScene()->mMaterials[materialIndex]));

                        materialName = assImpMaterial->GetName().c_str();
                        RenamedNodesMap::SanitizeNodeName(materialName, context.m_scene.GetGraph(), context.m_currentGraphPosition, "Material");
                        AZ_TraceContext("Material Name", materialName);

                        material = AZStd::make_shared<SceneData::GraphData::MaterialData>();

                        material->SetMaterialName(assImpMaterial->GetName());
                        material->SetTexture(DataTypes::IMaterialData::TextureMapType::Diffuse,
                            ResolveTexturePath(context.m_sourceScene.GetSceneFileName(),
                                assImpMaterial->GetTextureFileName(SDKMaterial::MaterialWrapper::MaterialMapType::Diffuse)).c_str());
                        material->SetTexture(DataTypes::IMaterialData::TextureMapType::Specular,
                            ResolveTexturePath(context.m_sourceScene.GetSceneFileName(),
                                assImpMaterial->GetTextureFileName(SDKMaterial::MaterialWrapper::MaterialMapType::Specular)).c_str());
                        material->SetTexture(DataTypes::IMaterialData::TextureMapType::Bump,
                            ResolveTexturePath(context.m_sourceScene.GetSceneFileName(),
                                assImpMaterial->GetTextureFileName(SDKMaterial::MaterialWrapper::MaterialMapType::Bump)).c_str());
                        material->SetTexture(DataTypes::IMaterialData::TextureMapType::Normal,
                            ResolveTexturePath(context.m_sourceScene.GetSceneFileName(),
                                assImpMaterial->GetTextureFileName(SDKMaterial::MaterialWrapper::MaterialMapType::Normal)).c_str());
                        material->SetUniqueId(assImpMaterial->GetUniqueId());
                        material->SetDiffuseColor(assImpMaterial->GetDiffuseColor());
                        material->SetSpecularColor(assImpMaterial->GetSpecularColor());
                        material->SetEmissiveColor(assImpMaterial->GetEmissiveColor());
                        material->SetShininess(assImpMaterial->GetShininess());

                        material->SetUseColorMap(assImpMaterial->GetUseColorMap());
                        material->SetBaseColor(assImpMaterial->GetBaseColor());

                        material->SetUseMetallicMap(assImpMaterial->GetUseMetallicMap());
                        material->SetMetallicFactor(assImpMaterial->GetMetallicFactor());
                        material->SetTexture(
                            DataTypes::IMaterialData::TextureMapType::Metallic,
                            ResolveTexturePath(context.m_sourceScene.GetSceneFileName(),
                                assImpMaterial->GetTextureFileName(SDKMaterial::MaterialWrapper::MaterialMapType::Metallic).c_str()));

                        material->SetUseRoughnessMap(assImpMaterial->GetUseRoughnessMap());
                        material->SetRoughnessFactor(assImpMaterial->GetRoughnessFactor());
                        material->SetTexture(
                            DataTypes::IMaterialData::TextureMapType::Roughness,
                            ResolveTexturePath(context.m_sourceScene.GetSceneFileName(),
                                assImpMaterial->GetTextureFileName(SDKMaterial::MaterialWrapper::MaterialMapType::Roughness).c_str()));

                        material->SetUseEmissiveMap(assImpMaterial->GetUseEmissiveMap());
                        material->SetEmissiveIntensity(assImpMaterial->GetEmissiveIntensity());

                        material->SetUseAOMap(assImpMaterial->GetUseAOMap());
                        material->SetTexture(
                            DataTypes::IMaterialData::TextureMapType::AmbientOcclusion,
                            ResolveTexturePath(context.m_sourceScene.GetSceneFileName(),
                                assImpMaterial->GetTextureFileName(SDKMaterial::MaterialWrapper::MaterialMapType::AmbientOcclusion).c_str()));

                        material->SetTexture(
                            DataTypes::IMaterialData::TextureMapType::Emissive,
                            ResolveTexturePath(context.m_sourceScene.GetSceneFileName(),
                                assImpMaterial->GetTextureFileName(SDKMaterial::MaterialWrapper::MaterialMapType::Emissive).c_str()));

                        material->SetTexture(
                            DataTypes::IMaterialData::TextureMapType::BaseColor,
                            ResolveTexturePath(context.m_sourceScene.GetSceneFileName(),
                                assImpMaterial->GetTextureFileName(SDKMaterial::MaterialWrapper::MaterialMapType::BaseColor).c_str()));

                        AZ_Assert(material, "Failed to allocate scene material data.");
                        if (!material)
                        {
                            combinedMaterialImportResults += Events::ProcessingResult::Failure;
                            continue;
                        }

                        materialMap[materialIndex] = material;
                    }
                    else
                    {
                        material = matFound->second;
                        materialName = material.get()->GetMaterialName();
                    }

                    Events::ProcessingResult materialResult;
                    Containers::SceneGraph::NodeIndex newIndex =
                        context.m_scene.GetGraph().AddChild(context.m_currentGraphPosition, materialName.c_str());

                    AZ_Assert(newIndex.IsValid(), "Failed to create SceneGraph node for attribute.");
                    if (!newIndex.IsValid())
                    {
                        combinedMaterialImportResults += Events::ProcessingResult::Failure;
                        continue;
                    }

                    AssImpSceneAttributeDataPopulatedContext dataPopulated(context, material, newIndex, materialName);
                    materialResult = Events::Process(dataPopulated);

                    if (materialResult != Events::ProcessingResult::Failure)
                    {
                        materialResult = SceneAPI::FbxSceneBuilder::AddAttributeDataNodeWithContexts(dataPopulated);
                    }

                    combinedMaterialImportResults += materialResult;
                }

                return combinedMaterialImportResults.GetResult();
            }

            AZStd::string AssImpMaterialImporter::ResolveTexturePath(const AZStd::string& sceneFilePath, const AZStd::string& textureFilePath) const
            {
                if (textureFilePath.empty())
                {
                    return textureFilePath;
                }


                AZStd::string texturePathRelativeToScene;
                AZStd::string cleanedUpSceneFilePath(sceneFilePath);
                AZ::StringFunc::Path::StripFullName(cleanedUpSceneFilePath);
                AZ::StringFunc::Path::Join(cleanedUpSceneFilePath.c_str(), textureFilePath.c_str(), texturePathRelativeToScene);

                // If the texture did start with marker to change directories upward, then it's relative to the FBX file,
                // and that path needs to be resolved. It can't be resolved later. This was handled internally by FBX SDK,
                // but is not handled by AssImp.
                if (textureFilePath.starts_with(".."))
                {
                    // Not checking for the file existing because it may not be there yet.
                    return texturePathRelativeToScene;
                }

                // The engine only supports relative paths to scan folders.
                // FBX files may have paths to textures, relative to the FBX file. Check for those, first.
                if (AZ::IO::FileIOBase::GetInstance()->Exists(texturePathRelativeToScene.c_str()))
                {
                    return texturePathRelativeToScene;
                }


                return textureFilePath;
            }
        } // namespace FbxSceneBuilder
    } // namespace SceneAPI
} // namespace AZ
