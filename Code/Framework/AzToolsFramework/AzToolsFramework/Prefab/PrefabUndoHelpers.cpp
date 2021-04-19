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

#include <AzToolsFramework/Prefab/PrefabDomUtils.h>
#include <AzToolsFramework/Prefab/PrefabUndo.h>
#include <AzToolsFramework/API/ToolsApplicationAPI.h>

namespace AzToolsFramework
{
    namespace Prefab
    {
        namespace PrefabUndoHelpers
        {
            void UpdatePrefabInstance(
                const Instance& instance, AZStd::string_view undoMessage, const PrefabDom& instanceDomBeforeUpdate,
                UndoSystem::URSequencePoint* undoBatch)
            {
                PrefabDom instanceDomAfterUpdate;
                PrefabDomUtils::StoreInstanceInPrefabDom(instance, instanceDomAfterUpdate);

                PrefabUndoInstance* state = aznew Prefab::PrefabUndoInstance(undoMessage);
                state->Capture(instanceDomBeforeUpdate, instanceDomAfterUpdate, instance.GetTemplateId());
                state->SetParent(undoBatch);
                state->Redo();
            }

            void AddLink(
                AZStd::string_view undoMessage, TemplateId sourceTemplateId, TemplateId targetTemplateId, PrefabDomReference patch,
                const InstanceAlias& instanceAlias, UndoSystem::URSequencePoint* undoBatch)
            {
                auto linkAddUndo = aznew PrefabUndoInstanceLink(undoMessage);
                linkAddUndo->Capture(targetTemplateId, sourceTemplateId, instanceAlias, patch, InvalidLinkId);
                linkAddUndo->SetParent(undoBatch);
                linkAddUndo->Redo();
            }
        }
    } // namespace Prefab
} // namespace AzToolsFramework
