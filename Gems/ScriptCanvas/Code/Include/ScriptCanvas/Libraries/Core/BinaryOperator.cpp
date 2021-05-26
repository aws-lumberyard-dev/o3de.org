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

#include "BinaryOperator.h"

namespace ScriptCanvas
{
    namespace Nodes
    {
        const char* BinaryOperator::k_evaluateName = "In";
        const char* BinaryOperator::k_outName = "Out";
        const char* BinaryOperator::k_onTrue = "True";
        const char* BinaryOperator::k_onFalse = "False";

        const char* BinaryOperator::k_lhsName = "Value A";
        const char* BinaryOperator::k_rhsName = "Value B";
        const char* BinaryOperator::k_resultName = "Result";

        void ArithmeticExpression::CustomizeReplacementNode(Node* replacementNode, AZStd::unordered_map<SlotId, AZStd::vector<SlotId>>& outSlotIdMap) const
        {
            auto slotIds = replacementNode->GetSlotIds("Value");
            if (slotIds.size() >= 2)
            {
                outSlotIdMap.emplace(this->GetSlotId(k_lhsName), AZStd::vector<SlotId>{ slotIds[0] });
                outSlotIdMap.emplace(this->GetSlotId(k_rhsName), AZStd::vector<SlotId>{ slotIds[1] });
            }
        }

        void ArithmeticExpression::OnInit()
        {
            {
                DataSlotConfiguration slotConfiguration;

                slotConfiguration.m_name = k_resultName;
                slotConfiguration.SetConnectionType(ConnectionType::Output);                
                slotConfiguration.SetType(Data::Type::Number());

                AddSlot(slotConfiguration);
            }

            BinaryOperator::OnInit();

            {
                ExecutionSlotConfiguration slotConfiguration;

                slotConfiguration.m_name = k_outName;
                slotConfiguration.m_toolTip = "Signaled after the arithmetic operation is done.";
                slotConfiguration.SetConnectionType(ConnectionType::Output);                

                AddSlot(slotConfiguration);
            }

            {
                DataSlotConfiguration slotConfiguration;

                slotConfiguration.m_name = k_lhsName;
                slotConfiguration.SetConnectionType(ConnectionType::Input);                
                slotConfiguration.SetType(Data::Type::Number());

                AddSlot(slotConfiguration);
            }

            {
                DataSlotConfiguration slotConfiguration;

                slotConfiguration.m_name = k_rhsName;
                slotConfiguration.SetConnectionType(ConnectionType::Input);                
                slotConfiguration.SetType(Data::Type::Number());

                AddSlot(slotConfiguration);
            }
        }

        void ArithmeticExpression::OnInputSignal(const SlotId& slotId)
        {
            if (slotId == GetSlotId(k_evaluateName))
            {
                const Datum output = Evaluate(*FindDatumByIndex(k_datumIndexLHS), *FindDatumByIndex(k_datumIndexRHS));
                if (auto slot = GetSlot(GetOutputSlotId()))
                {
                    PushOutput(output, *slot);
                }

                SignalOutput(GetSlotId(k_outName));
            }
        }

        void ArithmeticExpression::Reflect(AZ::ReflectContext* reflection)
        {
            if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(reflection))
            {
                serializeContext->Class<ArithmeticExpression, BinaryOperator>()
                    ->Version(0)
                    ->Attribute(AZ::Script::Attributes::Deprecated, true)
                    ;

                if (AZ::EditContext* editContext = serializeContext->GetEditContext())
                {
                    editContext->Class<ArithmeticExpression>("ArithmeticExpression", "ArithmeticExpression")
                        ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ;
                }
            }
        }

        Datum BinaryOperator::Evaluate([[maybe_unused]] const Datum& lhs, [[maybe_unused]] const Datum& rhs)
        {
            AZ_Assert(false, "Evaluate must be overridden");
            return Datum();
        };

        void BinaryOperator::OnInputSignal([[maybe_unused]] const SlotId& slot)
        {
            AZ_Assert(false, "OnInputSignal must be overridden");
        }

        void BinaryOperator::OnInit()
        {
            {
                ExecutionSlotConfiguration slotConfiguration;

                slotConfiguration.m_name = k_evaluateName;
                slotConfiguration.m_toolTip = "Signal to perform the evaluation when desired.";
                slotConfiguration.SetConnectionType(ConnectionType::Input);                

                AddSlot(slotConfiguration);
            }
        }

        SlotId BinaryOperator::GetOutputSlotId() const
        {
            return GetSlotId(k_resultName);
        }

        void BinaryOperator::Reflect(AZ::ReflectContext* reflection)
        {
            if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(reflection))
            {
                serializeContext->Class<BinaryOperator, Node>()
                    ->Version(0)
                    ->Attribute(AZ::Script::Attributes::Deprecated, true)
                    ;

                if (AZ::EditContext* editContext = serializeContext->GetEditContext())
                {
                    editContext->Class<BinaryOperator>("BinaryOperator", "BinaryOperator")
                        ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ;
                }
            }
        }

        void BooleanExpression::InitializeBooleanExpression()
        {
            AZ_Assert(false, "InitializeBooleanExpression must be overridden");
        }

        void BooleanExpression::OnInputSignal(const SlotId& slotId)
        {
            if (slotId == GetSlotId(k_evaluateName))
            {
                const Datum output = Evaluate(*FindDatumByIndex(k_datumIndexLHS), *FindDatumByIndex(k_datumIndexRHS));
                if (auto slot = GetSlot(GetOutputSlotId()))
                {
                    PushOutput(output, *slot);
                }

                const bool* result = output.GetAs<bool>();
                if (result && *result)
                {
                    SignalOutput(GetSlotId(k_onTrue));
                }
                else
                {
                    SignalOutput(GetSlotId(k_onFalse));
                }
            }
        }

        void BooleanExpression::OnInit()
        {            
            {
                DataSlotConfiguration slotConfiguration;
                
                slotConfiguration.m_name = k_resultName;
                slotConfiguration.SetConnectionType(ConnectionType::Output);                
                slotConfiguration.SetType(Data::Type::Boolean());

                AddSlot(slotConfiguration);
            }

            BinaryOperator::OnInit();

            {
                ExecutionSlotConfiguration slotConfiguration;

                slotConfiguration.m_name = k_onTrue;
                slotConfiguration.m_toolTip = "Signaled if the result of the operation is true.";
                slotConfiguration.SetConnectionType(ConnectionType::Output);                

                AddSlot(slotConfiguration);
            }

            {
                ExecutionSlotConfiguration slotConfiguration;

                slotConfiguration.m_name = k_onFalse;
                slotConfiguration.m_toolTip = "Signaled if the result of the operation is false.";
                slotConfiguration.SetConnectionType(ConnectionType::Output);

                AddSlot(slotConfiguration);
            }

            InitializeBooleanExpression();

        }

        void BooleanExpression::Reflect(AZ::ReflectContext* reflection)
        {
            if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(reflection))
            {
                serializeContext->Class<BooleanExpression, BinaryOperator>()
                    ->Attribute(AZ::Script::Attributes::Deprecated, true)
                    ->Version(0)
                    ;

                if (AZ::EditContext* editContext = serializeContext->GetEditContext())
                {
                    editContext->Class<BooleanExpression>("BooleanExpression", "BooleanExpression")
                        ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ;
                }
            }
        }
        
        void ComparisonExpression::InitializeBooleanExpression()
        {
            EqualityExpression::InitializeBooleanExpression();

            {
                DataSlotConfiguration slotConfiguration;

                slotConfiguration.m_name = k_resultName;
                slotConfiguration.SetConnectionType(ConnectionType::Output);                
                slotConfiguration.SetType(Data::Type::Boolean());

                AddSlot(slotConfiguration);
            }
        }

        static bool ComparisonExpressionVersionConverter(AZ::SerializeContext& serializeContext, AZ::SerializeContext::DataElementNode& rootElement)
        {
            if (rootElement.GetVersion() == 0)
            {
                int nodeElementIndex = rootElement.FindElement(AZ_CRC("BaseClass1", 0xd4925735));
                if (nodeElementIndex == -1)
                {
                    AZ_Error("Script Canvas", false, "Unable to find base class node element for ComparisonExpression class %s", false, rootElement.GetNameString());
                    return false;
                }

                // There was change to the ComparisonExpression Base Class type from BooleanExpression to ComparisonExpression.
                // Because the version was not incremented when this change took place version 0 instances of ComparisonExpression class will fail to load when the base class
                // is BinaryExpression
                // Explicitly making a copy of the baseNodeElement notice there is no "&" after AZ::SerializeContext::DataElementNode
                AZ::SerializeContext::DataElementNode baseNodeElement = rootElement.GetSubElement(nodeElementIndex);
                if (baseNodeElement.GetId() == azrtti_typeid<BooleanExpression>())
                {
                    rootElement.RemoveElement(nodeElementIndex);
                    int equalityExpressionElementIndex = rootElement.AddElement(serializeContext, "BaseClass1", azrtti_typeid<EqualityExpression>());
                    if (equalityExpressionElementIndex == -1)
                    {
                        AZ_Error("Script Canvas", false, "Unable to convert BooleanExpression data element to ComparisonExpression data element");
                        return false;
                    }

                    auto& equalityExpressionElement = rootElement.GetSubElement(equalityExpressionElementIndex);
                    if (equalityExpressionElement.AddElement(baseNodeElement) == -1)
                    {
                        AZ_Error("Script Canvas", false, "Unable to add boolean expression data element node as a base class to the equality expression data element node");
                        return false;
                    }
                }
            }
            return true;
        }

        void ComparisonExpression::Reflect(AZ::ReflectContext* reflection)
        {
            if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(reflection))
            {
                serializeContext->Class<ComparisonExpression, EqualityExpression>()
                    ->Version(1, &ComparisonExpressionVersionConverter)
                    ;

                if (AZ::EditContext* editContext = serializeContext->GetEditContext())
                {
                    editContext->Class<ComparisonExpression>("ComparisonExpression", "ComparisonExpression")
                        ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ;
                }
            }
        }
        
        void EqualityExpression::InitializeBooleanExpression()
        {
            {
                DynamicDataSlotConfiguration dataSlotConfiguration;

                dataSlotConfiguration.m_name = k_lhsName;                

                dataSlotConfiguration.SetConnectionType(ConnectionType::Input);
                dataSlotConfiguration.m_dynamicDataType = DynamicDataType::Any;
                dataSlotConfiguration.m_dynamicGroup = AZ::Crc32("ExpressionGroup");

                m_firstSlotId = AddSlot(dataSlotConfiguration);
            }

            {
                DynamicDataSlotConfiguration dataSlotConfiguration;

                dataSlotConfiguration.m_name = k_rhsName;

                dataSlotConfiguration.SetConnectionType(ConnectionType::Input);
                dataSlotConfiguration.m_dynamicDataType = DynamicDataType::Any;
                dataSlotConfiguration.m_dynamicGroup = AZ::Crc32("ExpressionGroup");

                m_secondSlotId = AddSlot(dataSlotConfiguration);
            }

            EndpointNotificationBus::MultiHandler::BusConnect({ GetEntityId(), m_firstSlotId });
            EndpointNotificationBus::MultiHandler::BusConnect({ GetEntityId(), m_secondSlotId });

            // DYNAMIC_SLOT_VERSION_CONVERTER
            ///////// Version Conversion to Dynamic Grouped based operators
            Slot* firstSlot = GetSlot(m_firstSlotId);

            bool addedGroup = false;

            if (firstSlot)
            {
                if (!firstSlot->IsDynamicSlot())
                {
                    firstSlot->SetDynamicDataType(DynamicDataType::Any);
                }

                if (firstSlot->GetDynamicGroup() == AZ::Crc32())
                {
                    SetDynamicGroup(firstSlot->GetId(), AZ::Crc32("ExpressionGroup"));
                    addedGroup = true;
                }
            }

            Slot* secondSlot = GetSlot(m_secondSlotId);

            if (secondSlot)
            {
                if (!secondSlot->IsDynamicSlot())
                {
                    secondSlot->SetDynamicDataType(DynamicDataType::Any);
                }

                if (secondSlot->GetDynamicGroup() == AZ::Crc32())
                {
                    SetDynamicGroup(secondSlot->GetId(), AZ_CRC("ExpressionGroup", 0xd34c978c));                    
                    addedGroup = true;
                }
            }

            if (addedGroup && m_displayType.IsValid())
            {                
                SetDisplayType(AZ_CRC("ExpressionGroup", 0xd34c978c), m_displayType);
            }
            ////
            ////
        }        

        void EqualityExpression::Reflect(AZ::ReflectContext* reflection)
        {
            if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(reflection))
            {
                serializeContext->Class<EqualityExpression, BooleanExpression>()
                    ->Version(0)
                    ->Attribute("DisplayType", &EqualityExpression::m_displayType)
                    ->Attribute(AZ::Script::Attributes::Deprecated, true)
                    ;

                if (AZ::EditContext* editContext = serializeContext->GetEditContext())
                {
                    editContext->Class<EqualityExpression>("EqualityExpression", "EqualityExpression")
                        ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ;
                }
            }
        }
    }
}
