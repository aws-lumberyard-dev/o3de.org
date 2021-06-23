/*
 * Copyright (c) Contributors to the Open 3D Engine Project
 * 
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AzCore/std/string/string_view.h>

namespace O3DELauncher
{
    bool WaitForAssetProcessorConnect()
    {
        // Dedicated server does not depend on Asset Processor and assumes that assets are already prepared.
        return false;
    }

    bool IsDedicatedServer()
    {
        return true;
    }

    const char* GetLogFilename()
    {
        return "@log@/Server.log";
    }
}
