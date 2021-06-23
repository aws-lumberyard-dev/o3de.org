/*
 * Copyright (c) Contributors to the Open 3D Engine Project
 * 
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */
#pragma once

#include <LyShine/IDraw2d.h>
#include <AzCore/Serialization/SerializeContext.h>

namespace LyShineExamplesSerialize
{
    //! Define the UI types for the AZ Serialize system
    void ReflectTypes(AZ::ReflectContext* context);
}
