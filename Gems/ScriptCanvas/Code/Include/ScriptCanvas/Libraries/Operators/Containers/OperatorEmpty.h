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

#include <ScriptCanvas/Core/Node.h>

#include <Include/ScriptCanvas/Libraries/Operators/Containers/OperatorEmpty.generated.h>

namespace ScriptCanvas
{
    namespace Nodes
    {
        namespace Operators
        {
            //! Deprecated: see MethodOverloaded for "Is Empty"
            class OperatorEmpty : public Node
            {
            public:

                SCRIPTCANVAS_NODE(OperatorEmpty);


                void CustomizeReplacementNode(Node* replacementNode, AZStd::unordered_map<SlotId, AZStd::vector<SlotId>>& outSlotIdMap) const override;

                static bool OperatorEmptyVersionConverter(AZ::SerializeContext& context, AZ::SerializeContext::DataElementNode& classElement);

                OperatorEmpty() = default;

                // Node...
                void OnInit() override;
                ////

                void OnInputSignal(const SlotId& slotId) override;

            };
        }
    }
}
