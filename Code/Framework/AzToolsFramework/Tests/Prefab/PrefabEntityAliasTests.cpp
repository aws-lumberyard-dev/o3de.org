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

#include <Prefab/PrefabTestDomUtils.h>
#include <Prefab/PrefabTestComponent.h>
#include <Prefab/PrefabTestFixture.h>

namespace UnitTest
{
    using PrefabEntityAliasTests = PrefabTestFixture;

    TEST_F(PrefabEntityAliasTests, PrefabEntityAlias_ReferenceEntityWithinSameInstance_ReferencePersists)
    {
        // Make a new entity with a test component
        AZ::Entity* newEntity = CreateEntity("New Entity", false);
        PrefabTestComponent* newComponent = newEntity->CreateComponent<PrefabTestComponent>();
        ASSERT_TRUE(newComponent);

        // Generate a second entity that will be referenced by the first
        AZ::Entity* referencedEntity = CreateEntity("Referenced Entity");
        newComponent->m_entityIdProperty = referencedEntity->GetId();

        // Place both entities in the same prefab
        AZStd::unique_ptr<Instance> newInstance =
            m_prefabSystemComponent->CreatePrefab({ newEntity, referencedEntity }, {}, "test/path");

        ASSERT_TRUE(newInstance);

        // Grab the alias of both entities so they can be found in a new instantiation
        EntityAliasOptionalReference newEntityAliasRef = newInstance->GetEntityAlias(newEntity->GetId());
        ASSERT_TRUE(newEntityAliasRef);

        EntityAliasOptionalReference referencedEntityAliasRef = newInstance->GetEntityAlias(referencedEntity->GetId());
        ASSERT_TRUE(referencedEntityAliasRef);

        EntityAlias newEntityAlias = newEntityAliasRef.value();
        EntityAlias referencedEntityAlias = referencedEntityAliasRef.value();

        // A new instance should maintain the entity reference while also having unique entity ids
        AZStd::unique_ptr<Instance> secondInstance =
            m_prefabSystemComponent->InstantiatePrefab(newInstance->GetTemplateId());

        // Activate the entities so that we can find them via the component application bus
        secondInstance->InitializeEntities();
        secondInstance->ActivateEntities();

        AZ::Entity* secondNewEntity = nullptr;

        AZ::ComponentApplicationBus::BroadcastResult(secondNewEntity,
            &AZ::ComponentApplicationBus::Events::FindEntity, secondInstance->GetEntityId(newEntityAlias));

        ASSERT_TRUE(secondNewEntity);

        PrefabTestComponent* secondComponent = secondNewEntity->FindComponent<PrefabTestComponent>();
        ASSERT_TRUE(secondComponent);

        // Validate that the entity reference is preserved in the second instance
        ASSERT_TRUE(secondComponent->m_entityIdProperty.IsValid());
        ASSERT_EQ(secondComponent->m_entityIdProperty, secondInstance->GetEntityId(referencedEntityAlias));
    }

    TEST_F(PrefabEntityAliasTests, PrefabEntityAlias_ReferenceNotInSameHierarchy_ReferenceGoesToNull)
    {
        // Make a new entity with a test component
        AZ::Entity* newEntity = CreateEntity("New Entity", false);
        PrefabTestComponent* newComponent = newEntity->CreateComponent<PrefabTestComponent>();
        ASSERT_TRUE(newComponent);

        // Generate a second entity that will be referenced by the first
        // however we won't add both to the same prefab hierarchy
        AZ::Entity* referencedEntity = CreateEntity("Referenced Entity");
        newComponent->m_entityIdProperty = referencedEntity->GetId();

        AZStd::unique_ptr<Instance> unrelatedInstance =
            m_prefabSystemComponent->CreatePrefab({ referencedEntity }, {}, "test/path/0");

        AZStd::unique_ptr<Instance> firstInstance =
            m_prefabSystemComponent->CreatePrefab({ newEntity }, {}, "test/path/1");

        ASSERT_TRUE(firstInstance);

        // Grab the alias of newEntity so it can be found in a new instantiation
        EntityAliasOptionalReference newEntityAliasRef = firstInstance->GetEntityAlias(newEntity->GetId());
        ASSERT_TRUE(newEntityAliasRef);

        EntityAlias newEntityAlias = newEntityAliasRef.value();

        // On instantiation the referenced entity should be invalid
        AZStd::unique_ptr<Instance> secondInstance =
            m_prefabSystemComponent->InstantiatePrefab(firstInstance->GetTemplateId());

        // Activate the entities so that we can find them via the component application bus
        secondInstance->InitializeEntities();
        secondInstance->ActivateEntities();

        AZ::Entity* secondNewEntity = nullptr;

        AZ::ComponentApplicationBus::BroadcastResult(secondNewEntity,
            &AZ::ComponentApplicationBus::Events::FindEntity, secondInstance->GetEntityId(newEntityAlias));

        ASSERT_TRUE(secondNewEntity);

        PrefabTestComponent* secondComponent = secondNewEntity->FindComponent<PrefabTestComponent>();
        ASSERT_TRUE(secondComponent);

        ASSERT_FALSE(secondComponent->m_entityIdProperty.IsValid());
    }

    TEST_F(PrefabEntityAliasTests, PrefabEntityAlias_ReferenceEntityFoundInNestedInstance_ReferencePersists)
    {
        // Make a new entity with a test component
        AZ::Entity* newEntity = CreateEntity("New Entity", false);
        PrefabTestComponent* newComponent = newEntity->CreateComponent<PrefabTestComponent>();
        ASSERT_TRUE(newComponent);

        // Generate a second entity that will be referenced by the first
        AZ::Entity* referencedEntity = CreateEntity("Referenced Entity");

        newComponent->m_entityIdProperty = referencedEntity->GetId();

        // Build out a prefab holding the referenced entity
        AZStd::unique_ptr<Instance> nestedInstance =
            m_prefabSystemComponent->CreatePrefab({ referencedEntity }, {}, "Test/Path/0");

        // Grab the alias of the nested instance
        EntityAliasOptionalReference referencedEntityAliasRef =
            nestedInstance->GetEntityAlias(referencedEntity->GetId());
        ASSERT_TRUE(referencedEntityAliasRef);

        EntityAlias referencedEntityAlias = referencedEntityAliasRef.value();

        TemplateId nestedTemplateId = nestedInstance->GetTemplateId();

        // Create our root instance and nest our first instance under it
        AZStd::unique_ptr<Instance> rootInstance =
            m_prefabSystemComponent->CreatePrefab({ newEntity },
                PrefabTestUtils::MakeInstanceList(AZStd::move(nestedInstance)), "Test/Path/1");

        // Acquire the alias name of our entity so we can look it up in future instances
        EntityAliasOptionalReference newEntityAliasRef =
            rootInstance->GetEntityAlias(newEntity->GetId());
        ASSERT_TRUE(newEntityAliasRef);

        EntityAlias newEntityAlias = newEntityAliasRef.value();

        // Acquire the nested instance alias so we can look it up in future instances
        AZStd::vector<InstanceAlias> nestedInstanceAliases = rootInstance->GetNestedInstanceAliases(nestedTemplateId);
        ASSERT_EQ(nestedInstanceAliases.size(), 1);

        InstanceAlias nestedAlias = nestedInstanceAliases[0];

        // Make a new instance of root
        // Entity references should be preserved among its unique entities
        AZStd::unique_ptr<Instance> secondRootInstance =
            m_prefabSystemComponent->InstantiatePrefab(rootInstance->GetTemplateId());

        ASSERT_TRUE(secondRootInstance);

        // Activate the entities so that we can find them via the component application bus
        secondRootInstance->InitializeNestedEntities();
        secondRootInstance->ActivateNestedEntities();

        // Find the new instances versions of the new and referenced entities using the aliases we saved
        AZ::EntityId secondNewEntityId = secondRootInstance->GetEntityId(newEntityAlias);

        InstanceOptionalReference secondNestedInstance = secondRootInstance->FindNestedInstance(nestedAlias);
        ASSERT_TRUE(secondNestedInstance);

        AZ::EntityId secondReferencedEntityId = secondNestedInstance->get().GetEntityId(referencedEntityAlias);

        AZ::Entity* secondNewEntity = nullptr;
        AZ::ComponentApplicationBus::BroadcastResult(secondNewEntity,
            &AZ::ComponentApplicationBus::Events::FindEntity, secondNewEntityId);

        ASSERT_TRUE(secondNewEntity);

        PrefabTestComponent* secondComponent = secondNewEntity->FindComponent<PrefabTestComponent>();
        ASSERT_TRUE(secondComponent);

        EXPECT_TRUE(secondComponent->m_entityIdProperty.IsValid());
        EXPECT_EQ(secondComponent->m_entityIdProperty, secondReferencedEntityId);
    }

    TEST_F(PrefabEntityAliasTests, PrefabEntityAlias_ReferenceEntityFoundInParentInstance_ReferencePersists)
    {
        // Make a new entity with a test component
        AZ::Entity* newEntity = CreateEntity("New Entity", false);
        PrefabTestComponent* newComponent = newEntity->CreateComponent<PrefabTestComponent>();
        ASSERT_TRUE(newComponent);

        // Generate a second entity that will be referenced by the first
        AZ::Entity* referencedEntity = CreateEntity("Referenced Entity");

        // Make our first instance to be nested under the prefab containing the entity we reference
        AZStd::unique_ptr<Instance> nestedInstance =
            m_prefabSystemComponent->CreatePrefab({ newEntity }, {}, "Test/Path/0");

        // Save off the entity alias so we can find it in future instances
        EntityAliasOptionalReference newEntityAliasRef = nestedInstance->GetEntityAlias(newEntity->GetId());
        ASSERT_TRUE(newEntityAliasRef);

        EntityAlias entityAlias = newEntityAliasRef.value();

        TemplateId nestedTemplateId = nestedInstance->GetTemplateId();

        // Make our root instance which contains the entity being referenced
        AZStd::unique_ptr<Instance> rootInstance =
            m_prefabSystemComponent->CreatePrefab({ referencedEntity },
                PrefabTestUtils::MakeInstanceList(AZStd::move(nestedInstance)), "Test/Path/1");

        // Acquire the nested instance alias so we can look it up in future instances
        AZStd::vector<InstanceAlias> nestedInstanceAliases = rootInstance->GetNestedInstanceAliases(nestedTemplateId);
        ASSERT_EQ(nestedInstanceAliases.size(), 1);

        InstanceAlias nestedAlias = nestedInstanceAliases[0];

        // Save off the referenced entity alias
        EntityAliasOptionalReference referencedEntityAliasRef =
            rootInstance->GetEntityAlias(referencedEntity->GetId());

        ASSERT_TRUE(referencedEntityAliasRef);

        EntityAlias referencedEntityAlias = referencedEntityAliasRef.value();

        // Capture the before and after for setting the reference property
        PrefabDom entityDomBeforeUpdate;
        m_instanceToTemplateInterface->GenerateDomForEntity(entityDomBeforeUpdate, *newEntity);

        newComponent->m_entityIdProperty = referencedEntity->GetId();

        PrefabDom entityDomAfterUpdate;
        m_instanceToTemplateInterface->GenerateDomForEntity(entityDomAfterUpdate, *newEntity);

        PrefabDom patch;
        m_instanceToTemplateInterface->GeneratePatch(patch, entityDomBeforeUpdate, entityDomAfterUpdate);

        // Patch the nested prefab to reference an entity in its parent
        m_instanceToTemplateInterface->PatchEntityInTemplate(patch, newEntity->GetId());

        // Using the aliases we saved grab the updated entities so we can verify the entity reference is still preserved
        InstanceOptionalReference updatedNestedInstance = rootInstance->FindNestedInstance(nestedAlias);
        ASSERT_TRUE(updatedNestedInstance);

        AZ::EntityId updatedNewEntityId = updatedNestedInstance->get().GetEntityId(entityAlias);
        AZ::Entity* updatedNewEntity = nullptr;

        // Activate the entities so that we can find them via the component application bus
        rootInstance->InitializeNestedEntities();
        rootInstance->ActivateNestedEntities();

        AZ::ComponentApplicationBus::BroadcastResult(updatedNewEntity,
            &AZ::ComponentApplicationBus::Events::FindEntity, updatedNewEntityId);

        ASSERT_TRUE(updatedNewEntity);

        PrefabTestComponent* updatedComponent = updatedNewEntity->FindComponent<PrefabTestComponent>();
        ASSERT_TRUE(updatedComponent);

        AZ::EntityId updatedReferencedEntityId = rootInstance->GetEntityId(referencedEntityAlias);

        EXPECT_TRUE(updatedComponent->m_entityIdProperty.IsValid());
        EXPECT_EQ(updatedComponent->m_entityIdProperty, updatedReferencedEntityId);
    }
}
