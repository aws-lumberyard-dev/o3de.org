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

#include <QWindow>

#include <AzCore/EBus/EBus.h>
#include <AzCore/Memory/SystemAllocator.h>
#include <AzCore/RTTI/RTTI.h>

#include <AzCore/Outcome/Outcome.h>

class QMainWindow;

namespace ScriptCanvasDeveloper
{
    typedef AZ::Outcome<void, AZStd::string> ActionReport;

    /**
        EditorAutomationAction is the base class that all other actions will inherit from. Exposes a setup, and a tick function which returns whether or not the action is complete.
    */
    class EditorAutomationAction
    {
    public:
        AZ_CLASS_ALLOCATOR(EditorAutomationAction, AZ::SystemAllocator, 0);
        AZ_RTTI(EditorAutomationAction, "{133E2ECE-5829-432F-BA37-60C75C5CCC34}")
    
        EditorAutomationAction() = default;
        virtual ~EditorAutomationAction() = default;

        bool IsAtPreconditionLimit() const { return m_preconditionAttempts >= 10; }        
        
        virtual bool IsMissingPrecondition() { return false; }

        void ResetPreconditionAttempts() { m_preconditionAttempts = 0; }

        EditorAutomationAction* GenerationPreconditionActions()
        {
            ++m_preconditionAttempts;
            return GenerateMissingPreconditionAction();
        }

        void SignalActionBegin()
        {
            ResetPreconditionAttempts();
            SetupAction();
        }
        
        virtual bool Tick() = 0;

        virtual ActionReport GenerateReport() const { return AZ::Success(); }

    protected:

        virtual EditorAutomationAction* GenerateMissingPreconditionAction() { return nullptr; }
        virtual void SetupAction() {}

    private:

        int m_preconditionAttempts = 0;
    };
}
