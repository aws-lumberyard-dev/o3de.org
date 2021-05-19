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

#include <AzCore/Asset/AssetManagerComponent.h>
#include <AzCore/RTTI/BehaviorContext.h>
#include <AzCore/Script/ScriptAsset.h>
#include <AzCore/Script/ScriptSystemComponent.h>
#include <AzCore/Script/ScriptContext.h>
#include <AzCore/UnitTest/TestTypes.h>
#include <AzToolsFramework/ToolsComponents/ScriptEditorComponent.h>

#include "EntityTestbed.h"

extern "C" {
#   include <Lua/lualib.h>
#   include <Lua/lauxlib.h>
}

namespace UnitTest
{
    using namespace AZ;
    using namespace AzFramework;

    // Global Properties used for Testing
    int mySubValue = 0;
    int myReloadValue = 0;

    class ScriptComponentTest
        : public testing::Test
    {
    public:
        AZ_TYPE_INFO(ScriptComponentTest, "{85CDBD49-70FF-416A-8154-B5525EDD30D4}");

        void SetUp() override
        {        
            ComponentApplication::Descriptor appDesc;
            appDesc.m_memoryBlocksByteSize = 100 * 1024 * 1024;
            //appDesc.m_recordsMode = AllocationRecords::RECORD_FULL;
            //appDesc.m_stackRecordLevels = 20;
            Entity* systemEntity = m_app.Create(appDesc);

            systemEntity->CreateComponent<MemoryComponent>();
            systemEntity->CreateComponent("{CAE3A025-FAC9-4537-B39E-0A800A2326DF}"); // JobManager component
            systemEntity->CreateComponent<StreamerComponent>();
            systemEntity->CreateComponent<AssetManagerComponent>();
            systemEntity->CreateComponent("{A316662A-6C3E-43E6-BC61-4B375D0D83B4}"); // Usersettings component
            systemEntity->CreateComponent<ScriptSystemComponent>();

            systemEntity->Init();
            systemEntity->Activate();

            EBUS_EVENT_RESULT(m_scriptContext, ScriptSystemRequestBus, GetContext, DefaultScriptContextId);
            EBUS_EVENT_RESULT(m_behaviorContext, AZ::ComponentApplicationBus, GetBehaviorContext);
            EBUS_EVENT_RESULT(m_serializeContext, AZ::ComponentApplicationBus, GetSerializeContext);

            AzToolsFramework::Components::ScriptEditorComponent::CreateDescriptor(); // descriptor is deleted by app
            AzToolsFramework::Components::ScriptEditorComponent::Reflect(m_serializeContext);

            ScriptComponent::CreateDescriptor(); // descriptor is deleted by app
            ScriptComponent::Reflect(m_serializeContext);
        }

        void TearDown() override
        {
            m_app.Destroy();
        }

        Data::Asset<ScriptAsset> CreateAndLoadScriptAsset(const AZStd::string& script)
        {
            Data::Asset<ScriptAsset> scriptAsset = Data::AssetManager::Instance().CreateAsset<ScriptAsset>(Uuid::CreateRandom());
            scriptAsset.Get()->m_scriptBuffer.insert(scriptAsset.Get()->m_scriptBuffer.begin(), script.begin(), script.end());
            EBUS_EVENT(Data::AssetManagerBus, OnAssetReady, scriptAsset);

            m_app.Tick();
            m_app.TickSystem(); // flush assets etc.

            return scriptAsset;
        }

        static ScriptComponent* BuildGameEntity(const Data::Asset<ScriptAsset>& scriptAsset, Entity& gameEntity)
        {
            // We first setup the ScriptEditorComponent.
            // After a script asset is loaded the ScriptEditorComponent builds the properties table.
            // BuildGameEntity() hands off the properties table to the game runtime ScriptComponent.
            Entity editorEntity;
            auto* scriptEditorComponent = editorEntity.CreateComponent<AzToolsFramework::Components::ScriptEditorComponent>();
            scriptEditorComponent->SetScript(scriptAsset);
            editorEntity.Init();
            editorEntity.Activate();

            scriptEditorComponent->BuildGameEntity(&gameEntity);

            auto* scriptComponent = gameEntity.FindComponent<ScriptComponent>();
            return scriptComponent;
        }

        static void OverwriteScriptBuffer(Data::Asset<ScriptAsset> scriptAsset, AZStd::string newScript)
        {
            scriptAsset.Get()->m_scriptBuffer.insert(scriptAsset.Get()->m_scriptBuffer.begin(), newScript.begin(), newScript.end());
        }

        ComponentApplication m_app;
        ScriptContext* m_scriptContext = nullptr;
        BehaviorContext* m_behaviorContext = nullptr;
        SerializeContext* m_serializeContext = nullptr;
    };


    TEST_F(ScriptComponentTest, ScriptInstancesCanReadButDontModifySourceTable)
    {
        // make sure script instances don't can read only share data, but don't modify the source table
        const AZStd::string script = "test = {\
                        --[[test with no properties table as this should work too!]]\
                        state = {\
                            mysubstate = {\
                               mysubvalue = 2,\
                            },\
                            myvalue = 0,\
                          },\
                        }\
                        function test:OnActivate()\
                          self.state.mysubstate.mysubvalue = 5\
                        end\
                        return test;";

        Data::Asset<ScriptAsset> scriptAsset = CreateAndLoadScriptAsset(script);

        auto* entity1 = aznew Entity();
        entity1->CreateComponent<ScriptComponent>()->SetScript(scriptAsset);

        entity1->Init();
        entity1->Activate();

        auto* entity2 = aznew Entity();
        entity2->CreateComponent<ScriptComponent>()->SetScript(scriptAsset);

        entity2->Init();
        entity2->Activate();

        m_behaviorContext->Property("globalMySubValue", BehaviorValueProperty(&mySubValue));
        m_scriptContext->Execute("globalMySubValue = test.state.mysubstate.mysubvalue", "Read my subvalue");
        AZ_TEST_ASSERT(mySubValue == 2); // we should not have changed test. table but the instance table of each component.

        delete entity1;
        delete entity2;
    }


    TEST_F(ScriptComponentTest, ScriptReloads)
    {
        // Test script reload
        m_behaviorContext->Property("myReloadValue", BehaviorValueProperty(&myReloadValue));
        const AZStd::string script1 ="local testReload = {}\
                                function testReload:OnActivate()\
                                  myReloadValue = 1\
                                end\
                                function testReload:OnDeactivate()\
                                  myReloadValue = 0\
                                end\
                                return testReload;";
        Data::Asset<ScriptAsset> scriptAsset1 = CreateAndLoadScriptAsset(script1);

        auto* entity = aznew Entity();
        entity->CreateComponent<ScriptComponent>()->SetScript(scriptAsset1);

        entity->Init();
        entity->Activate();

        // test value, it should set during activation of the first script
        AZ_TEST_ASSERT(myReloadValue == 1);

        const AZStd::string script2 ="local testReload = {}\
                                function testReload:OnActivate()\
                                   myReloadValue = 5\
                                end\
                                return testReload";

        // modify the asset
        Data::Asset<ScriptAsset> scriptAsset2(aznew ScriptAsset(scriptAsset1.GetId()), AZ::Data::AssetLoadBehavior::Default);
        OverwriteScriptBuffer(scriptAsset2, script2);

        // When reloading script assets from files, ScriptSystemComponent would clear old script caches automatically in the
        // function `ScriptSystemComponent::LoadAssetData()`. But here we are changing script directly in memory, therefore we 
        // need to clear old cache manually.
        AZ::ScriptSystemRequestBus::Broadcast(&AZ::ScriptSystemRequestBus::Events::ClearAssetReferences, scriptAsset1.GetId());

        // trigger reload
        Data::AssetManager::Instance().ReloadAssetFromData(scriptAsset2);
        
        // ReloadAssetFromData is (now) a queued event
        // Need to tick subsystems here to receive reload event.
        m_app.Tick();
        m_app.TickSystem();

        // test value with the reloaded value
        EXPECT_EQ(5, myReloadValue);

        delete entity;
    }

    TEST_F(ScriptComponentTest, LuaPropertiesAreDiscovered)
    {
        const AZStd::string script = "local test = {\
                                      Properties = {\
                                        myNum = { default = 2 },\
                                      },\
                                    }\
                                    function test:OnActivate()\
                                      self.Properties.myNum = 5\
                                    end\
                                    return test";

        const Data::Asset<ScriptAsset> scriptAsset = CreateAndLoadScriptAsset(script);
        Entity editorEntity, gameEntity;
        auto* scriptComponent = BuildGameEntity(scriptAsset, gameEntity);

        EXPECT_NE(scriptComponent->GetScriptProperty("myNum"), nullptr);
    }
} // namespace UnitTest
