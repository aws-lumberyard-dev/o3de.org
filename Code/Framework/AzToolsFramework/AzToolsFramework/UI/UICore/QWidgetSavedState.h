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

#ifndef QWIDGETSAVEDSTATE_H
#define QWIDGETSAVEDSTATE_H

#include <AzCore/base.h>
#include <AzCore/RTTI/RTTI.h>
#include <AzCore/Memory/SystemAllocator.h>
#include <AzCore/std/containers/vector.h>
#include <AzCore/UserSettings/UserSettings.h>

class QWidget;

#pragma once

namespace AzToolsFramework
{
    // the QWidgetSavedState is a nice base class for you to derive your own dialog or other widget's saved state from
    // it takes care of the geometry save and restore, using CaptureGeometry(widget) and RestoreGeometry(widget).
    class QWidgetSavedState : public AZ::UserSettings
    {
    public:
        AZ_RTTI(QWidgetSavedState, "{ED8EEF58-C577-4FCC-AE84-66FF63A1BDEB}", AZ::UserSettings);
        AZ_CLASS_ALLOCATOR(QWidgetSavedState, AZ::SystemAllocator, 0);
        
        virtual ~QWidgetSavedState() {}

        virtual void CaptureGeometry(QWidget *pWidget);
        virtual void RestoreGeometry(QWidget *pWidget);

        static void Reflect(AZ::ReflectContext* context);
    private:
        AZStd::vector<AZ::u8> m_windowGeometry;
    };
}

#endif
