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

#include <GraphCanvas/Widgets/EditorContextMenu/EditorContextMenu.h>

#include <GraphCanvas/Widgets/EditorContextMenu/ContextMenuActions/AlignmentMenuActions/AlignmentActionsMenuGroup.h>
#include <GraphCanvas/Widgets/EditorContextMenu/ContextMenuActions/EditMenuActions/EditActionsMenuGroup.h>
#include <GraphCanvas/Widgets/EditorContextMenu/ContextMenuActions/NodeGroupMenuActions/NodeGroupActionsMenuGroup.h>
#include <GraphCanvas/Widgets/EditorContextMenu/ContextMenuActions/CommentMenuActions/CommentActionsMenuGroup.h>

namespace GraphCanvas
{
    class CommentContextMenu
        : public EditorContextMenu
    {
    public:
        CommentContextMenu(EditorId editorId, QWidget* parent = nullptr);
        ~CommentContextMenu() override = default;
        
    protected:
    
        void OnRefreshActions(const GraphId& graphId, const AZ::EntityId& targetMemberId) override;

    private:

        EditActionsMenuGroup m_editActionGroup;
        CommentActionsMenuGroup m_commentActionGroup;
        NodeGroupActionsMenuGroup m_nodeGroupActionGroup;
        AlignmentActionsMenuGroup m_alignmentActionGroup;

        ContextMenuAction* m_createPresetFrom;
        ApplyCommentPresetMenuActionGroup m_applyCommentPresets;
    };
}
