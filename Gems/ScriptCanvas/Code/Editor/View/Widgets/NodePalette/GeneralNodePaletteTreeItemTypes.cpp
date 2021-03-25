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

#include <AzCore/Serialization/SerializeContext.h>

#include <QCoreApplication>

#include <GraphCanvas/Components/SceneBus.h>
#include <GraphCanvas/Components/Nodes/NodeUIBus.h>

#include "GeneralNodePaletteTreeItemTypes.h"

#include "Editor/Components/IconComponent.h"

#include "Editor/Nodes/NodeCreateUtils.h"
#include "Editor/Translation/TranslationHelper.h"

#include "ScriptCanvas/Bus/RequestBus.h"
#include "Editor/Include/ScriptCanvas/GraphCanvas/NodeDescriptorBus.h"
#include "Editor/GraphCanvas/GraphCanvasEditorNotificationBusId.h"

#include <Core/Attributes.h>
#include <Editor/View/Widgets/NodePalette/NodePaletteModel.h>
#include <Libraries/Core/Method.h>
#include <Libraries/Core/MethodOverloaded.h>

namespace ScriptCanvasEditor
{
    ///////////////////////////////
    // CreateClassMethodMimeEvent
    ///////////////////////////////

    void CreateClassMethodMimeEvent::Reflect(AZ::ReflectContext* reflectContext)
    {
        AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(reflectContext);

        if (serializeContext)
        {
            serializeContext->Class<CreateClassMethodMimeEvent, GraphCanvas::GraphCanvasMimeEvent>()
                ->Version(1)
                ->Field("ClassName", &CreateClassMethodMimeEvent::m_className)
                ->Field("MethodName", &CreateClassMethodMimeEvent::m_methodName)
                ->Field("IsOverload", &CreateClassMethodMimeEvent::m_isOverload)
                ;
        }
    }

    CreateClassMethodMimeEvent::CreateClassMethodMimeEvent(const QString& className, const QString& methodName, bool isOverload)
        : m_className(className.toUtf8().data())
        , m_methodName(methodName.toUtf8().data())
        , m_isOverload(isOverload)
    {
    }

    ScriptCanvasEditor::NodeIdPair CreateClassMethodMimeEvent::CreateNode(const ScriptCanvas::ScriptCanvasId& scriptCanvasId) const
    {
        if (m_isOverload)
        {
            return Nodes::CreateObjectMethodOverloadNode(m_className, m_methodName, scriptCanvasId);
        }
        else
        {
            return Nodes::CreateObjectMethodNode(m_className, m_methodName, scriptCanvasId);
        }
    }

    ////////////////////////////////////
    // ClassMethodEventPaletteTreeItem
    ////////////////////////////////////

    ClassMethodEventPaletteTreeItem::ClassMethodEventPaletteTreeItem(AZStd::string_view className, AZStd::string_view methodName, bool isOverload)
        : DraggableNodePaletteTreeItem(methodName, ScriptCanvasEditor::AssetEditorId)
        , m_className(className.data())
        , m_methodName(methodName.data())
        , m_isOverload(isOverload)
    {
        AZStd::string displayMethodName = TranslationHelper::GetKeyTranslation(TranslationContextGroup::ClassMethod, m_className.toUtf8().data(), m_methodName.toUtf8().data(), TranslationItemType::Node, TranslationKeyId::Name);

        if (displayMethodName.empty())
        {
            SetName(m_methodName);
        }
        else
        {
            SetName(displayMethodName.c_str());
        }

        AZStd::string displayEventTooltip = TranslationHelper::GetKeyTranslation(TranslationContextGroup::ClassMethod, m_className.toUtf8().data(), m_methodName.toUtf8().data(), TranslationItemType::Node, TranslationKeyId::Tooltip);

        if (!displayEventTooltip.empty())
        {
            SetToolTip(displayEventTooltip.c_str());
        }

        SetTitlePalette("MethodNodeTitlePalette");
    }

    GraphCanvas::GraphCanvasMimeEvent* ClassMethodEventPaletteTreeItem::CreateMimeEvent() const
    {
        return aznew CreateClassMethodMimeEvent(m_className, m_methodName, m_isOverload);
    }

    AZStd::string ClassMethodEventPaletteTreeItem::GetClassMethodName() const
    {
        return m_className.toUtf8().data();
    }

    AZStd::string ClassMethodEventPaletteTreeItem::GetMethodName() const
    {
        return m_methodName.toUtf8().data();
    }

    bool ClassMethodEventPaletteTreeItem::IsOverload() const
    {
        return m_isOverload;
    }

    //! Implementation of the CreateGlobalMethod Mime Event
    void CreateGlobalMethodMimeEvent::Reflect(AZ::ReflectContext* reflectContext)
    {
        AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(reflectContext);

        if (serializeContext)
        {
            serializeContext->Class<CreateGlobalMethodMimeEvent, GraphCanvas::GraphCanvasMimeEvent>()
                ->Version(0)
                ->Field("MethodName", &CreateGlobalMethodMimeEvent::m_methodName)
                ;
        }
    }

    CreateGlobalMethodMimeEvent::CreateGlobalMethodMimeEvent(AZStd::string methodName)
        : m_methodName{ AZStd::move(methodName) }
    {
    }

    ScriptCanvasEditor::NodeIdPair CreateGlobalMethodMimeEvent::CreateNode(const ScriptCanvas::ScriptCanvasId& scriptCanvasId) const
    {
        return Nodes::CreateGlobalMethodNode(m_methodName, scriptCanvasId);
    }

    //! Global Method Palette Tree Item implementation
    GlobalMethodEventPaletteTreeItem::GlobalMethodEventPaletteTreeItem(const GlobalMethodNodeModelInformation& nodeModelInformation)
        : DraggableNodePaletteTreeItem(nodeModelInformation.m_methodName, ScriptCanvasEditor::AssetEditorId)
        , m_methodName{ nodeModelInformation.m_methodName }
    {
        SetToolTip(QString::fromUtf8(nodeModelInformation.m_displayName.data(),
            aznumeric_cast<int>(nodeModelInformation.m_displayName.size())));

        SetToolTip(QString::fromUtf8(nodeModelInformation.m_toolTip.data(),
            aznumeric_cast<int>(nodeModelInformation.m_toolTip.size())));

        SetTitlePalette("MethodNodeTitlePalette");
    }

    GraphCanvas::GraphCanvasMimeEvent* GlobalMethodEventPaletteTreeItem::CreateMimeEvent() const
    {
        return aznew CreateGlobalMethodMimeEvent(m_methodName);
    }


    const AZStd::string& GlobalMethodEventPaletteTreeItem::GetMethodName() const
    {
        return m_methodName;
    }

    //////////////////////////////
    // CreateCustomNodeMimeEvent
    //////////////////////////////

    void CreateCustomNodeMimeEvent::Reflect(AZ::ReflectContext* reflectContext)
    {
        AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(reflectContext);

        if (serializeContext)
        {
            serializeContext->Class<CreateCustomNodeMimeEvent, GraphCanvas::GraphCanvasMimeEvent>()
                ->Version(2)
                ->Field("TypeId", &CreateCustomNodeMimeEvent::m_typeId)
                ->Field("StyleOverride", &CreateCustomNodeMimeEvent::m_styleOverride)
                ->Field("TitlePalette", &CreateCustomNodeMimeEvent::m_titlePalette)
                ;
        }
    }

    CreateCustomNodeMimeEvent::CreateCustomNodeMimeEvent(const AZ::Uuid& typeId)
        : m_typeId(typeId)
    {
    }

    CreateCustomNodeMimeEvent::CreateCustomNodeMimeEvent(const AZ::Uuid& typeId, const AZStd::string& styleOverride, const AZStd::string& titlePalette)
        : m_typeId(typeId)
        , m_styleOverride(styleOverride)
        , m_titlePalette(titlePalette)
    {
    }

    ScriptCanvasEditor::NodeIdPair CreateCustomNodeMimeEvent::CreateNode(const ScriptCanvas::ScriptCanvasId& scriptCanvasId) const
    {
        Nodes::StyleConfiguration styleConfiguration;
        styleConfiguration.m_nodeSubStyle = m_styleOverride;
        styleConfiguration.m_titlePalette = m_titlePalette;

        return Nodes::CreateNode(m_typeId, scriptCanvasId, styleConfiguration);
    }

    //////////////////////////////
    // CustomNodePaletteTreeItem
    //////////////////////////////

    CustomNodePaletteTreeItem::CustomNodePaletteTreeItem(const AZ::Uuid& typeId, AZStd::string_view nodeName)
        : DraggableNodePaletteTreeItem(nodeName, ScriptCanvasEditor::AssetEditorId)
        , m_typeId(typeId)
    {
    }

    GraphCanvas::GraphCanvasMimeEvent* CustomNodePaletteTreeItem::CreateMimeEvent() const
    {
        return aznew CreateCustomNodeMimeEvent(m_typeId, GetStyleOverride(), GetTitlePalette());
    }

    AZ::Uuid CustomNodePaletteTreeItem::GetTypeId() const
    {
        return m_typeId;
    }
}
