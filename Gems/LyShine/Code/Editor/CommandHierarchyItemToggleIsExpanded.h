/*
 * Copyright (c) Contributors to the Open 3D Engine Project
 * 
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */
#pragma once

#include <QUndoCommand>

class CommandHierarchyItemToggleIsExpanded
    : public QUndoCommand
{
public:

    void undo() override;
    void redo() override;

    // IMPORTANT: We DON'T want this command to support mergeWith().
    // Otherwise we leave commands on the undo stack that have no
    // effect (NOOP).
    //
    // To avoid the NOOPs, we can either:
    //
    // (1) Delete the NOPs from the undo stack.
    // or
    // (2) NOT support mergeWith().
    //
    // The problem with (1) is that it only allows odd number of
    // state changes to be undoable. (2) is more consistent
    // by making all state changes undoable.

    static void Push(UndoStack* stack,
        HierarchyWidget* hierarchy,
        HierarchyItem* item);

private:

    CommandHierarchyItemToggleIsExpanded(UndoStack* stack,
        HierarchyWidget* hierarchy,
        HierarchyItem* item);

    void SetIsExpanded(bool isExpanded);

    UndoStack* m_stack;

    HierarchyWidget* m_hierarchy;

    AZ::EntityId m_id;
    bool m_toIsExpanded;
};
