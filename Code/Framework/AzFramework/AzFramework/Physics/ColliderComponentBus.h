/*
 * Copyright (c) Contributors to the Open 3D Engine Project
 * 
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */
#pragma once

#include <AzCore/Component/ComponentBus.h>

namespace Physics
{
    /// Events dispatched by a ColliderComponent.
    /// A ColliderComponent describes the shape of an entity to the physics system.
    class ColliderComponentEvents
        : public AZ::ComponentBus
    {
    public:
        virtual void OnColliderChanged() {}
    };
    using ColliderComponentEventBus = AZ::EBus<ColliderComponentEvents>;
} // namespace Physics
