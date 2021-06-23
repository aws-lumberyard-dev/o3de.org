/*
 * Copyright (c) Contributors to the Open 3D Engine Project
 * 
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AzTest/AzTest.h>

class SceneUITestEnvironment
    : public AZ::Test::ITestEnvironment
{
public:
    virtual ~SceneUITestEnvironment() {}

protected:
    void SetupEnvironment() override
    {
        
    }

    void TeardownEnvironment() override
    {
        
    }
};

AZ_UNIT_TEST_HOOK(new SceneUITestEnvironment);
