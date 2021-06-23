/*
 * Copyright (c) Contributors to the Open 3D Engine Project
 * 
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <ScriptCanvas/Core/Node.h>
#include <Include/ScriptCanvas/Libraries/UnitTesting/AddFailure.generated.h>

namespace ScriptCanvas
{
    namespace Nodes
    {
        namespace UnitTesting
        {
            //! Used to represent that a unit test graph's operation has failed
            class AddFailure
                : public Node
            {
            public:

                SCRIPTCANVAS_NODE(AddFailure);

                void OnInputSignal(const SlotId& slotId) override;

            };
        }
    }
}
