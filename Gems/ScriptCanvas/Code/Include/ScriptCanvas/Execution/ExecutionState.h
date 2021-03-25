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

#include <AzCore/Component/EntityId.h>
#include <AzCore/EBus/Event.h>
#include <AzCore/Memory/Memory.h>
#include <AzCore/Memory/SystemAllocator.h>
#include <AzCore/RTTI/RTTI.h>
#include <AzCore/Script/ScriptAsset.h>

#include <ScriptCanvas/Asset/RuntimeAsset.h>
#include <ScriptCanvas/Execution/ExecutionStateDeclarations.h>
#include <ScriptCanvas/Grammar/PrimitivesDeclarations.h>

namespace AZ
{
    class ReflectContext;

    struct BehaviorValueParameter;
}

namespace ScriptCanvas
{
    class RuntimeComponent;

    struct ExecutionStateConfig
    {
        AZ::Data::Asset<RuntimeAsset> asset;
        RuntimeComponent& component;
        const RuntimeData& runtimeData;

        ExecutionStateConfig(AZ::Data::Asset<RuntimeAsset> asset, RuntimeComponent& component);
    };
    
    class ExecutionState
        : public AZStd::enable_shared_from_this<ExecutionState>
    {
    public:
        AZ_RTTI(ExecutionState, "{85C66E59-F012-460E-9756-B36819753F4D}", AZStd::enable_shared_from_this<ExecutionState>);
        AZ_CLASS_ALLOCATOR(ExecutionState, AZ::SystemAllocator, 0);

        static ExecutionStatePtr Create(const ExecutionStateConfig& config);

        static void Reflect(AZ::ReflectContext* reflectContext);

        const RuntimeComponent* m_component = nullptr;
        
        ExecutionState(const ExecutionStateConfig& config);

        virtual ~ExecutionState() = default;

        virtual void Execute() = 0;

        AZ::Data::AssetId GetAssetId() const;

        AZ::ComponentId GetComponentId() const;

        AZ::EntityId GetEntityId() const;

        virtual ExecutionMode GetExecutionMode() const = 0;

        GraphIdentifier GetGraphIdentifier() const;

        GraphIdentifier GetGraphIdentifier(const AZ::Data::AssetId& id) const;

        AZ::EntityId GetScriptCanvasId() const;

        virtual void Initialize() = 0;

        ExecutionStatePtr SharedFromThis();

        ExecutionStateConstPtr SharedFromThisConst() const;

        virtual void StopExecution() = 0;

        AZStd::string ToString() const;

        ExecutionStateWeakPtr WeakFromThis();

        ExecutionStateWeakConstPtr WeakFromThisConst() const;
    };

} 
