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

#include "Math.h"

#include <ScriptCanvas/Internal/Nodes/ExpressionNodeBase.h>

#pragma warning (disable:4503) // decorated name length exceeded, name was truncated

namespace ScriptCanvas
{
    namespace Library
    {
        void Math::Reflect(AZ::ReflectContext* reflection)
        {
            if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(reflection))
            {
                serializeContext->Class<Math, LibraryDefinition>()
                    ->Version(1)
                    ;

                if (AZ::EditContext* editContext = serializeContext->GetEditContext())
                {
                    editContext->Class<Math>("Math", "")->
                        ClassElement(AZ::Edit::ClassElements::EditorData, "")->
                        Attribute(AZ::Edit::Attributes::Icon, "Editor/Icons/ScriptCanvas/Libraries/Math.png")->
                        Attribute(AZ::Edit::Attributes::CategoryStyle, ".math")->
                        Attribute(ScriptCanvas::Attributes::Node::TitlePaletteOverride, "MathNodeTitlePalette")
                        ;
                }
            }

            if (AZ::BehaviorContext* behaviorContext = azrtti_cast<AZ::BehaviorContext*>(reflection))
            {
                using namespace ScriptCanvas::Nodes::Math;
                SCRIPT_CANVAS_GENERICS_TO_VM(AABBNodes::Registrar, AABB, behaviorContext, AABBNodes::k_categoryName);
                SCRIPT_CANVAS_GENERICS_TO_VM(CRCNodes::Registrar, CRC, behaviorContext, CRCNodes::k_categoryName);
                SCRIPT_CANVAS_GENERICS_TO_VM(ColorNodes::Registrar, Color, behaviorContext, ColorNodes::k_categoryName);
                SCRIPT_CANVAS_GENERICS_TO_VM(MathNodes::Registrar, Math, behaviorContext, MathNodes::k_categoryName);
                SCRIPT_CANVAS_GENERICS_TO_VM(Matrix3x3Nodes::Registrar, Matrix3x3, behaviorContext, Matrix3x3Nodes::k_categoryName);
                SCRIPT_CANVAS_GENERICS_TO_VM(Matrix4x4Nodes::Registrar, Matrix4x4, behaviorContext, Matrix4x4Nodes::k_categoryName);
                SCRIPT_CANVAS_GENERICS_TO_VM(OBBNodes::Registrar, OBB, behaviorContext, OBBNodes::k_categoryName);
                SCRIPT_CANVAS_GENERICS_TO_VM(PlaneNodes::Registrar, Plane, behaviorContext, PlaneNodes::k_categoryName);
                SCRIPT_CANVAS_GENERICS_TO_VM(QuaternionNodes::Registrar, Quaternion, behaviorContext, QuaternionNodes::k_categoryName);
                SCRIPT_CANVAS_GENERICS_TO_VM(RandomNodes::Registrar, Random, behaviorContext, RandomNodes::k_categoryName);
                SCRIPT_CANVAS_GENERICS_TO_VM(TransformNodes::Registrar, Transform, behaviorContext, TransformNodes::k_categoryName);
                SCRIPT_CANVAS_GENERICS_TO_VM(Vector2Nodes::Registrar, Vector2, behaviorContext, Vector2Nodes::k_categoryName);
                SCRIPT_CANVAS_GENERICS_TO_VM(Vector3Nodes::Registrar, Vector3, behaviorContext, Vector3Nodes::k_categoryName);
                SCRIPT_CANVAS_GENERICS_TO_VM(Vector4Nodes::Registrar, Vector4, behaviorContext, Vector4Nodes::k_categoryName);
            }

            Nodes::Internal::ExpressionNodeBase::Reflect(reflection);
        }

        void Math::InitNodeRegistry(NodeRegistry& nodeRegistry)
        {
            using namespace ScriptCanvas::Nodes::Math;
            AddNodeToRegistry<Math, AABB>(nodeRegistry);
            AddNodeToRegistry<Math, CRC>(nodeRegistry);
            AddNodeToRegistry<Math, Color>(nodeRegistry);
            AddNodeToRegistry<Math, Divide>(nodeRegistry);
            AddNodeToRegistry<Math, Matrix3x3>(nodeRegistry);
            AddNodeToRegistry<Math, Matrix4x4>(nodeRegistry);
            AddNodeToRegistry<Math, MathExpression>(nodeRegistry);
            AddNodeToRegistry<Math, Multiply>(nodeRegistry);
            AddNodeToRegistry<Math, Number>(nodeRegistry);
            AddNodeToRegistry<Math, OBB>(nodeRegistry);
            AddNodeToRegistry<Math, Plane>(nodeRegistry);           
            AddNodeToRegistry<Math, Random>(nodeRegistry);
            AddNodeToRegistry<Math, Quaternion>(nodeRegistry);
            AddNodeToRegistry<Math, Subtract>(nodeRegistry);
            AddNodeToRegistry<Math, Sum>(nodeRegistry);
            AddNodeToRegistry<Math, Transform>(nodeRegistry);
            AddNodeToRegistry<Math, Vector2>(nodeRegistry);
            AddNodeToRegistry<Math, Vector3>(nodeRegistry);
            AddNodeToRegistry<Math, Vector4>(nodeRegistry);

            AABBNodes::Registrar::AddToRegistry<Math>(nodeRegistry);
            ColorNodes::Registrar::AddToRegistry<Math>(nodeRegistry);
            CRCNodes::Registrar::AddToRegistry<Math>(nodeRegistry);
            MathNodes::Registrar::AddToRegistry<Math>(nodeRegistry);
            Matrix3x3Nodes::Registrar::AddToRegistry<Math>(nodeRegistry);
            Matrix4x4Nodes::Registrar::AddToRegistry<Math>(nodeRegistry);
            OBBNodes::Registrar::AddToRegistry<Math>(nodeRegistry);
            PlaneNodes::Registrar::AddToRegistry<Math>(nodeRegistry);
            QuaternionNodes::Registrar::AddToRegistry<Math>(nodeRegistry);
            RandomNodes::Registrar::AddToRegistry<Math>(nodeRegistry);
            TransformNodes::Registrar::AddToRegistry<Math>(nodeRegistry);
            Vector2Nodes::Registrar::AddToRegistry<Math>(nodeRegistry);
            Vector3Nodes::Registrar::AddToRegistry<Math>(nodeRegistry);
            Vector4Nodes::Registrar::AddToRegistry<Math>(nodeRegistry);
        }
        
        AZStd::vector<AZ::ComponentDescriptor*> Math::GetComponentDescriptors()
        {
            AZStd::vector<AZ::ComponentDescriptor*> descriptors = {
                ScriptCanvas::Nodes::Math::AABB::CreateDescriptor(),
                ScriptCanvas::Nodes::Math::CRC::CreateDescriptor(),
                ScriptCanvas::Nodes::Math::Color::CreateDescriptor(),
                ScriptCanvas::Nodes::Math::Divide::CreateDescriptor(),
                ScriptCanvas::Nodes::Math::Matrix3x3::CreateDescriptor(),
                ScriptCanvas::Nodes::Math::Matrix4x4::CreateDescriptor(),
                ScriptCanvas::Nodes::Math::MathExpression::CreateDescriptor(),
                ScriptCanvas::Nodes::Math::Multiply::CreateDescriptor(),
                ScriptCanvas::Nodes::Math::Number::CreateDescriptor(),
                ScriptCanvas::Nodes::Math::OBB::CreateDescriptor(),
                ScriptCanvas::Nodes::Math::Plane::CreateDescriptor(),
                ScriptCanvas::Nodes::Math::Random::CreateDescriptor(),
                ScriptCanvas::Nodes::Math::Quaternion::CreateDescriptor(),
                ScriptCanvas::Nodes::Math::Subtract::CreateDescriptor(),
                ScriptCanvas::Nodes::Math::Sum::CreateDescriptor(),
                ScriptCanvas::Nodes::Math::Transform::CreateDescriptor(),
                ScriptCanvas::Nodes::Math::Vector2::CreateDescriptor(),
                ScriptCanvas::Nodes::Math::Vector3::CreateDescriptor(),
                ScriptCanvas::Nodes::Math::Vector4::CreateDescriptor(),
            };

            AABBNodes::Registrar::AddDescriptors(descriptors);
            ColorNodes::Registrar::AddDescriptors(descriptors);
            CRCNodes::Registrar::AddDescriptors(descriptors);
            MathNodes::Registrar::AddDescriptors(descriptors);
            Matrix3x3Nodes::Registrar::AddDescriptors(descriptors);
            Matrix4x4Nodes::Registrar::AddDescriptors(descriptors);
            OBBNodes::Registrar::AddDescriptors(descriptors);
            PlaneNodes::Registrar::AddDescriptors(descriptors);
            QuaternionNodes::Registrar::AddDescriptors(descriptors);
            RandomNodes::Registrar::AddDescriptors(descriptors);
            TransformNodes::Registrar::AddDescriptors(descriptors);
            Vector2Nodes::Registrar::AddDescriptors(descriptors);
            Vector3Nodes::Registrar::AddDescriptors(descriptors);
            Vector4Nodes::Registrar::AddDescriptors(descriptors);
            return descriptors;
        }
    }
}
