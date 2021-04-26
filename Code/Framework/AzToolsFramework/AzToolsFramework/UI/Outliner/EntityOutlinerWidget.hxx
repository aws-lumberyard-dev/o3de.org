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

#if !defined(Q_MOC_RUN)
#include <AzCore/Memory/SystemAllocator.h>
#include <AzCore/base.h>

#include <AzToolsFramework/API/ToolsApplicationAPI.h>
#include <AzToolsFramework/ComponentMode/EditorComponentModeBus.h>
#include <AzToolsFramework/Entity/EditorEntityInfoBus.h>
#include <AzToolsFramework/Entity/EditorEntityContextBus.h>
#include <AzToolsFramework/ToolsMessaging/EntityHighlightBus.h>
#include <AzToolsFramework/UI/Outliner/EntityOutlinerCacheBus.h>
#include <AzToolsFramework/UI/Outliner/EntityOutlinerSearchWidget.h>
#include <AzToolsFramework/UI/Outliner/EntityOutlinerWidgetInterface.h>
#include <AzToolsFramework/UI/SearchWidget/SearchWidgetTypes.hxx>

#include <QIcon>
#include <QWidget>
#endif

#pragma once

class QAction;
class QItemSelection;

namespace Ui
{
    class EntityOutlinerWidgetUI;
}

namespace AzToolsFramework
{
    class EditorEntityUiInterface;
    class EntityOutlinerListModel;
    class EntityOutlinerSortFilterProxyModel;

    namespace EntityOutliner
    {
        class DisplayOptionsMenu;
        enum class DisplaySortMode : unsigned char;
        enum class DisplayOption : unsigned char;
    }

    class EntityOutlinerWidget
        : public QWidget
        , private EditorPickModeNotificationBus::Handler
        , private EntityHighlightMessages::Bus::Handler
        , private EntityOutlinerModelNotificationBus::Handler
        , private ToolsApplicationEvents::Bus::Handler
        , private EditorEntityContextNotificationBus::Handler
        , private EditorEntityInfoNotificationBus::Handler
        , private ComponentModeFramework::EditorComponentModeNotificationBus::Handler
        , private EntityOutlinerWidgetInterface
    {
        Q_OBJECT;
    public:
        AZ_CLASS_ALLOCATOR(EntityOutlinerWidget, AZ::SystemAllocator, 0)

        explicit EntityOutlinerWidget(QWidget* pParent = NULL, Qt::WindowFlags flags = Qt::WindowFlags());
        virtual ~EntityOutlinerWidget();

    private Q_SLOTS:
        void OnSelectionChanged(const QItemSelection&, const QItemSelection&);
        void OnOpenTreeContextMenu(const QPoint& pos);

        void OnSearchTextChanged(const QString& activeTextFilter);
        void OnFilterChanged(const AzQtComponents::SearchTypeFilterList& activeTypeFilters);

        void OnSortModeChanged(EntityOutliner::DisplaySortMode sortMode);
        void OnDisplayOptionChanged(EntityOutliner::DisplayOption displayOption, bool enable);

    private:
        AzFramework::EntityContextId GetPickModeEntityContextId();

        // EntityHighlightMessages
        virtual void EntityHighlightRequested(AZ::EntityId) override;
        virtual void EntityStrongHighlightRequested(AZ::EntityId) override;

        // EditorPickModeNotificationBus
        void OnEntityPickModeStarted() override;
        void OnEntityPickModeStopped() override;

        // EditorEntityContextNotificationBus
        void OnEditorEntityCreated(const AZ::EntityId& entityId) override;
        void OnStartPlayInEditor() override;
        void OnStopPlayInEditor() override;
        void OnFocusInEntityOutliner(const EntityIdList& entityIdList) override;

        /// EditorEntityInfoNotificationBus implementation
        void OnEntityInfoUpdatedAddChildEnd(AZ::EntityId /*parentId*/, AZ::EntityId /*childId*/) override;
        void OnEntityInfoUpdatedName(AZ::EntityId entityId, const AZStd::string& /*name*/) override;

        // EditorComponentModeNotificationBus
        void EnteredComponentMode(const AZStd::vector<AZ::Uuid>& componentModeTypes) override;
        void LeftComponentMode(const AZStd::vector<AZ::Uuid>& componentModeTypes) override;

        // EntityOutlinerWidgetInterface
        void SetUpdatesEnabled(bool enable) override;
        void ExpandEntityChildren(AZ::EntityId entityId) override;

        // Build a selection object from the given entities. Entities already in the Widget's selection buffers are ignored.
        template <class EntityIdCollection>
        QItemSelection BuildSelectionFromEntities(const EntityIdCollection& entityIds);

        Ui::EntityOutlinerWidgetUI* m_gui;
        EntityOutlinerListModel* m_listModel;
        EntityOutlinerSortFilterProxyModel* m_proxyModel;
        AZ::u64 m_selectionContextId;
        AZStd::vector<AZ::EntityId> m_selectedEntityIds;

        void PrepareSelection();
        void DoCreateEntity();
        void DoCreateEntityWithParent(const AZ::EntityId& parentId);
        void DoDuplicateSelection();
        void DoDeleteSelection();
        void DoDeleteSelectionAndDescendants();
        void DoRenameSelection();
        void DoMoveEntityUp();
        void DoMoveEntityDown();
        void GoToEntitiesInViewport();

        void SetIndexAsCurrentAndSelected(const QModelIndex& index);

        void SetupActions();

        QAction* m_actionToCreateEntity;
        QAction* m_actionToDeleteSelection;
        QAction* m_actionToDeleteSelectionAndDescendants;
        QAction* m_actionToRenameSelection;
        QAction* m_actionToReparentSelection;
        QAction* m_actionToMoveEntityUp;
        QAction* m_actionToMoveEntityDown;
        QAction* m_actionGoToEntitiesInViewport;

        void OnTreeItemClicked(const QModelIndex& index);
        void OnTreeItemDoubleClicked(const QModelIndex& index);
        void OnTreeItemExpanded(const QModelIndex& index);
        void OnTreeItemCollapsed(const QModelIndex& index);
        void OnExpandEntity(const AZ::EntityId& entityId, bool expand);
        void OnSelectEntity(const AZ::EntityId& entityId, bool selected);
        void OnEnableSelectionUpdates(bool enable);
        void OnDropEvent();
        bool m_inObjectPickMode;

        void InvalidateFilter();
        void ClearFilter();

        AZ::EntityId GetEntityIdFromIndex(const QModelIndex& index) const;
        QModelIndex GetIndexFromEntityId(const AZ::EntityId& entityId) const;
        void ExtractEntityIdsFromSelection(const QItemSelection& selection, EntityIdList& entityIdList) const;

        // OutlinerModelNotificationBus::Handler
        // Receive notification from the outliner model that we should scroll
        // to a given entity
        void QueueScrollToNewContent(const AZ::EntityId& entityId) override;

        void ScrollToNewContent();
        bool m_scrollToNewContentQueued;
        bool m_scrollToSelectedEntity;
        bool m_dropOperationInProgress;
        bool m_expandSelectedEntity;
        bool m_focusInEntityOutliner;
        AZ::EntityId m_scrollToEntityId;

        void QueueUpdateSelection();
        void UpdateSelection();
        EntityIdSet m_entitiesToSelect;
        EntityIdSet m_entitiesToDeselect;
        EntityIdSet m_entitiesSelectedByOutliner;
        EntityIdSet m_entitiesDeselectedByOutliner;
        bool m_selectionChangeQueued;
        bool m_selectionChangeInProgress;
        bool m_enableSelectionUpdates;

        QIcon m_emptyIcon;
        QIcon m_clearIcon;

        void QueueContentUpdateSort(const AZ::EntityId& entityId);
        void SortContent();

        EntityOutliner::DisplayOptionsMenu* m_displayOptionsMenu;
        EntityIdSet m_entitiesToSort;
        EntityOutliner::DisplaySortMode m_sortMode;
        bool m_sortContentQueued;

        EditorEntityUiInterface* m_editorEntityUiInterface = nullptr;
    };

}
