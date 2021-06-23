/*
 * Copyright (c) Contributors to the Open 3D Engine Project
 * 
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */
#pragma once

#include <AzCore/std/string/string.h>

namespace AZ
{
    class Entity;
}

namespace AutomatedLauncherTesting
{
    class SpawnDynamicSlice
    {
        public:
            static AZ::Entity* CreateSpawner(const AZStd::string& path, const AZStd::string& entityName);
    };

} // namespace AutomatedLauncherTesting
