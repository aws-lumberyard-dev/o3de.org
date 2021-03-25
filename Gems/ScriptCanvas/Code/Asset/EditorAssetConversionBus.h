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

#include <AzCore/EBus/EBus.h>
#include <AzCore/Outcome/Outcome.h>
#include <AzCore/Asset/AssetCommon.h>
#include <ScriptCanvas/Translation/Translation.h>

namespace AZ
{
    class ScriptAsset;
}

namespace ScriptCanvas
{
    class ScriptCanvasFunctionAsset;
    class RuntimeAsset;
    class SubgraphInterfaceAsset;
}

namespace ScriptCanvasEditor
{
    class ScriptCanvasAsset;
    class ScriptCanvasFunctionAsset;

    class EditorAssetConversionBusTraits
        : public AZ::EBusTraits
    {
    public:
        static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;
      
        virtual AZ::Data::Asset<ScriptCanvasEditor::ScriptCanvasAsset> LoadAsset(AZStd::string_view graphPath) = 0;
        virtual AZ::Data::Asset<ScriptCanvasEditor::ScriptCanvasFunctionAsset> LoadFunctionAsset(AZStd::string_view graphPath) = 0;
        virtual AZ::Outcome<ScriptCanvas::Translation::LuaAssetResult, AZStd::string> CreateLuaAsset(const AZ::Data::Asset<ScriptCanvasEditor::ScriptCanvasAsset>& editAsset, AZStd::string_view graphPathForRawLuaFile) = 0;
        virtual AZ::Outcome<ScriptCanvas::Translation::LuaAssetResult, AZStd::string> CreateLuaFunctionAsset(const AZ::Data::Asset<ScriptCanvasEditor::ScriptCanvasFunctionAsset>& editAsset, AZStd::string_view graphPathForRawLuaFile) = 0;
        virtual AZ::Outcome<AZ::Data::Asset<ScriptCanvas::RuntimeAsset>, AZStd::string> CreateRuntimeAsset(const AZ::Data::Asset<ScriptCanvasEditor::ScriptCanvasAsset>& editAsset) = 0;
        virtual AZ::Outcome<AZ::Data::Asset<ScriptCanvas::SubgraphInterfaceAsset>, AZStd::string> CreateFunctionRuntimeAsset(const AZ::Data::Asset<ScriptCanvasEditor::ScriptCanvasFunctionAsset>& editAsset) = 0;
    };

    using EditorAssetConversionBus = AZ::EBus<EditorAssetConversionBusTraits>;
}
