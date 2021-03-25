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


#include <AzToolsFramework/UI/Layer/LayerUiHandler.h>
#include <AzToolsFramework/UI/Outliner/EntityOutlinerListModel.hxx>

#include <QAbstractItemModel>
#include <QPainter>
#include <QPainterPath>
#include <QTreeView>

namespace AzToolsFramework
{
    const int LayerUiHandler::m_layerSquareSize = 20;
    const int LayerUiHandler::m_layerStripeWidth = 1;
    const int LayerUiHandler::m_layerDividerLineHeight = 1;
    const int LayerUiHandler::m_lastEntityInLayerDividerLineHeight = 1;
    const QColor LayerUiHandler::m_layerBackgroundColor = QColor("#2F2F2F");
    const QColor LayerUiHandler::m_layerDescendantBackgroundColor = QColor("#333333");
    const QColor LayerUiHandler::m_layerBorderTopColor = QColor("#515151");
    const QColor LayerUiHandler::m_layerBorderBottomColor = QColor("#252525");
    const QString LayerUiHandler::m_layerIconPath = QString(":/Icons/layer_icon.svg");
    
    QString LayerUiHandler::GenerateItemInfoString(AZ::EntityId entityId) const
    {
        QString result;
        bool isLayerEntity = false;
        AzToolsFramework::Layers::EditorLayerComponentRequestBus::EventResult(
            isLayerEntity,
            entityId,
            &AzToolsFramework::Layers::EditorLayerComponentRequestBus::Events::HasLayer);

        if (!isLayerEntity)
        {
            return result;
        }

        bool hasUnsavedLayerChanges = false;
        AzToolsFramework::Layers::EditorLayerComponentRequestBus::EventResult(
            hasUnsavedLayerChanges,
            entityId,
            &AzToolsFramework::Layers::EditorLayerComponentRequestBus::Events::HasUnsavedChanges);

        if (hasUnsavedLayerChanges)
        {
            result = QObject::tr("*");
        }

        bool isLayerNameValid = false;
        AzToolsFramework::Layers::EditorLayerComponentRequestBus::EventResult(
            isLayerNameValid,
            entityId,
            &AzToolsFramework::Layers::EditorLayerComponentRequestBus::Events::IsLayerNameValid);

        if (!isLayerNameValid)
        {
            result = QObject::tr("<font color=\"red\">(!)</font> %1").arg(result);
        }

        return result;
    }

    QPixmap LayerUiHandler::GenerateItemIcon(AZ::EntityId /*entityId*/) const
    {
        return QPixmap(m_layerIconPath);
    }

    void LayerUiHandler::PaintItemBackground(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& /*index*/) const
    {
        QPainterPath backgroundPath;
        backgroundPath.addRect(option.rect);
        painter->fillPath(backgroundPath, m_layerBackgroundColor);
    }

    void LayerUiHandler::PaintDescendantBackground(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& /*index*/,
        const QModelIndex& /*descendantIndex*/) const
    {
        QPainterPath backgroundPath;
        backgroundPath.addRect(option.rect);
        painter->fillPath(backgroundPath, m_layerDescendantBackgroundColor);
    }

    void LayerUiHandler::PaintItemForeground(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
    {
        AZ::EntityId entityId(index.data(EntityOutlinerListModel::EntityIdRole).value<AZ::u64>());

        if (!painter)
        {
            return;
        }

        QColor layerColor;
        AzToolsFramework::Layers::EditorLayerComponentRequestBus::EventResult(
            layerColor,
            entityId,
            &AzToolsFramework::Layers::EditorLayerComponentRequestBus::Events::GetLayerColor);

        const QTreeView* outlinerTreeView(qobject_cast<const QTreeView*>(option.widget));
        int indentation = outlinerTreeView->indentation();

        bool isFirstColumn = index.column() == EntityOutlinerListModel::ColumnName;
        bool hasVisibleChildren = index.data(EntityOutlinerListModel::ExpandedRole).value<bool>() && index.model()->hasChildren(index);

        if (isFirstColumn)
        {
            // Layer colored box
            QPainterPath layerIconPath;
            QPoint layerBoxOffset(0, (option.rect.height() - m_layerSquareSize) / 2);
            QRect layerIconRect(option.rect.topLeft() + layerBoxOffset, QSize(m_layerSquareSize, m_layerSquareSize));
            layerIconPath.addRect(layerIconRect);
            painter->fillPath(layerIconPath, layerColor);

            // Left border
            PaintLayerStripeAndBorder(
                painter,
                option.rect.left(),
                option.rect.top(),
                option.rect.bottom(),
                m_layerBorderBottomColor,
                layerColor);
        }

        QModelIndex nameColumn = index.sibling(index.row(), EntityOutlinerListModel::Column::ColumnName);
        QModelIndex sibling = index.sibling(index.row() + 1, index.column());

        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, false);

        QPoint lineBottomLeft(option.rect.bottomLeft());
        QPoint lineTopLeft(option.rect.topLeft());

        if (isFirstColumn)
        {
            lineTopLeft.setX(lineTopLeft.x() + m_layerStripeWidth);
        }
        QPen topLinePen(m_layerBorderTopColor, m_layerDividerLineHeight);
        painter->setPen(topLinePen);
        painter->drawLine(lineTopLeft, option.rect.topRight());
        
        if (isFirstColumn)
        {
            lineBottomLeft.setX(lineBottomLeft.x());
            // Leave room for the 2 layer stripes on the first child to connect to the layer box.
            if (hasVisibleChildren)
            {
                lineBottomLeft.setX(lineBottomLeft.x() + m_layerStripeWidth);
            }
        }
        
        QPen bottomLinePen(m_layerBorderBottomColor, m_layerDividerLineHeight);
        painter->setPen(bottomLinePen);
        painter->drawLine(lineBottomLeft, option.rect.bottomRight());
        
        painter->restore();
    }

    void LayerUiHandler::PaintDescendantForeground(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index,
        const QModelIndex& descendantIndex) const
    {
        if (!painter)
        {
            return;
        }

        AZ::EntityId entityId(index.data(EntityOutlinerListModel::EntityIdRole).value<AZ::u64>());
        AZ::EntityId descendantEntityId(descendantIndex.data(EntityOutlinerListModel::EntityIdRole).value<AZ::u64>());

        QColor layerColor;
        AzToolsFramework::Layers::EditorLayerComponentRequestBus::EventResult(layerColor, entityId,
            &AzToolsFramework::Layers::EditorLayerComponentRequestBus::Events::GetLayerColor);

        const QTreeView* outlinerTreeView(qobject_cast<const QTreeView*>(option.widget));
        bool isFirstColumn = descendantIndex.column() == EntityOutlinerListModel::ColumnName;
        bool isLastColumn = descendantIndex.column() == EntityOutlinerListModel::ColumnLockToggle;
        int ancestorLeft = outlinerTreeView->visualRect(index).left();

        // Draw the left stripe for this layer on the descendant's rect
        if (isFirstColumn)
        {
            PaintLayerStripeAndBorder(
                painter,
                ancestorLeft,
                option.rect.top() - m_layerDividerLineHeight,
                option.rect.bottom(),
                m_layerBorderBottomColor,
                layerColor);
        }

        if (isLastColumn)
        {
            // If this is the last visible entity in the layer, draw a line at the bottom
            if(IsLastVisibleChild(index, descendantIndex))
            {
                // Get index for last column to have the border get to the edge on the right
                painter->save();
                painter->setRenderHint(QPainter::Antialiasing, false);

                QPen bottomLinePen(m_layerBorderBottomColor, m_layerDividerLineHeight);

                QPoint lineLeft = option.rect.bottomLeft();
                lineLeft.setX(ancestorLeft);

                painter->setPen(bottomLinePen);
                painter->drawLine(lineLeft, option.rect.bottomRight());

                painter->restore();
            }
        }
    }

    void LayerUiHandler::PaintLayerStripeAndBorder(QPainter* painter, int stripeX, int top, int bottom,
        QColor layerBorderColor, QColor layerColor)
    {
        if (!painter)
        {
            return;
        }

        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, false);

        QPen layerPen;
        layerPen.setWidthF(m_layerStripeWidth);
        layerPen.setColor(layerBorderColor);
        painter->setPen(layerPen);

        QLine layerStripeLine(stripeX, top, stripeX, bottom);
        painter->drawLine(layerStripeLine);

        layerPen.setColor(layerColor);
        // Draw the line a little taller so it connects to the top of the layer
        layerStripeLine.translate(m_layerStripeWidth, 0);
        painter->setPen(layerPen);
        painter->drawLine(layerStripeLine);

        painter->restore();
    }

    bool LayerUiHandler::IsLastVisibleChild(const QModelIndex& parent, const QModelIndex& child)
    {
        QModelIndex lastVisibleItemIndex = GetLastVisibleChild(parent);
        QModelIndex index = child;

        // GetLastVisibleChild returns an index set to the ColumnName column
        if (index.column() != EntityOutlinerListModel::ColumnName)
        {
            index = index.siblingAtColumn(EntityOutlinerListModel::ColumnName);
        }

        return index == lastVisibleItemIndex;
    }

    QModelIndex LayerUiHandler::GetLastVisibleChild(const QModelIndex& parent)
    {
        auto model = parent.model();
        QModelIndex index = parent;

        // The parenting information for the index are stored in the ColumnName column
        if (index.column() != EntityOutlinerListModel::ColumnName)
        {
            index = index.siblingAtColumn(EntityOutlinerListModel::ColumnName);
        }

        return Internal_GetLastVisibleChild(model, index);
    }

    QModelIndex LayerUiHandler::Internal_GetLastVisibleChild(const QAbstractItemModel* model, const QModelIndex& index)
    {
        if (!model->hasChildren(index) || !index.data(EntityOutlinerListModel::ExpandedRole).value<bool>())
        {
            return index;
        }

        int childCount = index.data(EntityOutlinerListModel::ChildCountRole).value<int>();
        QModelIndex lastChild = model->index(childCount - 1, EntityOutlinerListModel::ColumnName, index);

        return Internal_GetLastVisibleChild(model, lastChild);
    }
}
