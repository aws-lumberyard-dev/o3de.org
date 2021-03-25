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

#include "precompiled.h"

#include <AzCore/Serialization/IdUtils.h>
#include <Editor/Assets/ScriptCanvasAssetReference.h>
#include <Editor/Assets/ScriptCanvasAssetReferenceContainer.h>

namespace ScriptCanvasEditor
{
    void ScriptCanvasAssetReference::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<ScriptCanvasAssetReference>()
                ->DataContainer<ScriptCanvasAssetReferenceContainer>()
                ;
        }
    }

    ScriptCanvasAssetReference::ScriptCanvasAssetReference(AZ::Data::Asset<ScriptCanvasAsset> sliceAsset, bool storeInObjectStream)
    {
        SetAsset(sliceAsset);
        SetAssetDataStoredInternally(storeInObjectStream);
    }

    void ScriptCanvasAssetReference::SetAsset(AZ::Data::Asset<ScriptCanvasAsset> sliceAsset)
    {
        m_asset = sliceAsset;
    }

    const AZ::Data::Asset<ScriptCanvasAsset>& ScriptCanvasAssetReference::GetAsset() const
    {
        return m_asset;
    }

    AZ::Data::Asset<ScriptCanvasAsset>& ScriptCanvasAssetReference::GetAsset()
    {
        return m_asset;
    }

    void ScriptCanvasAssetReference::SetAssetDataStoredInternally(bool storeInObjectStream)
    {
        m_storeInObjectStream = storeInObjectStream;
    }

    bool ScriptCanvasAssetReference::GetAssetDataStoredInternally() const
    {
        return m_storeInObjectStream;
    }

}