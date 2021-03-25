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

#include "RuntimeAsset.h"

#include <AzCore/Component/Entity.h>

namespace ScriptCanvasRuntimeAssetCpp
{
    enum class RuntimeDataVersion
    {
        AddDependencies = 3,
        ChangeScriptRequirementToAsset,
        // add your entry above
        Current
    };

    enum class FunctionRuntimeDataVersion
    {
        MergeBackEnd2dotZero,
        AddSubgraphInterface,
        RemoveLegacyData,
        RemoveConnectionToRuntimeData,
        // add your entry above
        Current
    };
}

namespace ScriptCanvas
{
    RuntimeData::RuntimeData(RuntimeData&& other)
    {
        *this = AZStd::move(other);
    }

    RuntimeData& RuntimeData::operator=(RuntimeData&& other)
    {
        if (this != &other)
        {
            m_input = AZStd::move(other.m_input);
            m_debugMap = AZStd::move(other.m_debugMap);
            m_script = AZStd::move(other.m_script);
            m_requiredAssets = AZStd::move(other.m_requiredAssets);
            m_requiredScriptEvents = AZStd::move(other.m_requiredScriptEvents);
        }

        return *this;
    }

    void RuntimeData::Reflect(AZ::ReflectContext* reflectContext)
    {
        Translation::RuntimeInputs::Reflect(reflectContext);

        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(reflectContext))
        {
            serializeContext->Class<RuntimeData>()
                ->Version(static_cast<int>(ScriptCanvasRuntimeAssetCpp::RuntimeDataVersion::Current))
                ->Field("input", &RuntimeData::m_input)
                ->Field("debugMap", &RuntimeData::m_debugMap)
                ->Field("script", &RuntimeData::m_script)
                ->Field("requiredAssets", &RuntimeData::m_requiredAssets)
                ->Field("requiredScriptEvents", &RuntimeData::m_requiredScriptEvents)
                ;
        }
    }

    bool RuntimeData::RequiresStaticInitialization() const
    {
        return !m_cloneSources.empty();
    }

    ////////////////////////
    // SubgraphInterfaceData
    ////////////////////////

    void SubgraphInterfaceData::Reflect(AZ::ReflectContext* reflectContext)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(reflectContext))
        {
            serializeContext->Class<SubgraphInterfaceData>()
                ->Version(static_cast<int>(ScriptCanvasRuntimeAssetCpp::FunctionRuntimeDataVersion::Current))
                ->Field("m_name", &SubgraphInterfaceData::m_name)
                ->Field("m_version", &SubgraphInterfaceData::m_version)
                ->Field("interface", &SubgraphInterfaceData::m_interface)
                ;
        }
    }

    SubgraphInterfaceData::SubgraphInterfaceData(SubgraphInterfaceData&& other)
    {
        *this = AZStd::move(other);
    }
    
    SubgraphInterfaceData& SubgraphInterfaceData::operator=(SubgraphInterfaceData&& other)
    {
        if (this != &other)
        {
            m_name = AZStd::move(other.m_name);
            m_version = AZStd::move(other.m_version);
            m_interface = AZStd::move(other.m_interface);
        }

        return *this;
    }
}
