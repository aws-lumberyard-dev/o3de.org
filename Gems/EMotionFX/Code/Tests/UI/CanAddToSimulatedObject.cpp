/*
 * Copyright (c) Contributors to the Open 3D Engine Project
 * 
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <gtest/gtest.h>

#include <QPushButton>
#include <QAction>
#include <QtTest>

#include <AzCore/std/algorithm.h>
#include <AzFramework/Physics/Character.h>
#include <AzFramework/Physics/ShapeConfiguration.h>
#include <Editor/ColliderContainerWidget.h>
#include <Editor/Plugins/Ragdoll/RagdollNodeInspectorPlugin.h>
#include <Editor/Plugins/SimulatedObject/SimulatedObjectColliderWidget.h>
#include <Editor/Plugins/SkeletonOutliner/SkeletonOutlinerPlugin.h>
#include <Editor/ReselectingTreeView.h>
#include <Tests/TestAssetCode/SimpleActors.h>
#include <Tests/TestAssetCode/ActorFactory.h>
#include <Tests/UI/ModalPopupHandler.h>
#include <Tests/UI/UIFixture.h>
#include <Tests/D6JointLimitConfiguration.h>

namespace EMotionFX
{
    class CanAddToSimulatedObjectFixture
        : public UIFixture
    {
    public:
        void SetUp() override
        {
            SetupQtAndFixtureBase();

            AZ::SerializeContext* serializeContext = nullptr;
            AZ::ComponentApplicationBus::BroadcastResult(serializeContext, &AZ::ComponentApplicationBus::Events::GetSerializeContext);

            D6JointLimitConfiguration::Reflect(serializeContext);

            SetupPluginWindows();
        }
    };

    TEST_F(CanAddToSimulatedObjectFixture, CanAddExistingJointsAndUnaddedChildren)
    {
        RecordProperty("test_case_id", "C14603914");

        AutoRegisteredActor actor = ActorFactory::CreateAndInit<SimpleJointChainActor>(7, "CanAddToSimulatedObjectActor");

        ActorInstance* actorInstance = ActorInstance::Create(actor.get());

        // Change the Editor mode to Simulated Objects
        EMStudio::GetMainWindow()->ApplicationModeChanged("SimulatedObjects");

        // Select the newly created actor instance
        AZStd::string result;
        EXPECT_TRUE(CommandSystem::GetCommandManager()->ExecuteCommand(AZStd::string{ "Select -actorInstanceID " } + AZStd::to_string(actorInstance->GetID()), result)) << result.c_str();

        EMotionFX::SkeletonOutlinerPlugin* skeletonOutliner = static_cast<EMotionFX::SkeletonOutlinerPlugin*>(EMStudio::GetPluginManager()->FindActivePlugin(EMotionFX::SkeletonOutlinerPlugin::CLASS_ID));
        EXPECT_TRUE(skeletonOutliner);
        ReselectingTreeView* skeletonTreeView = skeletonOutliner->GetDockWidget()->findChild<ReselectingTreeView*>("EMFX.SkeletonOutlinerPlugin.SkeletonOutlinerTreeView");
        EXPECT_TRUE(skeletonTreeView);
        const QAbstractItemModel* skeletonModel = skeletonTreeView->model();

        QModelIndexList indexList;
        skeletonTreeView->RecursiveGetAllChildren(skeletonModel->index(0, 0), indexList);
        EXPECT_EQ(indexList.size(), 7);

        SelectIndexes(indexList, skeletonTreeView, 2, 4);

        // Bring up the contextMenu
        const QRect rect = skeletonTreeView->visualRect(indexList[3]);
        EXPECT_TRUE(rect.isValid());
        BringUpContextMenu(skeletonTreeView, rect);

        QMenu* contextMenu = skeletonOutliner->GetDockWidget()->findChild<QMenu*>("EMFX.SkeletonOutlinerPlugin.ContextMenu");
        EXPECT_TRUE(contextMenu);
        QAction* addSelectedJointAction;
        EXPECT_TRUE(UIFixture::GetActionFromContextMenu(addSelectedJointAction, contextMenu, "Add to simulated object"));
        QMenu* addSelectedJointMenu = addSelectedJointAction->menu();
        EXPECT_TRUE(addSelectedJointMenu);
        QAction* newSimulatedObjectAction;
        EXPECT_TRUE(UIFixture::GetActionFromContextMenu(newSimulatedObjectAction, addSelectedJointMenu, "New simulated object..."));

        // Handle the add children dialog box.
        ModalPopupHandler messageBoxPopupHandler;
        messageBoxPopupHandler.WaitForPopupPressDialogButton<QMessageBox*>(QDialogButtonBox::No);
        newSimulatedObjectAction->trigger();

        EMStudio::InputDialogValidatable* inputDialog = qobject_cast<EMStudio::InputDialogValidatable*>(FindTopLevelWidget("EMFX.SimulatedObjectActionManager.SimulatedObjectDialog"));
        ASSERT_NE(inputDialog, nullptr) << "Cannot find input dialog.";

        inputDialog->SetText("TestObj");
        inputDialog->accept();

        QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

        ASSERT_EQ(actor->GetSimulatedObjectSetup()->GetNumSimulatedObjects(), 1);
        const auto simulatedObject = actor->GetSimulatedObjectSetup()->GetSimulatedObject(0);
        EXPECT_STREQ(simulatedObject->GetName().c_str(), "TestObj");
        EXPECT_EQ(simulatedObject->GetNumSimulatedJoints(), 3) << "There aren't 3 joints in the object";

        // Select 1 extra joint this time but still have the original 3
        SelectIndexes(indexList, skeletonTreeView, 2, 5);
        {
            // Bring up the contextMenu
            const QRect rect2 = skeletonTreeView->visualRect(indexList[4]);
            EXPECT_TRUE(rect2.isValid());
            BringUpContextMenu(skeletonTreeView, rect2);

            QMenu* contextMenu2 = skeletonOutliner->GetDockWidget()->findChild<QMenu*>("EMFX.SkeletonOutlinerPlugin.ContextMenu");
            EXPECT_TRUE(contextMenu2);
            QAction* addSelectedJointAction2;
            EXPECT_TRUE(UIFixture::GetActionFromContextMenu(addSelectedJointAction2, contextMenu2, "Add to simulated object"));
            QMenu* addSelectedJointMenu2 = addSelectedJointAction2->menu();
            EXPECT_TRUE(addSelectedJointMenu2);
            QAction* newSimulatedObjectAction2;
            ASSERT_TRUE(UIFixture::GetActionFromContextMenu(newSimulatedObjectAction2, addSelectedJointMenu2, "TestObj")) << "Can't find named simulated object in menu";

            // Handle the add children dialog box.
            ModalPopupHandler messageBoxPopupHandler2;
            messageBoxPopupHandler2.WaitForPopupPressDialogButton<QMessageBox*>(QDialogButtonBox::No);
            newSimulatedObjectAction2->trigger();
        }
        EXPECT_EQ(simulatedObject->GetNumSimulatedRootJoints(), 1);
        EXPECT_EQ(simulatedObject->GetNumSimulatedJoints(), 4) << "More than 1 extra object added";

        QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

        actorInstance->Destroy();
    }

    TEST_F(CanAddToSimulatedObjectFixture, CanAddCollidersfromRagdoll)
    {
        RecordProperty("test_case_id", "C13291807");
        AutoRegisteredActor actor = ActorFactory::CreateAndInit<SimpleJointChainActor>(7, "CanAddToSimulatedObjectActor");

        ActorInstance* actorInstance = ActorInstance::Create(actor.get());

        // Change the Editor mode to Physics
        EMStudio::GetMainWindow()->ApplicationModeChanged("Physics");

        // Select the newly created actor instance
        AZStd::string result;
        EXPECT_TRUE(CommandSystem::GetCommandManager()->ExecuteCommand(AZStd::string{ "Select -actorInstanceID " } + AZStd::to_string(actorInstance->GetID()), result)) << result.c_str();

        EMotionFX::SkeletonOutlinerPlugin* skeletonOutliner = static_cast<EMotionFX::SkeletonOutlinerPlugin*>(EMStudio::GetPluginManager()->FindActivePlugin(EMotionFX::SkeletonOutlinerPlugin::CLASS_ID));
        EXPECT_TRUE(skeletonOutliner);
        ReselectingTreeView* skeletonTreeView = skeletonOutliner->GetDockWidget()->findChild<ReselectingTreeView*>("EMFX.SkeletonOutlinerPlugin.SkeletonOutlinerTreeView");
        EXPECT_TRUE(skeletonTreeView);
        const QAbstractItemModel* skeletonModel = skeletonTreeView->model();

        QModelIndexList indexList;
        skeletonTreeView->RecursiveGetAllChildren(skeletonModel->index(0, 0), indexList);
        EXPECT_EQ(indexList.size(), 7);

        SelectIndexes(indexList, skeletonTreeView, 2, 4);

        // Bring up the contextMenu to add the collider to the Ragdoll.
        const QRect rect = skeletonTreeView->visualRect(indexList[3]);
        EXPECT_TRUE(rect.isValid());
        BringUpContextMenu(skeletonTreeView, rect);

        QMenu* contextMenu = skeletonOutliner->GetDockWidget()->findChild<QMenu*>("EMFX.SkeletonOutlinerPlugin.ContextMenu");
        EXPECT_TRUE(contextMenu);
        QAction* ragdollAction;
        EXPECT_TRUE(UIFixture::GetActionFromContextMenu(ragdollAction, contextMenu, "Ragdoll"));
        QMenu* ragdollMenu = ragdollAction->menu();
        EXPECT_TRUE(ragdollMenu);
        QAction* addToRagdollAction;
        EXPECT_TRUE(UIFixture::GetActionFromContextMenu(addToRagdollAction, ragdollMenu, "Add to ragdoll"));
        addToRagdollAction->trigger();

        // Change the Editor mode to SimulatedObjects
        EMStudio::GetMainWindow()->ApplicationModeChanged("SimulatedObjects");

        SelectIndexes(indexList, skeletonTreeView, 2, 4);

        // Copy the ragdoll collider setup to simulated object colliders
        QDockWidget* simulatedObjectInspectorDock = EMStudio::GetMainWindow()->findChild<QDockWidget*>("EMFX.SimulatedObjectWidget.SimulatedObjectInspectorDock");
        ASSERT_TRUE(simulatedObjectInspectorDock);
        QPushButton* addColliderButton = simulatedObjectInspectorDock->findChild<QPushButton*>("EMFX.SimulatedObjectColliderWidget.AddColliderButton");
        ASSERT_TRUE(addColliderButton);
        QTest::mouseClick(addColliderButton, Qt::LeftButton);

        QMenu* addColliderMenu = addColliderButton->findChild<QMenu*>("EMFX.AddColliderButton.ContextMenu");
        EXPECT_TRUE(addColliderMenu);
        QAction* copyRagdollAction;
        ASSERT_TRUE(UIFixture::GetActionFromContextMenu(copyRagdollAction, addColliderMenu, "Copy from Ragdoll"));
        copyRagdollAction->trigger();

        SimulatedObjectColliderWidget* colliderWidget = simulatedObjectInspectorDock->findChild<SimulatedObjectColliderWidget*>(
            "EMFX.SimulatedJointWidget.SimulatedObjectColliderWidget");
        ASSERT_TRUE(colliderWidget);
        ColliderContainerWidget* containerWidget =
            colliderWidget->findChild<ColliderContainerWidget*>("EMFX.SimulatedObjectColliderWidget.ColliderContainerWidget");
        ASSERT_TRUE(containerWidget);
        EXPECT_EQ(containerWidget->ColliderType(), PhysicsSetup::ColliderConfigType::Unknown) << "Collider type not Unknown";

        for (int i = 2; i <= 4; ++i)
        {
            skeletonTreeView->selectionModel()->clearSelection();
            SelectIndexes(indexList, skeletonTreeView, i, i);
            EXPECT_EQ(containerWidget->ColliderType(), PhysicsSetup::ColliderConfigType::SimulatedObjectCollider)
                << "Simulated Collider type not found";
        }

        const AZStd::shared_ptr<PhysicsSetup>& physicsSetup = actor->GetPhysicsSetup();
        Physics::CharacterColliderConfiguration* colliderConfig = physicsSetup->GetColliderConfigByType(PhysicsSetup::ColliderConfigType::SimulatedObjectCollider);
        EXPECT_TRUE(colliderConfig) << "Can't find Simulated Object Colliders";
        AZStd::vector<Physics::CharacterColliderNodeConfiguration> nodes = colliderConfig->m_nodes;

        const AZStd::vector<AZStd::string> gotNodeNamesInColliderConfig = [&nodes]()
        {
            AZStd::vector<AZStd::string> nodeNames(nodes.size());
            AZStd::transform(begin(nodes), end(nodes), begin(nodeNames), [](const auto& nodeConfig) { return nodeConfig.m_name; });
            return nodeNames;
        }();

        const AZStd::vector<Physics::ShapeType> gotShapeTypes = [&nodes]()
        {
            AZStd::vector<Physics::ShapeType> shapeTypes(nodes.size());
            AZStd::transform(begin(nodes), end(nodes), begin(shapeTypes), [](const Physics::CharacterColliderNodeConfiguration& nodeConfig)
            {
                return nodeConfig.m_shapes.empty() ? Physics::ShapeType(0xff) : nodeConfig.m_shapes[0].second->GetShapeType();
            });
            return shapeTypes;
        }();

        EXPECT_THAT(gotNodeNamesInColliderConfig, testing::Pointwise(StrEq(), AZStd::vector<AZStd::string> {"joint2", "joint3", "joint4"}));
        EXPECT_THAT(gotShapeTypes, testing::Pointwise(testing::Eq(), AZStd::vector<Physics::ShapeType> {Physics::ShapeType::Capsule, Physics::ShapeType::Capsule, Physics::ShapeType::Capsule}));

        QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
        actorInstance->Destroy();
    }
}   // namespace EMotionFX
