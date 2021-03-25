/*
 * All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates, or
 * a third party where indicated.
 *
 * For complete copyright and license terms please see the LICENSE at the root of this
 * distribution (the "License"). All use of this software is governed by the License,
 * or, if provided, by the license below or the license accompanying this file. Do not
 * remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *
 */

#include <AzCore/RTTI/BehaviorContext.h>
#include <AzTest/AzTest.h>

#include <ScriptCanvas/AWSScriptBehaviorDynamoDB.h>
#include <TestFramework/AWSCoreFixture.h>

using namespace AWSCore;

class AWSScriptBehaviorDynamoDBNotificationBusHandlerMock
    : public AWSScriptBehaviorDynamoDBNotificationBusHandler
{
public:
    AWSScriptBehaviorDynamoDBNotificationBusHandlerMock()
    {
        AWSScriptBehaviorDynamoDBNotificationBus::Handler::BusConnect();
    }

    ~AWSScriptBehaviorDynamoDBNotificationBusHandlerMock()
    {
        AWSScriptBehaviorDynamoDBNotificationBus::Handler::BusDisconnect();
    }

    MOCK_METHOD1(OnGetItemSuccess, void(const DynamoDBAttributeValueMap&));
    MOCK_METHOD1(OnGetItemError, void(const AZStd::string&));
};

using AWSScriptBehaviorDynamoDBTest = UnitTest::ScopedAllocatorSetupFixture;

TEST_F(AWSScriptBehaviorDynamoDBTest, GetItemRaw_CallWithEmptyTableName_InvokeOnError)
{
    AWSScriptBehaviorDynamoDBNotificationBusHandlerMock dynamodbHandlerMock;
    EXPECT_CALL(dynamodbHandlerMock, OnGetItemError(::testing::_)).Times(1);
    DynamoDBAttributeValueMap dummyMap;
    AWSScriptBehaviorDynamoDB::GetItemRaw("", dummyMap, "dummyRegion");
}

TEST_F(AWSScriptBehaviorDynamoDBTest, GetItemRaw_CallWithEmptyKeyMap_InvokeOnError)
{
    AWSScriptBehaviorDynamoDBNotificationBusHandlerMock dynamodbHandlerMock;
    EXPECT_CALL(dynamodbHandlerMock, OnGetItemError(::testing::_)).Times(1);
    DynamoDBAttributeValueMap dummyMap;
    AWSScriptBehaviorDynamoDB::GetItemRaw("dummyTable", dummyMap, "dummyRegion");
}

TEST_F(AWSScriptBehaviorDynamoDBTest, GetItemRaw_CallWithInvalidKeyMap_InvokeOnError)
{
    AWSScriptBehaviorDynamoDBNotificationBusHandlerMock dynamodbHandlerMock;
    EXPECT_CALL(dynamodbHandlerMock, OnGetItemError(::testing::_)).Times(1);
    DynamoDBAttributeValueMap dummyMap;
    dummyMap.emplace("dummyKey", "{invalidJsonFormat}");
    AWSScriptBehaviorDynamoDB::GetItemRaw("dummyTable", dummyMap, "dummyRegion");
}

TEST_F(AWSScriptBehaviorDynamoDBTest, GetItemRaw_CallWithEmptyRegionName_InvokeOnError)
{
    AWSScriptBehaviorDynamoDBNotificationBusHandlerMock dynamodbHandlerMock;
    EXPECT_CALL(dynamodbHandlerMock, OnGetItemError(::testing::_)).Times(1);
    DynamoDBAttributeValueMap dummyMap;
    dummyMap.emplace("dummyKey", "{}");
    AWSScriptBehaviorDynamoDB::GetItemRaw("dummyTable", dummyMap, "");
}

TEST_F(AWSScriptBehaviorDynamoDBTest, GetItem_NoTableNameInResourceMappingFound_InvokeOnError)
{
    AWSScriptBehaviorDynamoDBNotificationBusHandlerMock dynamodbHandlerMock;
    EXPECT_CALL(dynamodbHandlerMock, OnGetItemError(::testing::_)).Times(1);
    DynamoDBAttributeValueMap dummyMap;
    AWSScriptBehaviorDynamoDB::GetItem("dummyTable", dummyMap);
}

TEST_F(AWSScriptBehaviorDynamoDBTest, OnSuccessOnError_Call_GetExpectedNumOfInvoke)
{
    AWSScriptBehaviorDynamoDBNotificationBusHandlerMock dynamodbHandlerMock;
    EXPECT_CALL(dynamodbHandlerMock, OnGetItemSuccess(::testing::_)).Times(1);
    EXPECT_CALL(dynamodbHandlerMock, OnGetItemError(::testing::_)).Times(1);
    DynamoDBAttributeValueMap dummyMap;
    AWSScriptBehaviorDynamoDBNotificationBus::Broadcast(&AWSScriptBehaviorDynamoDBNotificationBus::Events::OnGetItemSuccess, dummyMap);
    AWSScriptBehaviorDynamoDBNotificationBus::Broadcast(&AWSScriptBehaviorDynamoDBNotificationBus::Events::OnGetItemError, "dummy success message");
}
