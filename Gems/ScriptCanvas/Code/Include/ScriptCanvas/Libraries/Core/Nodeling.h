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

#include <ScriptCanvas/Core/Graph.h>
#include <ScriptCanvas/Core/Node.h>
#include <ScriptCanvas/Core/NodelingBus.h>

#include <Include/ScriptCanvas/Libraries/Core/Nodeling.generated.h>

namespace ScriptCanvas
{
    namespace Nodes
    {
        namespace Core
        {
            namespace Internal
            {
                //! Base class for a special purpose node that can be used for many kinds of applications
                //! such as execution and data definitions for functions, routing nodes, custom utilities, etc.
                class Nodeling
                    : public Node
                    , public NodePropertyInterfaceListener
                    , public NodelingRequestBus::Handler
                {
                private:
                    enum NodeVersion
                    {
                        Initial = 1,
                    };

                public:

                    SCRIPTCANVAS_NODE(Nodeling);

                    Nodeling();
                    ~Nodeling() = default;

                    AZStd::string GetSlotDisplayGroup() const { return "NodelingSlotDisplayGroup"; }
                    AZ::Crc32 GetSlotDisplayGroupId() const { return AZ_CRC("NodelingSlotDisplayGroup", 0xedf94173); }

                    AZ::Crc32 GetPropertyId() const { return AZ_CRC("NodeNameProperty", 0xe967a10a); }

                    const AZ::Uuid& GetIdentifier() const { return m_identifier; }

                    // Node
                    AZStd::string GetNodeName() const override { return GetDisplayName(); }
                    void OnInit() override;
                    void OnGraphSet() override;

                    void ConfigureVisualExtensions() override;

                    NodePropertyInterface* GetPropertyInterface(AZ::Crc32 propertyId) override;
                    ////

                    // NodelingRequestBus
                    AZ::EntityId GetNodeId() const override { return GetEntityId(); }
                    GraphScopedNodeId GetGraphScopedNodeId() const override { return GetScopedNodeId(); }
                    const AZStd::string& GetDisplayName() const override { return m_displayName; }

                    bool IsSupportedByNewBackend() const override { return true; }

                    void SetDisplayName(const AZStd::string& displayName) override;
                    ////

                    void RemapId();

                protected:

                    AZStd::string m_previousName;

                    Data::StringType m_displayName;
                    AZ::Uuid m_identifier;

                    virtual void OnDisplayNameChanged();

                private:
                    // NodePropertyInterface
                    void OnPropertyChanged() override;
                    ////

                    TypedNodePropertyInterface<ScriptCanvas::Data::StringType> m_displayNameInterface;
                };
            }
        }
    }
}
