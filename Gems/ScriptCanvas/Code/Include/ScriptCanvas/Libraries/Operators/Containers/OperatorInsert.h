/*
 * Copyright (c) Contributors to the Open 3D Engine Project
 * 
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <ScriptCanvas/Libraries/Operators/Operator.h>
#include <Include/ScriptCanvas/Libraries/Operators/Containers/OperatorInsert.generated.h>

namespace ScriptCanvas
{
    namespace Nodes
    {
        namespace Operators
        {
            //! Deprecated: see MethodOverloaded for "Insert"
            class OperatorInsert : public OperatorBase
            {
            public:

                SCRIPTCANVAS_NODE(OperatorInsert);


                OperatorInsert()
                    : OperatorBase(DefaultContainerManipulationOperatorConfiguration())
                {
                }

            protected:

                void ConfigureContracts(SourceType sourceType, AZStd::vector<ContractDescriptor>& contractDescs) override;
                void OnSourceTypeChanged() override;
                void OnInputSignal(const SlotId& slotId) override;
                void InvokeOperator();

            };
        }
    }
}
