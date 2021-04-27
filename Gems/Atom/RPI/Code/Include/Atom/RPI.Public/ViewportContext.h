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

#include <AzFramework/Viewport/ViewportBus.h>
#include <AzFramework/Windowing/WindowBus.h>
#include <Atom/RPI.Public/WindowContext.h>
#include <Atom/RPI.Public/Scene.h>
#include <Atom/RPI.Public/SceneBus.h>
#include <AzCore/EBus/Event.h>

namespace AZ
{
    namespace RPI
    {
        class ViewportContextManager;

        //! ViewportContext wraps a native window and represents a minimal viewport
        //! in which a scene is rendered on-screen
        //! ViewportContexts are registered on creation to allow consumers to listen to notifications
        //! and manage the view stack for a given viewport
        class ViewportContext
            : public SceneNotificationBus::Handler
            , public AzFramework::WindowNotificationBus::Handler
            , public AzFramework::ViewportRequestBus::Handler
        {
        public:
            AZ_CLASS_ALLOCATOR(ViewportContext, AZ::SystemAllocator, 0);

            //! Used by ViewportContextManager, use AZ::Interface<ViewportContextRequestsInterface>::Get()->CreateViewportContext to create a viewport
            //! context from outside of the ViewportContextManager.
            ViewportContext(ViewportContextManager* manager, AzFramework::ViewportId id, const AZ::Name& name, RHI::Device& device, AzFramework::NativeWindowHandle nativeWindow, ScenePtr renderScene);
            ~ViewportContext();

            //! Gets an opaque ID that can be used to uniquely identify this ViewportContext.
            AzFramework::ViewportId GetId() const;
            //! Convenience method, gets the window handle associated with this viewport's window manager.
            AzFramework::NativeWindowHandle GetWindowHandle() const;
            //! Gets the window context associated with this viewport.
            WindowContextSharedPtr GetWindowContext();
            //! Gets the root scene (if any) associated with this viewport.
            ScenePtr GetRenderScene();
            //! Gets the current render pipeline associated with our WindowContext, if there is one.
            RenderPipelinePtr GetCurrentPipeline();
            //! Sets the root scene associated with this viewport.
            //! This does not provide a default render pipeline, one must be provided to enable rendering.
            void SetRenderScene(ScenePtr scene);
            //! Runs one simulation and render tick and renders a frame to this viewport's window.
            //! @note This is likely to be replaced by a tick management system in the RPI.
            void RenderTick();

            //! Gets the current name of this ViewportContext.
            //! This name is used to tie this ViewportContext to its View stack, and ViewportContexts may be
            //! renamed via AZ::Interface<ViewportContextRequestsInterface>::Get()->RenameViewportContext.
            AZ::Name GetName() const;

            //! Gets the default view associated with this ViewportContext.
            //! Alternatively, use AZ::Interface<ViewportContextRequestsInterface>::Get()->GetCurrentView.
            ViewPtr GetDefaultView();
            ConstViewPtr GetDefaultView() const;

            //! Gets the current size of the viewport.
            AzFramework::WindowSize GetViewportSize() const;

            // SceneNotificationBus interface
            //! Ensures our default view remains set when our scene's render pipelines are modified.
            void OnRenderPipelineAdded(RenderPipelinePtr pipeline) override;
            //! Ensures our default view remains set when our scene's render pipelines are modified.
            void OnRenderPipelineRemoved(RenderPipeline* pipeline) override;
            //! OnBeginPrepareRender is forwarded to our RenderTick notification to allow subscribers to do rendering.
            void OnBeginPrepareRender() override;

            //WindowNotificationBus interface
            //! Used to fire a notification when our window resizes
            void OnWindowResized(uint32_t width, uint32_t height) override;

            using SizeChangedEvent = AZ::Event<AzFramework::WindowSize>;
            //! Notifies consumers when the viewport size has changed.
            //! Alternatively, connect to ViewportContextNotificationsBus and listen to ViewportContextNotifications::OnViewportSizeChanged.
            void ConnectSizeChangedHandler(SizeChangedEvent::Handler& handler);

            using MatrixChangedEvent = AZ::Event<const AZ::Matrix4x4&>;
            //! Notifies consumers when the view matrix has changed.
            void ConnectViewMatrixChangedHandler(MatrixChangedEvent::Handler& handler);
            //! Notifies consumers when the projection matrix has changed.
            void ConnectProjectionMatrixChangedHandler(MatrixChangedEvent::Handler& handler);

            using SceneChangedEvent = AZ::Event<ScenePtr>;
            //! Notifies consumers when the render scene has changed.
            void ConnectSceneChangedHandler(SceneChangedEvent::Handler& handler);

            // ViewportRequestBus interface
            //! Gets the current camera's view matrix.
            const AZ::Matrix4x4& GetCameraViewMatrix() const override;
            //! Sets the current camera's view matrix.
            void SetCameraViewMatrix(const AZ::Matrix4x4& matrix) override;
            //! Gets the current camera's projection matrix.
            const AZ::Matrix4x4& GetCameraProjectionMatrix() const override;
            //! Sets the current camera's projection matrix.
            void SetCameraProjectionMatrix(const AZ::Matrix4x4& matrix) override;
            //! Convenience method, gets the AZ::Transform corresponding to this camera's view matrix.
            AZ::Transform GetCameraTransform() const override;
            //! Convenience method, sets the camera's view matrix from this AZ::Transform.
            void SetCameraTransform(const AZ::Transform& transform) override;

        private:
            // Used by the manager to set the current default camera.
            void SetDefaultView(ViewPtr view);
            // Ensures our render pipeline's default camera matches ours.
            void UpdatePipelineView();

            ScenePtr m_rootScene;
            WindowContextSharedPtr m_windowContext;
            ViewPtr m_defaultView;
            AzFramework::WindowSize m_viewportSize;
            SizeChangedEvent m_sizeChangedEvent;
            MatrixChangedEvent m_viewMatrixChangedEvent;
            MatrixChangedEvent::Handler m_onViewMatrixChangedHandler;
            MatrixChangedEvent m_projectionMatrixChangedEvent;
            MatrixChangedEvent::Handler m_onProjectionMatrixChangedHandler;
            SceneChangedEvent m_sceneChangedEvent;
            ViewportContextManager* m_manager;
            RenderPipelinePtr m_currentPipeline;
            Name m_name;
            AzFramework::ViewportId m_id;

            friend class ViewportContextManager;
        };
    } // namespace RPI
} // namespace AZ
