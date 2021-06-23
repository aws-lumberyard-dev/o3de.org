/*
 * Copyright (c) Contributors to the Open 3D Engine Project
 * 
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */
#pragma once

#include <GraphCanvas/Widgets/EditorContextMenu/ContextMenuActions/ConstructMenuActions/ConstructContextMenuAction.h>

namespace GraphCanvas
{
    class AddBookmarkMenuAction
        : public ConstructContextMenuAction
    {
    public:
        AZ_CLASS_ALLOCATOR(AddBookmarkMenuAction, AZ::SystemAllocator, 0);

        AddBookmarkMenuAction(QObject* parent);
        virtual ~AddBookmarkMenuAction() = default;

        SceneReaction TriggerAction(const AZ::Vector2& scenePos) override;
    };
}
