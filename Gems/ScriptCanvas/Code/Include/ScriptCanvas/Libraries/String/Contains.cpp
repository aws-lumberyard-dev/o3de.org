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

#include "Contains.h"

#include <AzFramework/StringFunc/StringFunc.h>
#include <ScriptCanvas/Utils/VersionConverters.h>

namespace ScriptCanvas
{
    namespace Nodes
    {
        namespace String
        {
            void Contains::OnInit()
            {
                // Version Conversion Code for a slot renaming.
                Slot* slot = GetSlotByName("Ignore Case");

                if (slot)
                {
                    RenameSlot((*slot), "Case Sensitive");

                    ModifiableDatumView datumView;
                    ModifyUnderlyingSlotDatum(slot->GetId(), datumView);

                    if (datumView.IsValid() && datumView.IsType(ScriptCanvas::Data::Type::Boolean()))
                    {
                        const bool* datumValue = datumView.GetAs<ScriptCanvas::Data::BooleanType>();

                        Datum newDatum = Datum(!(*datumValue));
                        datumView.AssignToDatum(newDatum);
                    }
                }
                ////
            }

            void Contains::CustomizeReplacementNode(Node* replacementNode, AZStd::unordered_map<SlotId, AZStd::vector<SlotId>>& outSlotIdMap) const
            {
                auto newDataOutSlots = replacementNode->GetSlotsByType(ScriptCanvas::CombinedSlotType::DataOut);
                auto oldDataOutSlots = this->GetSlotsByType(ScriptCanvas::CombinedSlotType::DataOut);
                if (newDataOutSlots.size() == oldDataOutSlots.size())
                {
                    for (size_t index = 0; index < newDataOutSlots.size(); index++)
                    {
                        outSlotIdMap.emplace(oldDataOutSlots[index]->GetId(), AZStd::vector<SlotId>{ newDataOutSlots[index]->GetId() });
                    }
                }
            }

            void Contains::OnInputSignal(const SlotId&)
            {
                AZ_PROFILE_SCOPE(AZ::Debug::ProfileCategory::ScriptCanvas, "ScriptCanvas::Contains::OnInputSignal");

                AZStd::string sourceString = ContainsProperty::GetSource(this);
                AZStd::string patternString = ContainsProperty::GetPattern(this);

                bool caseSensitive = ContainsProperty::GetCaseSensitive(this);
                bool searchFromEnd = ContainsProperty::GetSearchFromEnd(this);

                auto index = AzFramework::StringFunc::Find(sourceString.c_str(), patternString.c_str(), 0, searchFromEnd, caseSensitive);
                if (index != AZStd::string::npos)
                {
                    const SlotId outputIndexSlotId = ContainsProperty::GetIndexSlotId(this);
                    if (auto* indexSlot = GetSlot(outputIndexSlotId))
                    {
                        Datum outputIndex(index);
                        PushOutput(outputIndex, *indexSlot);
                    }

                    SignalOutput(GetSlotId("True"));
                    return;
                }

                SignalOutput(GetSlotId("False"));
            }
        }
    }
}
