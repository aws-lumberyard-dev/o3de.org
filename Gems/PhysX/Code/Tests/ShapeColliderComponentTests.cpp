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

#include "PhysX_precompiled.h"
#include <AzCore/UnitTest/TestTypes.h>
#include <AzToolsFramework/UnitTest/AzToolsFrameworkTestHelpers.h>
#include <AzToolsFramework/ToolsComponents/EditorNonUniformScaleComponent.h>
#include <Tests/EditorTestUtilities.h>
#include <EditorColliderComponent.h>
#include <EditorShapeColliderComponent.h>
#include <EditorRigidBodyComponent.h>
#include <EditorForceRegionComponent.h>
#include <ShapeColliderComponent.h>
#include <RigidBodyComponent.h>
#include <StaticRigidBodyComponent.h>
#include <RigidBodyStatic.h>
#include <LmbrCentral/Shape/BoxShapeComponentBus.h>
#include <LmbrCentral/Shape/CylinderShapeComponentBus.h>
#include <LmbrCentral/Shape/PolygonPrismShapeComponentBus.h>
#include <LmbrCentral/Shape/SphereShapeComponentBus.h>
#include <LmbrCentral/Shape/CompoundShapeComponentBus.h>
#include <PhysX/ForceRegionComponentBus.h>
#include <PhysX/PhysXLocks.h>
#include <PhysX/SystemComponentBus.h>
#include <Tests/PhysXTestCommon.h>

namespace PhysXEditorTests
{
    namespace
    {
        struct TestData
        {
            const AZStd::vector<AZ::Vector2> polygonHShape = {
                AZ::Vector2(0.0f, 0.0f),
                AZ::Vector2(0.0f, 3.0f),
                AZ::Vector2(1.0f, 3.0f),
                AZ::Vector2(1.0f, 2.0f),
                AZ::Vector2(2.0f, 2.0f),
                AZ::Vector2(2.0f, 3.0f),
                AZ::Vector2(3.0f, 3.0f),
                AZ::Vector2(3.0f, 0.0f),
                AZ::Vector2(2.0f, 0.0f),
                AZ::Vector2(2.0f, 1.0f),
                AZ::Vector2(1.0f, 1.0f),
                AZ::Vector2(1.0f, 0.0f)
            };
        };
    }

    TEST_F(PhysXEditorFixture, EditorShapeColliderComponent_ShapeDependencySatisfied_EntityIsValid)
    {
        EntityPtr entity = CreateInactiveEditorEntity("ShapeColliderComponentEditorEntity");
        entity->CreateComponent<PhysX::EditorShapeColliderComponent>();
        entity->CreateComponent(LmbrCentral::EditorBoxShapeComponentTypeId);

        // the entity should be in a valid state because the shape component requirement is satisfied
        AZ::Entity::DependencySortOutcome sortOutcome = entity->EvaluateDependenciesGetDetails();
        EXPECT_TRUE(sortOutcome.IsSuccess());
    }

    TEST_F(PhysXEditorFixture, EditorShapeColliderComponent_ShapeDependencyMissing_EntityIsInvalid)
    {
        EntityPtr entity = CreateInactiveEditorEntity("ShapeColliderComponentEditorEntity");
        entity->CreateComponent<PhysX::EditorShapeColliderComponent>();

        // the entity should not be in a valid state because the shape collider component requires a shape component
        AZ::Entity::DependencySortOutcome sortOutcome = entity->EvaluateDependenciesGetDetails();
        EXPECT_FALSE(sortOutcome.IsSuccess());
        EXPECT_TRUE(sortOutcome.GetError().m_code == AZ::Entity::DependencySortResult::MissingRequiredService);
    }

    TEST_F(PhysXEditorFixture, EditorShapeColliderComponent_MultipleShapeColliderComponents_EntityIsInvalid)
    {
        EntityPtr entity = CreateInactiveEditorEntity("ShapeColliderComponentEditorEntity");
        entity->CreateComponent<PhysX::EditorShapeColliderComponent>();
        entity->CreateComponent(LmbrCentral::EditorBoxShapeComponentTypeId);

        // adding a second shape collider component should make the entity invalid
        entity->CreateComponent<PhysX::EditorShapeColliderComponent>();

        // the entity should be in a valid state because the shape component requirement is satisfied
        AZ::Entity::DependencySortOutcome sortOutcome = entity->EvaluateDependenciesGetDetails();
        EXPECT_FALSE(sortOutcome.IsSuccess());
        EXPECT_TRUE(sortOutcome.GetError().m_code == AZ::Entity::DependencySortResult::HasIncompatibleServices);
    }

    TEST_F(PhysXEditorFixture, EditorShapeColliderComponent_ShapeColliderPlusMultipleColliderComponents_EntityIsValid)
    {
        EntityPtr entity = CreateInactiveEditorEntity("ShapeColliderComponentEditorEntity");
        entity->CreateComponent<PhysX::EditorShapeColliderComponent>();
        entity->CreateComponent(LmbrCentral::EditorBoxShapeComponentTypeId);

        // the shape collider component should be compatible with multiple collider components
        entity->CreateComponent<PhysX::EditorColliderComponent>();
        entity->CreateComponent<PhysX::EditorColliderComponent>();

        // the entity should be in a valid state because the shape component requirement is satisfied
        AZ::Entity::DependencySortOutcome sortOutcome = entity->EvaluateDependenciesGetDetails();
        EXPECT_TRUE(sortOutcome.IsSuccess());
    }

    TEST_F(PhysXEditorFixture, EditorShapeColliderComponent_ShapeColliderWithBox_CorrectRuntimeComponents)
    {
        // create an editor entity with a shape collider component and a box shape component
        EntityPtr editorEntity = CreateInactiveEditorEntity("ShapeColliderComponentEditorEntity");
        editorEntity->CreateComponent<PhysX::EditorShapeColliderComponent>();
        editorEntity->CreateComponent(LmbrCentral::EditorBoxShapeComponentTypeId);
        editorEntity->Activate();

        EntityPtr gameEntity = CreateActiveGameEntityFromEditorEntity(editorEntity.get());

        // check that the runtime entity has the expected components
        EXPECT_TRUE(gameEntity->FindComponent<PhysX::ShapeColliderComponent>() != nullptr);
        EXPECT_TRUE(gameEntity->FindComponent(LmbrCentral::BoxShapeComponentTypeId) != nullptr);
    }

    TEST_F(PhysXEditorFixture, EditorShapeColliderComponent_ShapeColliderWithBox_CorrectRuntimeGeometry)
    {
        // create an editor entity with a shape collider component and a box shape component
        EntityPtr editorEntity = CreateInactiveEditorEntity("ShapeColliderComponentEditorEntity");
        editorEntity->CreateComponent<PhysX::EditorShapeColliderComponent>();
        editorEntity->CreateComponent(LmbrCentral::EditorBoxShapeComponentTypeId);
        editorEntity->Activate();

        const AZ::Vector3 boxDimensions(2.0f, 3.0f, 4.0f);
        LmbrCentral::BoxShapeComponentRequestsBus::Event(editorEntity->GetId(),
            &LmbrCentral::BoxShapeComponentRequests::SetBoxDimensions, boxDimensions);

        EntityPtr gameEntity = CreateActiveGameEntityFromEditorEntity(editorEntity.get());

        // since there was no editor rigid body component, the runtime entity should have a static rigid body
        const auto* staticBody = azdynamic_cast<PhysX::StaticRigidBody*>(gameEntity->FindComponent<PhysX::StaticRigidBodyComponent>()->GetSimulatedBody());
        const auto* pxRigidStatic = static_cast<const physx::PxRigidStatic*>(staticBody->GetNativePointer());

        PHYSX_SCENE_READ_LOCK(pxRigidStatic->getScene());

        // there should be a single shape on the rigid body and it should be a box
        EXPECT_EQ(pxRigidStatic->getNbShapes(), 1);
        physx::PxShape* shape = nullptr;
        pxRigidStatic->getShapes(&shape, 1, 0);
        EXPECT_EQ(shape->getGeometryType(), physx::PxGeometryType::eBOX);

        // the bounding box of the rigid body should reflect the dimensions of the box set above
        AZ::Aabb aabb = staticBody->GetAabb();
        EXPECT_TRUE(aabb.GetMax().IsClose(0.5f * boxDimensions));
        EXPECT_TRUE(aabb.GetMin().IsClose(-0.5f * boxDimensions));
    }

    void SetPolygonPrismVertices(AZ::EntityId entityId, const AZStd::vector<AZ::Vector2>& vertices)
    {
        AZ::PolygonPrismPtr polygonPrism;
        LmbrCentral::PolygonPrismShapeComponentRequestBus::EventResult(polygonPrism, entityId,
            &LmbrCentral::PolygonPrismShapeComponentRequests::GetPolygonPrism);
        if (polygonPrism)
        {
            polygonPrism->m_vertexContainer.SetVertices(vertices);
        }
    }

    void SetPolygonPrismHeight(AZ::EntityId entityId, float height)
    {
        AZ::PolygonPrismPtr polygonPrism;
        LmbrCentral::PolygonPrismShapeComponentRequestBus::EventResult(polygonPrism, entityId,
            &LmbrCentral::PolygonPrismShapeComponentRequests::GetPolygonPrism);
        if (polygonPrism)
        {
            polygonPrism->SetHeight(height);
        }
    }

    TEST_F(PhysXEditorFixture, EditorShapeColliderComponent_ShapeColliderWithPolygonPrism_CorrectRuntimeGeometry)
    {
        // create an editor entity with a shape collider component and a polygon prism shape component
        EntityPtr editorEntity = CreateInactiveEditorEntity("ShapeColliderComponentEditorEntity");
        editorEntity->CreateComponent<PhysX::EditorShapeColliderComponent>();
        editorEntity->CreateComponent(LmbrCentral::EditorPolygonPrismShapeComponentTypeId);

        // suppress the shape collider error that will be raised because the polygon prism vertices have not been set yet
        UnitTest::ErrorHandler polygonPrismErrorHandler("Invalid polygon prism");
        editorEntity->Activate();

        // modify the geometry of the polygon prism
        TestData testData;
        SetPolygonPrismVertices(editorEntity->GetId(), testData.polygonHShape);
        SetPolygonPrismHeight(editorEntity->GetId(), 2.0f);

        EntityPtr gameEntity = CreateActiveGameEntityFromEditorEntity(editorEntity.get());

        // since there was no editor rigid body component, the runtime entity should have a static rigid body
        const auto* staticBody = azdynamic_cast<PhysX::StaticRigidBody*>(gameEntity->FindComponent<PhysX::StaticRigidBodyComponent>()->GetSimulatedBody());
        const auto* pxRigidStatic = static_cast<const physx::PxRigidStatic*>(staticBody->GetNativePointer());

        PHYSX_SCENE_READ_LOCK(pxRigidStatic->getScene());

        // the input polygon prism was not convex, so should have been decomposed into multiple shapes
        const int numShapes = pxRigidStatic->getNbShapes();
        EXPECT_TRUE(numShapes > 1);

        // the shapes should all be convex meshes
        for (int shapeIndex = 0; shapeIndex < numShapes; shapeIndex++)
        {
            physx::PxShape* shape = nullptr;
            pxRigidStatic->getShapes(&shape, 1, shapeIndex);
            EXPECT_EQ(shape->getGeometryType(), physx::PxGeometryType::eCONVEXMESH);
        }

        // the vertices of the input polygon prism ranged from (0, 0) to (3, 3) and the height was set to 2
        // the bounding box of the static rigid body should reflect those values
        AZ::Aabb aabb = staticBody->GetAabb();
        EXPECT_TRUE(aabb.GetMax().IsClose(AZ::Vector3(3.0f, 3.0f, 2.0f)));
        EXPECT_TRUE(aabb.GetMin().IsClose(AZ::Vector3::CreateZero()));
    }

    TEST_F(PhysXEditorFixture, EditorShapeColliderComponent_ShapeColliderWithPolygonPrismAndNonUniformScale_CorrectRunAabb)
    {
        // create an editor entity with a shape collider component and a polygon prism shape component
        EntityPtr editorEntity = CreateInactiveEditorEntity("ShapeColliderComponentEditorEntity");
        editorEntity->CreateComponent<PhysX::EditorShapeColliderComponent>();
        editorEntity->CreateComponent(LmbrCentral::EditorPolygonPrismShapeComponentTypeId);

        // add a non-uniform scale component
        editorEntity->CreateComponent<AzToolsFramework::Components::EditorNonUniformScaleComponent>();

        // suppress the shape collider error that will be raised because the polygon prism vertices have not been set yet
        UnitTest::ErrorHandler polygonPrismErrorHandler("Invalid polygon prism");
        editorEntity->Activate();

        // modify the geometry of the polygon prism
        TestData testData;
        AZ::EntityId entityId = editorEntity->GetId();
        SetPolygonPrismVertices(entityId, testData.polygonHShape);
        SetPolygonPrismHeight(entityId, 2.0f);

        // update the transform scale and non-uniform scale
        AZ::TransformBus::Event(entityId, &AZ::TransformBus::Events::SetLocalUniformScale, 2.0f);
        AZ::NonUniformScaleRequestBus::Event(entityId, &AZ::NonUniformScaleRequests::SetScale, AZ::Vector3(0.5f, 1.5f, 2.0f));

        EntityPtr gameEntity = CreateActiveGameEntityFromEditorEntity(editorEntity.get());

        // since there was no editor rigid body component, the runtime entity should have a static rigid body
        const auto* staticBody = azdynamic_cast<PhysX::StaticRigidBody*>(gameEntity->FindComponent<PhysX::StaticRigidBodyComponent>()->GetSimulatedBody());

        // the vertices of the input polygon prism ranged from (0, 0) to (3, 3) and the height was set to 2
        // the bounding box of the static rigid body should reflect those values combined with the scale values above
        AZ::Aabb aabb = staticBody->GetAabb();
        EXPECT_TRUE(aabb.GetMax().IsClose(AZ::Vector3(3.0f, 9.0f, 8.0f)));
        EXPECT_TRUE(aabb.GetMin().IsClose(AZ::Vector3::CreateZero()));
    }

    TEST_F(PhysXEditorFixture, EditorShapeColliderComponent_ShapeColliderWithCylinder_CorrectRuntimeComponents)
    {
        // create an editor entity with a shape collider component and a cylinder shape component
        EntityPtr editorEntity = CreateInactiveEditorEntity("ShapeColliderComponentEditorEntity");
        editorEntity->CreateComponent<PhysX::EditorShapeColliderComponent>();
        editorEntity->CreateComponent(LmbrCentral::EditorCylinderShapeComponentTypeId);
        editorEntity->Activate();

        EntityPtr gameEntity = CreateActiveGameEntityFromEditorEntity(editorEntity.get());

        // check that the runtime entity has the expected components
        EXPECT_TRUE(gameEntity->FindComponent<PhysX::ShapeColliderComponent>() != nullptr);
        EXPECT_TRUE(gameEntity->FindComponent(LmbrCentral::CylinderShapeComponentTypeId) != nullptr);
    }

    TEST_F(PhysXEditorFixture, EditorShapeColliderComponent_ShapeColliderWithCylinderWithValidRadiusAndValidHeight_CorrectRuntimeGeometry)
    {
        // create an editor entity with a shape collider component and a cylinder shape component
        EntityPtr editorEntity = CreateInactiveEditorEntity("ShapeColliderComponentEditorEntity");
        editorEntity->CreateComponent<PhysX::EditorShapeColliderComponent>();
        editorEntity->CreateComponent(LmbrCentral::EditorCylinderShapeComponentTypeId);
        editorEntity->Activate();
        
        const float validRadius = 1.0f;
        const float validHeight = 1.0f;
        
        LmbrCentral::CylinderShapeComponentRequestsBus::Event(editorEntity->GetId(),
            &LmbrCentral::CylinderShapeComponentRequests::SetRadius, validRadius);
        
        LmbrCentral::CylinderShapeComponentRequestsBus::Event(editorEntity->GetId(),
            &LmbrCentral::CylinderShapeComponentRequests::SetHeight, validHeight);
        
        EntityPtr gameEntity = CreateActiveGameEntityFromEditorEntity(editorEntity.get());
        
        // since there was no editor rigid body component, the runtime entity should have a static rigid body
        const auto* staticBody = azdynamic_cast<PhysX::StaticRigidBody*>(gameEntity->FindComponent<PhysX::StaticRigidBodyComponent>()->GetSimulatedBody());
        const auto* pxRigidStatic = static_cast<const physx::PxRigidStatic*>(staticBody->GetNativePointer());

        PHYSX_SCENE_READ_LOCK(pxRigidStatic->getScene());
        
        // there should be a single shape on the rigid body and it should be a convex mesh
        EXPECT_EQ(pxRigidStatic->getNbShapes(), 1);
        physx::PxShape* shape = nullptr;
        pxRigidStatic->getShapes(&shape, 1, 0);
        EXPECT_EQ(shape->getGeometryType(), physx::PxGeometryType::eCONVEXMESH);
        
        // the bounding box of the rigid body should reflect the dimensions of the cylinder set above
        AZ::Aabb aabb = staticBody->GetAabb();
        const float validDiameter = validRadius * 2.0f;
        
        // Check that the z positions of the bounding box match that of the cylinder
        EXPECT_NEAR(aabb.GetMin().GetZ(), -0.5f * validHeight, AZ::Constants::Tolerance);
        EXPECT_NEAR(aabb.GetMax().GetZ(), 0.5f * validHeight, AZ::Constants::Tolerance);
        
        // check that the xy points are not outside the radius of the cylinder
        AZ::Vector2 vecMin(aabb.GetMin().GetX(), aabb.GetMin().GetY());
        AZ::Vector2 vecMax(aabb.GetMax().GetX(), aabb.GetMax().GetY());
        EXPECT_TRUE(AZ::GetAbs(vecMin.GetX()) <= validRadius);
        EXPECT_TRUE(AZ::GetAbs(vecMin.GetY()) <= validRadius);
        EXPECT_TRUE(AZ::GetAbs(vecMax.GetX()) <= validRadius);
        EXPECT_TRUE(AZ::GetAbs(vecMax.GetX()) <= validRadius);        
    }

    TEST_F(PhysXEditorFixture, EditorShapeColliderComponent_ShapeColliderWithCylinderWithNullRadius_HandledGracefully)
    {
        ValidateInvalidEditorShapeColliderComponentParams(0.f, 1.f);
    }
    
    TEST_F(PhysXEditorFixture, EditorShapeColliderComponent_ShapeColliderWithCylinderWithNullHeight_HandledGracefully)
    {
        ValidateInvalidEditorShapeColliderComponentParams(0.f, 1.f);
    }
    
    TEST_F(PhysXEditorFixture, EditorShapeColliderComponent_ShapeColliderWithCylinderWithNullRadiusAndNullHeight_HandledGracefully)
    {
        ValidateInvalidEditorShapeColliderComponentParams(0.f, 0.f);
    }
    
    TEST_F(PhysXEditorFixture, EditorShapeColliderComponent_ShapeColliderWithCylinderWithNegativeRadiusAndNullHeight_HandledGracefully)
    {
        ValidateInvalidEditorShapeColliderComponentParams(-1.f, 0.f);
    }
    
    TEST_F(PhysXEditorFixture, EditorShapeColliderComponent_ShapeColliderWithCylinderWithNullRadiusAndNegativeHeight_HandledGracefully)
    {
        ValidateInvalidEditorShapeColliderComponentParams(0.f, -1.f);
    }

    TEST_F(PhysXEditorFixture, EditorShapeColliderComponent_ShapeColliderWithUnsupportedShape_HandledGracefully)
    {
        UnitTest::ErrorHandler unsupportedShapeWarningHandler("Unsupported shape");

        // create an editor entity with a shape collider component and a cylinder shape component
        // the cylinder shape is not currently supported by the shape collider component
        EntityPtr editorEntity = CreateInactiveEditorEntity("ShapeColliderComponentEditorEntity");
        editorEntity->CreateComponent<PhysX::EditorShapeColliderComponent>();
        editorEntity->CreateComponent(LmbrCentral::EditorCompoundShapeComponentTypeId);
        editorEntity->Activate();

        // expect 2 warnings
                //1 raised for the unsupported shape
                //2 when re-creating the underlying simulated body
        EXPECT_EQ(unsupportedShapeWarningHandler.GetWarningCount(), 2);

        EntityPtr gameEntity = CreateActiveGameEntityFromEditorEntity(editorEntity.get());

        // since there was no editor rigid body component, the runtime entity should have a static rigid body
        const auto* staticBody = azdynamic_cast<PhysX::StaticRigidBody*>(gameEntity->FindComponent<PhysX::StaticRigidBodyComponent>()->GetSimulatedBody());
        const auto* pxRigidStatic = static_cast<const physx::PxRigidStatic*>(staticBody->GetNativePointer());

        PHYSX_SCENE_READ_LOCK(pxRigidStatic->getScene());

        // there should be no shapes on the rigid body because the cylinder is not supported
        EXPECT_EQ(pxRigidStatic->getNbShapes(), 0);
    }

    TEST_F(PhysXEditorFixture, EditorShapeColliderComponent_ShapeColliderWithBoxAndRigidBody_CorrectRuntimeComponents)
    {
        // create an editor entity with a shape collider component and a box shape component
        EntityPtr editorEntity = CreateInactiveEditorEntity("ShapeColliderComponentEditorEntity");
        editorEntity->CreateComponent<PhysX::EditorShapeColliderComponent>();
        editorEntity->CreateComponent<PhysX::EditorRigidBodyComponent>();
        editorEntity->CreateComponent(LmbrCentral::EditorBoxShapeComponentTypeId);
        editorEntity->Activate();

        EntityPtr gameEntity = CreateActiveGameEntityFromEditorEntity(editorEntity.get());

        // check that the runtime entity has the expected components
        EXPECT_TRUE(gameEntity->FindComponent<PhysX::ShapeColliderComponent>() != nullptr);
        EXPECT_TRUE(gameEntity->FindComponent(LmbrCentral::BoxShapeComponentTypeId) != nullptr);
        EXPECT_TRUE(gameEntity->FindComponent<PhysX::RigidBodyComponent>() != nullptr);
    }

    TEST_F(PhysXEditorFixture, EditorShapeColliderComponent_ShapeColliderWithBoxAndRigidBody_CorrectRuntimeEntity)
    {
        // create an editor entity with a shape collider component and a box shape component
        EntityPtr editorEntity = CreateInactiveEditorEntity("ShapeColliderComponentEditorEntity");
        editorEntity->CreateComponent<PhysX::EditorShapeColliderComponent>();
        editorEntity->CreateComponent<PhysX::EditorRigidBodyComponent>();
        editorEntity->CreateComponent(LmbrCentral::EditorBoxShapeComponentTypeId);
        editorEntity->Activate();

        const AZ::Vector3 boxDimensions(2.0f, 3.0f, 4.0f);
        LmbrCentral::BoxShapeComponentRequestsBus::Event(editorEntity->GetId(),
            &LmbrCentral::BoxShapeComponentRequests::SetBoxDimensions, boxDimensions);

        EntityPtr gameEntity = CreateActiveGameEntityFromEditorEntity(editorEntity.get());

        // since there was an editor rigid body component, the runtime entity should have a dynamic rigid body
        const auto* rigidBody = gameEntity->FindComponent<PhysX::RigidBodyComponent>()->GetRigidBody();
        const auto* pxRigidDynamic = static_cast<const physx::PxRigidDynamic*>(rigidBody->GetNativePointer());

        PHYSX_SCENE_READ_LOCK(pxRigidDynamic->getScene());

        // there should be a single shape on the rigid body and it should be a box
        EXPECT_EQ(pxRigidDynamic->getNbShapes(), 1);
        physx::PxShape* shape = nullptr;
        pxRigidDynamic->getShapes(&shape, 1, 0);
        EXPECT_EQ(shape->getGeometryType(), physx::PxGeometryType::eBOX);

        // the bounding box of the rigid body should reflect the dimensions of the box set above
        AZ::Aabb aabb = rigidBody->GetAabb();
        EXPECT_TRUE(aabb.GetMax().IsClose(0.5f * boxDimensions));
        EXPECT_TRUE(aabb.GetMin().IsClose(-0.5f * boxDimensions));
    }

    TEST_F(PhysXEditorFixture, EditorShapeColliderComponent_TransformChanged_ColliderUpdated)
    {
        // create an editor entity with a shape collider component and a box shape component
        EntityPtr editorEntity = CreateInactiveEditorEntity("ShapeColliderComponentEditorEntity");
        editorEntity->CreateComponent<PhysX::EditorShapeColliderComponent>();
        editorEntity->CreateComponent(LmbrCentral::EditorBoxShapeComponentTypeId);
        editorEntity->Activate();
        AZ::EntityId editorEntityId = editorEntity->GetId();
        AZ::Vector3 boxDimensions = AZ::Vector3::CreateOne();
        LmbrCentral::BoxShapeComponentRequestsBus::EventResult(boxDimensions, editorEntityId,
            &LmbrCentral::BoxShapeComponentRequests::GetBoxDimensions);

        // update the transform
        const float scale = 2.0f;
        AZ::TransformBus::Event(editorEntityId, &AZ::TransformInterface::SetLocalUniformScale, scale);
        const AZ::Vector3 translation(10.0f, 20.0f, 30.0f);
        AZ::TransformBus::Event(editorEntityId, &AZ::TransformInterface::SetWorldTranslation, translation);

        // make a game entity and check its bounding box is consistent with the changed transform
        EntityPtr gameEntity = CreateActiveGameEntityFromEditorEntity(editorEntity.get());
        const auto* staticBody = azdynamic_cast<PhysX::StaticRigidBody*>(gameEntity->FindComponent<PhysX::StaticRigidBodyComponent>()->GetSimulatedBody());
        AZ::Aabb aabb = staticBody->GetAabb();
        EXPECT_TRUE(aabb.GetMax().IsClose(translation + 0.5f * scale * boxDimensions));
        EXPECT_TRUE(aabb.GetMin().IsClose(translation - 0.5f * scale * boxDimensions));
    }

    void SetTrigger(PhysX::EditorShapeColliderComponent* editorShapeColliderComponent, bool isTrigger)
    {
        AZ::SerializeContext* serializeContext = nullptr;
        AZ::ComponentApplicationBus::BroadcastResult(serializeContext, &AZ::ComponentApplicationRequests::GetSerializeContext);
        AzToolsFramework::InstanceDataHierarchy instanceDataHierarchy;
        instanceDataHierarchy.AddRootInstance(editorShapeColliderComponent);
        instanceDataHierarchy.Build(serializeContext, AZ::SerializeContext::ENUM_ACCESS_FOR_WRITE);
        AzToolsFramework::InstanceDataHierarchy::InstanceDataNode* instanceNode =
            instanceDataHierarchy.FindNodeByPartialAddress({ AZ_CRC("Trigger", 0x1a6b0f5d) });
        if (instanceNode)
        {
            instanceNode->Write<bool>(isTrigger);
        }
    }

    EntityPtr CreateRigidBox(const AZ::Vector3& boxDimensions, const AZ::Vector3& position)
    {
        EntityPtr rigidBodyEditorEntity = CreateInactiveEditorEntity("RigidBodyEditorEntity");
        rigidBodyEditorEntity->CreateComponent<PhysX::EditorShapeColliderComponent>();
        rigidBodyEditorEntity->CreateComponent(LmbrCentral::EditorBoxShapeComponentTypeId);
        rigidBodyEditorEntity->CreateComponent<PhysX::EditorRigidBodyComponent>();
        rigidBodyEditorEntity->Activate();
        LmbrCentral::BoxShapeComponentRequestsBus::Event(rigidBodyEditorEntity->GetId(),
            &LmbrCentral::BoxShapeComponentRequests::SetBoxDimensions, boxDimensions);
        AZ::TransformBus::Event(rigidBodyEditorEntity->GetId(), &AZ::TransformInterface::SetWorldTranslation, position);
        AzToolsFramework::ToolsApplicationRequestBus::Broadcast(
            &AzToolsFramework::ToolsApplicationRequests::RemoveDirtyEntity, rigidBodyEditorEntity->GetId());

        return CreateActiveGameEntityFromEditorEntity(rigidBodyEditorEntity.get());
    }

    // LYN-1241 - Test disabled due to AZ_Error reports about MaterialLibrary being not found in the AssetCatalog
    TEST_F(PhysXEditorFixture, DISABLED_EditorShapeColliderComponent_PolygonPrismForceRegion_AppliesForceAtRuntime)
    {
        // create an editor entity with shape collider, polygon prism shape and force region components
        EntityPtr forceRegionEditorEntity = CreateInactiveEditorEntity("ShapeColliderComponentEditorEntity");
        auto* shapeColliderComponent = forceRegionEditorEntity->CreateComponent<PhysX::EditorShapeColliderComponent>();
        SetTrigger(shapeColliderComponent, true);
        forceRegionEditorEntity->CreateComponent(LmbrCentral::EditorPolygonPrismShapeComponentTypeId);
        forceRegionEditorEntity->CreateComponent<PhysX::EditorForceRegionComponent>();

        // suppress the shape collider error that will be raised because the polygon prism vertices have not been set yet
        UnitTest::ErrorHandler polygonPrismErrorHandler("Invalid polygon prism");
        forceRegionEditorEntity->Activate();

        // modify the geometry of the polygon prism
        TestData testData;
        SetPolygonPrismVertices(forceRegionEditorEntity->GetId(), testData.polygonHShape);
        SetPolygonPrismHeight(forceRegionEditorEntity->GetId(), 2.0f);

        EntityPtr forceRegionGameEntity = CreateActiveGameEntityFromEditorEntity(forceRegionEditorEntity.get());

        // add a force to the force region
        PhysX::ForceRegionRequestBus::Event(forceRegionGameEntity->GetId(),
            &PhysX::ForceRegionRequests::AddForceWorldSpace, AZ::Vector3::CreateAxisX(), 100.0f);

        const AZ::Vector3 boxDimensions(0.5f, 0.5f, 0.5f);
        // create one box over the centre of the polygon prism
        const AZ::Vector3 box1Position(1.5f, 1.5f, 3.0f);
        // create another box over one of the notches in the H
        const AZ::Vector3 box2Position(1.5f, 0.5f, 3.0f);
        EntityPtr rigidBodyGameEntity1 = CreateRigidBox(boxDimensions, box1Position);
        EntityPtr rigidBodyGameEntity2 = CreateRigidBox(boxDimensions, box2Position);

        PhysX::TestUtils::UpdateScene(m_defaultScene, AzPhysics::SystemConfiguration::DefaultFixedTimestep, 100);

        // the first rigid body should have been moved in the positive x direction by the force region
        EXPECT_TRUE(rigidBodyGameEntity1->GetTransform()->GetWorldTranslation().GetX() > box1Position.GetX() + AZ::Constants::FloatEpsilon);
        // the second rigid body should not have entered the force region and so its X position should not have been affected
        EXPECT_NEAR(rigidBodyGameEntity2->GetTransform()->GetWorldTranslation().GetX(), box2Position.GetX(), 1e-3f);
    }

    TEST_F(PhysXEditorFixture, EditorShapeColliderComponent_ShapeColliderWithScaleSetToParentEntity_CorrectRuntimeScale)
    {
        // create an editor parent entity (empty, need transform component only)
        EntityPtr editorParentEntity = CreateInactiveEditorEntity("ParentEntity");
        editorParentEntity->Activate();

        // set some scale to parent entity
        const float parentScale = 2.0f;
        AZ::TransformBus::Event(editorParentEntity->GetId(), &AZ::TransformInterface::SetLocalUniformScale, parentScale);

        // create an editor child entity with a shape collider component and a box shape component
        EntityPtr editorChildEntity = CreateInactiveEditorEntity("ChildEntity");
        editorChildEntity->CreateComponent<PhysX::EditorShapeColliderComponent>();
        editorChildEntity->CreateComponent<PhysX::EditorRigidBodyComponent>();
        editorChildEntity->CreateComponent(LmbrCentral::EditorBoxShapeComponentTypeId);
        editorChildEntity->Activate();

        // set some dimensions to child entity box component
        const AZ::Vector3 boxDimensions(2.0f, 3.0f, 4.0f);
        LmbrCentral::BoxShapeComponentRequestsBus::Event(editorChildEntity->GetId(),
            &LmbrCentral::BoxShapeComponentRequests::SetBoxDimensions,
            boxDimensions);

        // set one entity as parent of another
        AZ::TransformBus::Event(editorChildEntity->GetId(),
            &AZ::TransformBus::Events::SetParentRelative,
            editorParentEntity->GetId());

        // build child game entity (parent will be built implicitly)
        EntityPtr gameChildEntity = CreateActiveGameEntityFromEditorEntity(editorChildEntity.get());

        // since there was an editor rigid body component, the runtime entity should have a dynamic rigid body
        const AzPhysics::RigidBody* rigidBody =
            gameChildEntity->FindComponent<PhysX::RigidBodyComponent>()->GetRigidBody();
        
        // the bounding box of the rigid body should reflect the dimensions of the box set above
        // and parent entity scale
        const AZ::Aabb aabb = rigidBody->GetAabb();
        EXPECT_THAT(aabb.GetMax(), UnitTest::IsClose(0.5f * boxDimensions * parentScale));
        EXPECT_THAT(aabb.GetMin(), UnitTest::IsClose(-0.5f * boxDimensions * parentScale));
    }

} // namespace PhysXEditorTests
