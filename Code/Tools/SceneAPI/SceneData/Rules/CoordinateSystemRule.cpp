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

#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/std/smart_ptr/make_shared.h>
#include <AzCore/RTTI/ReflectContext.h>
#include <AzCore/Memory/SystemAllocator.h>
#include <SceneAPI/SceneCore/Containers/RuleContainer.h>
#include <SceneAPI/SceneCore/Utilities/Reporting.h>
#include <SceneAPI/SceneData/Rules/CoordinateSystemRule.h>

namespace AZ::SceneAPI::SceneData
{
    AZ_CLASS_ALLOCATOR_IMPL(CoordinateSystemRule, AZ::SystemAllocator, 0)

    CoordinateSystemRule::CoordinateSystemRule()
        : m_targetCoordinateSystem(ZUpPositiveYForward)
    {
    }

    void CoordinateSystemRule::UpdateCoordinateSystemConverter()
    {
        switch (m_targetCoordinateSystem)
        {
            case ZUpPositiveYForward:
            {
                // Source coordinate system, use identity for now, which will currently just assume LY's coordinate system.
                const AZ::Vector3 sourceBasisVectors[3] = { AZ::Vector3( 1.0f, 0.0f, 0.0f), 
                                                            AZ::Vector3( 0.0f, 1.0f, 0.0f), 
                                                            AZ::Vector3( 0.0f, 0.0f, 1.0f) };

                // The target coordinate system, with X and Y inverted (rotate 180 degrees over Z)
                const AZ::Vector3 targetBasisVectors[3] = { AZ::Vector3(-1.0f, 0.0f, 0.0f), 
                                                            AZ::Vector3( 0.0f,-1.0f, 0.0f), 
                                                            AZ::Vector3( 0.0f, 0.0f, 1.0f) };

                // X, Y and Z are all at the same indices inside the target coordinate system, compared to the source coordinate system.
                const AZ::u32 targetBasisIndices[3] = { 0, 1, 2 };

                m_coordinateSystemConverter = CoordinateSystemConverter::CreateFromBasisVectors(sourceBasisVectors, targetBasisVectors, targetBasisIndices);                                             
            }
            break;

            case ZUpNegativeYForward:
            {
                // Source coordinate system, use identity for now, which will currently just assume LY's coordinate system.
                const AZ::Vector3 sourceBasisVectors[3] = { AZ::Vector3( 1.0f, 0.0f, 0.0f), 
                                                            AZ::Vector3( 0.0f, 1.0f, 0.0f), 
                                                            AZ::Vector3( 0.0f, 0.0f, 1.0f) };

                // The target coordinate system, which is the same as the source, so basically we won't do anything here.
                const AZ::Vector3 targetBasisVectors[3] = { AZ::Vector3( 1.0f, 0.0f, 0.0f), 
                                                            AZ::Vector3( 0.0f, 1.0f, 0.0f), 
                                                            AZ::Vector3( 0.0f, 0.0f, 1.0f) };

                // X, Y and Z are all at the same indices inside the target coordinate system, compared to the source coordinate system.
                const AZ::u32 targetBasisIndices[3] = { 0, 1, 2 };

                m_coordinateSystemConverter = CoordinateSystemConverter::CreateFromBasisVectors(sourceBasisVectors, targetBasisVectors, targetBasisIndices);                                             
            }
            break;

            default:
                AZ_Assert(false, "Unsupported coordinate system conversion");
        };
    }

    void CoordinateSystemRule::SetTargetCoordinateSystem(CoordinateSystem targetCoordinateSystem)
    {
        m_targetCoordinateSystem = targetCoordinateSystem;
        UpdateCoordinateSystemConverter();
    }

    CoordinateSystemRule::CoordinateSystem CoordinateSystemRule::GetTargetCoordinateSystem() const
    {
        return m_targetCoordinateSystem;
    }

    void CoordinateSystemRule::Reflect(AZ::ReflectContext* context)
    {
        AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context);
        if (!serializeContext)
        {
            return;
        }

        serializeContext->Class<CoordinateSystemRule, IRule>()->Version(1)
            ->Field("targetCoordinateSystem", &CoordinateSystemRule::m_targetCoordinateSystem);

        AZ::EditContext* editContext = serializeContext->GetEditContext();
        if (editContext)
        {
            editContext->Class<CoordinateSystemRule>("Coordinate system change", "Modify the target coordinate system, applying a transformation to all data (transforms and vertex data if it exists).")
                ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->Attribute(AZ::Edit::Attributes::NameLabelOverride, "")
                ->DataElement(AZ::Edit::UIHandlers::ComboBox, &CoordinateSystemRule::m_targetCoordinateSystem, "Facing direction", "Change the direction the actor/motion will face by applying a post transformation to the data.")
                    ->EnumAttribute(CoordinateSystem::ZUpNegativeYForward, "Do nothing")
                    ->EnumAttribute(CoordinateSystem::ZUpPositiveYForward, "Rotate 180 degrees around the up axis");
        }
    }

    bool CoordinateSystemRule::ConvertLegacyCoordinateSystemRule(AZ::SerializeContext& serializeContext,
        AZ::SerializeContext::DataElementNode& classElement)
    {
        AZ::SerializeContext::DataElementNode* ruleContainerNode = classElement.FindSubElement(AZ_CRC("rules", 0x899a993c));
        if (!ruleContainerNode)
        {
            AZ_TracePrintf(AZ::SceneAPI::Utilities::ErrorWindow, "Can't find rule container.\n");
            return false;
        }

        AZ::SerializeContext::DataElementNode* rulesNode = ruleContainerNode->FindSubElement(AZ_CRC("rules", 0x899a993c));
        if (!rulesNode)
        {
            AZ_TracePrintf(AZ::SceneAPI::Utilities::ErrorWindow, "Can't find rules within rule container.\n");
            return false;
        }

        const AZ::Uuid oldCoordSysRuleId("{603207E2-4F55-4C33-9AAB-98CA75C1E351}");
        const int numRules = rulesNode->GetNumSubElements();
        for (int i = 0; i < numRules; ++i)
        {
            AZ::SerializeContext::DataElementNode& sharedPointerNode = rulesNode->GetSubElement(i);
            if (sharedPointerNode.GetNumSubElements() == 1)
            {
                AZ::SerializeContext::DataElementNode& currentRuleNode = sharedPointerNode.GetSubElement(0);

                // Convert the coordinate system rule
                if (currentRuleNode.GetId() == oldCoordSysRuleId)
                {
                    int targetCoordinateSystem = 0;
                    currentRuleNode.FindSubElementAndGetData(AZ_CRC("targetCoordinateSystem"), targetCoordinateSystem);

                    AZStd::shared_ptr<CoordinateSystemRule> coordSysRule = AZStd::make_shared<CoordinateSystemRule>();
                    coordSysRule->SetTargetCoordinateSystem(static_cast<AZ::SceneAPI::DataTypes::ICoordinateSystemRule::CoordinateSystem>(targetCoordinateSystem));

                    rulesNode->RemoveElement(i);
                    rulesNode->AddElementWithData<AZStd::shared_ptr<AZ::SceneAPI::DataTypes::IRule>>(serializeContext, "element", coordSysRule);

                    return true;
                }
            }
        }

        return true;
    }
} // namespace AZ::SceneAPI::SceneData
