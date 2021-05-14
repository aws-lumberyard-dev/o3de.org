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

#include <AzToolsFramework/ComponentMode/EditorBaseComponentMode.h>
#include <AzToolsFramework/Manipulators/ScaleManipulators.h>

namespace AzToolsFramework
{
    namespace Components
    {
        //! Interface for handling non-uniform scale manipulator requests.
        //! Used by NonUniformScaleComponentMode.
        class NonUniformScaleManipulatorRequests
            : public AZ::EntityComponentBus
        {
        public:
            //! Gets the non-uniform scale.
            virtual AZ::Vector3 GetScale() const = 0;

            //! Sets the non-uniform scale.
            virtual void SetScale(const AZ::Vector3& scale) = 0;

        protected:
            ~NonUniformScaleManipulatorRequests() = default;
        };

        //! Type to inherit to implement NonUniformScaleManipulatorRequests.
        using NonUniformScaleManipulatorRequestBus = AZ::EBus<NonUniformScaleManipulatorRequests>;

        class NonUniformScaleComponentMode
            : public AzToolsFramework::ComponentModeFramework::EditorBaseComponentMode
        {
        public:
            AZ_CLASS_ALLOCATOR(NonUniformScaleComponentMode, AZ::SystemAllocator, 0)

            NonUniformScaleComponentMode(const AZ::EntityComponentIdPair& entityComponentIdPair, AZ::Uuid componentType);
            NonUniformScaleComponentMode(const NonUniformScaleComponentMode&) = delete;
            NonUniformScaleComponentMode& operator=(const NonUniformScaleComponentMode&) = delete;
            NonUniformScaleComponentMode(NonUniformScaleComponentMode&&) = delete;
            NonUniformScaleComponentMode& operator=(NonUniformScaleComponentMode&&) = delete;
            ~NonUniformScaleComponentMode();

            // EditorBaseComponentMode
            void Refresh() override;

        private:
            AZ::EntityComponentIdPair m_entityComponentIdPair;
            AZStd::unique_ptr<ScaleManipulators> m_manipulators;
            AZ::Vector3 m_initialScale;
        };
    } // namespace Components
} // namespace AzToolsFramework
