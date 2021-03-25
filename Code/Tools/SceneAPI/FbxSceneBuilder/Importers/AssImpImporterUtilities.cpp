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

#include <SceneAPI/FbxSceneBuilder/Importers/AssImpImporterUtilities.h>

#include <AzCore/Debug/Trace.h>
#include <AzCore/StringFunc/StringFunc.h>

#include <assimp/scene.h>

namespace AZ
{
    namespace SceneAPI
    {
        namespace FbxSceneBuilder
        {
            bool IsSkinnedMesh(const aiNode& node, const aiScene& scene)
            {
                unsigned boneCount = 0;

                for (unsigned mesh = 0; mesh < node.mNumMeshes; ++mesh)
                {
                    if (scene.mMeshes[node.mMeshes[mesh]]->HasBones())
                    {
                        ++boneCount;
                    }
                }

                if (boneCount > 1 && boneCount != node.mNumMeshes)
                {
                    AZ_Error("AssImpImporterUtilities", false, "Node has %d meshes but only %d are skinned.  "
                        "This is unexpected and may result in errors", node.mNumMeshes, boneCount);
                }

                return boneCount > 0;
            }

            bool IsPivotNode(const aiString& nodeName, size_t* pos)
            {
                AZStd::string_view name(nodeName.C_Str(), nodeName.length);

                size_t myPos;

                if (!pos)
                {
                    pos = &myPos;
                }

                *pos = AZ::StringFunc::Find(name, PivotNodeMarker);

                return *pos != name.npos;
            }

            void SplitPivotNodeName(const aiString& nodeName, size_t pivotPos, AZStd::string_view& baseNodeName, AZStd::string_view& pivotType)
            {
                AZStd::string_view nodeNameView(nodeName.data, nodeName.length);
                baseNodeName = AZStd::string_view(nodeNameView.substr(0, pivotPos));
                pivotType = AZStd::string_view(nodeNameView.substr(pivotPos + sizeof PivotNodeMarker - 1));
            }

            aiMatrix4x4 GetConcatenatedLocalTransform(const aiNode* currentNode)
            {
                aiNode* parent = currentNode->mParent;
                aiMatrix4x4 combinedTransform = currentNode->mTransformation;

                while (parent)
                {
                    size_t pos;

                    if (IsPivotNode(parent->mName, &pos))
                    {
                        combinedTransform = parent->mTransformation * combinedTransform;
                        parent = parent->mParent;
                    }
                    else
                    {
                        break;
                    }
                }

                return combinedTransform;
            }
        } // namespace FbxSceneBuilder
    } // namespace SceneAPI
} // namespace AZ
