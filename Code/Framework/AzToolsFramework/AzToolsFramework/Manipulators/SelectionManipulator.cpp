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

#include "SelectionManipulator.h"

#include <AzFramework/Entity/EntityDebugDisplayBus.h>

namespace AzToolsFramework
{
    AZStd::shared_ptr<SelectionManipulator> SelectionManipulator::MakeShared(
        const AZ::Transform& worldFromLocal, const AZ::Vector3& nonUniformScale)
    {
        return AZStd::shared_ptr<SelectionManipulator>(aznew SelectionManipulator(worldFromLocal, nonUniformScale));
    }

    SelectionManipulator::SelectionManipulator(const AZ::Transform& worldFromLocal, const AZ::Vector3& nonUniformScale)
    {
        SetSpace(worldFromLocal);
        SetNonUniformScale(nonUniformScale);
        AttachLeftMouseDownImpl();
        AttachRightMouseDownImpl();
    }

    void SelectionManipulator::InstallLeftMouseDownCallback(const MouseActionCallback& onMouseDownCallback)
    {
        m_onLeftMouseDownCallback = onMouseDownCallback;
    }

    void SelectionManipulator::InstallLeftMouseUpCallback(const MouseActionCallback& onMouseUpCallback)
    {
        m_onLeftMouseUpCallback = onMouseUpCallback;
    }

    void SelectionManipulator::InstallRightMouseDownCallback(const MouseActionCallback& onMouseDownCallback)
    {
        m_onRightMouseDownCallback = onMouseDownCallback;
    }

    void SelectionManipulator::InstallRightMouseUpCallback(const MouseActionCallback& onMouseUpCallback)
    {
        m_onRightMouseUpCallback = onMouseUpCallback;
    }

    void SelectionManipulator::OnLeftMouseDownImpl(
        const ViewportInteraction::MouseInteraction& interaction, float /*rayIntersectionDistance*/)
    {
        if (m_onLeftMouseDownCallback)
        {
            m_onLeftMouseDownCallback(interaction);
        }
    }

    void SelectionManipulator::OnLeftMouseUpImpl(const ViewportInteraction::MouseInteraction& interaction)
    {
        if (MouseOver() && m_onLeftMouseUpCallback)
        {
            m_onLeftMouseUpCallback(interaction);
        }
    }

    void SelectionManipulator::OnRightMouseDownImpl(
        const ViewportInteraction::MouseInteraction& interaction, float /*rayIntersectionDistance*/)
    {
        if (m_onRightMouseDownCallback)
        {
            m_onRightMouseDownCallback(interaction);
        }
    }

    void SelectionManipulator::OnRightMouseUpImpl(const ViewportInteraction::MouseInteraction& interaction)
    {
        if (m_onRightMouseUpCallback)
        {
            m_onRightMouseUpCallback(interaction);
        }
    }

    void SelectionManipulator::Draw(
        const ManipulatorManagerState& managerState,
        AzFramework::DebugDisplayRequests& debugDisplay,
        const AzFramework::CameraState& cameraState,
        const ViewportInteraction::MouseInteraction& mouseInteraction)
    {
        for (auto& view : m_manipulatorViews)
        {
            view->Draw(
                GetManipulatorManagerId(), managerState, GetManipulatorId(),
                { TransformUniformScale(GetSpace()), GetNonUniformScale(), GetLocalPosition(), MouseOver() }, debugDisplay, cameraState,
                mouseInteraction);
        }
    }

    void SelectionManipulator::SetBoundsDirtyImpl()
    {
        for (auto& view : m_manipulatorViews)
        {
            view->SetBoundDirty(GetManipulatorManagerId());
        }
    }

    void SelectionManipulator::InvalidateImpl()
    {
        for (auto& view : m_manipulatorViews)
        {
            view->Invalidate(GetManipulatorManagerId());
        }
    }
} // namespace AzToolsFramework
