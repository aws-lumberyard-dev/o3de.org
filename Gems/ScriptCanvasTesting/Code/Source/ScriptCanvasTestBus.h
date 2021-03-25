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

#include <AzCore/EBus/EBus.h>
#include <AzCore/std/string/string.h>
#include <AzCore/std/string/string_view.h>

#include <ScriptCanvas/Data/Data.h>

namespace AZ
{
    class ReflectContext;
}

namespace ScriptCanvasTesting
{
    class GlobalBusTraits : public AZ::EBusTraits
    {
    public:
        AZ_TYPE_INFO(GlobalBusTraits, "{DED849D7-CF17-408B-8D87-E31FC7D3CEC4}");

        static void Reflect(AZ::ReflectContext* context);

        virtual AZStd::string AppendSweet(AZStd::string_view) = 0;
        virtual int Increment(int value) = 0;
        virtual bool Not(bool value) = 0;
        virtual int Sum(int numberA, int numberB) = 0;
        virtual void Void(AZStd::string_view) = 0;

        virtual AZ::Event<>* GetZeroParamEvent() = 0;
        virtual AZ::Event<AZStd::vector<AZStd::string>&>* GetByReferenceEvent() = 0;
        virtual AZ::Event<int, bool, AZStd::string>* GetByValueEvent() = 0;
    };
    using GlobalEBus = AZ::EBus<GlobalBusTraits>;

    class LocalBusTraits : public AZ::EBusTraits
    {
    public:
        AZ_TYPE_INFO(LocalBusTraits, "{749B6949-CBBB-44D9-A57D-9973DC88E639}");

        static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::ById;

        using BusIdType = double;

        static void Reflect(AZ::ReflectContext* context);

        virtual AZStd::string AppendSweet(AZStd::string_view) = 0;
        virtual int Increment(int value) = 0;
        virtual bool Not(bool value) = 0;
        virtual void Void(AZStd::string_view) = 0;
    };
    using LocalEBus = AZ::EBus<LocalBusTraits>;

    class NativeHandlingOnlyBusTraits : public AZ::EBusTraits
    {
    public:
        AZ_TYPE_INFO(NativeHandlingOnlyBusTraits, "{5AED48A7-3E16-41F6-B1C0-4E1FBBA84F83}");

        static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::ById;

        using BusIdType = AZ::EntityId;

        static void Reflect(AZ::ReflectContext* context);

        virtual AZStd::string AppendSweet(AZStd::string_view) = 0;
        virtual int Increment(int value) = 0;
        virtual AZ::EntityId TwistTypeEntityId() = 0;
        virtual AZ::Vector3 TwistTypeVector3() = 0;
        virtual AZStd::tuple<AZ::EntityId, bool> TwistTupleEntityId() = 0;
        virtual AZStd::tuple<AZ::Vector3, bool> TwistTupleVector3() = 0;
        virtual bool Not(bool value) = 0;
        virtual void Void(AZStd::string_view) = 0;
    };
    using NativeHandlingOnlyEBus = AZ::EBus<NativeHandlingOnlyBusTraits>;

    class TestTupleMethods
    {
    public:
        AZ_TYPE_INFO(TestTupleMethods, "{E794CE93-7AC6-476E-BF10-B107A2E4D807}");

        static AZStd::tuple<ScriptCanvas::Data::Vector3Type, ScriptCanvas::Data::StringType, ScriptCanvas::Data::BooleanType> Three(
            const ScriptCanvas::Data::Vector3Type& v, const ScriptCanvas::Data::StringType& s, const ScriptCanvas::Data::BooleanType& b)
        {
            return AZStd::make_tuple(v, s, b);
        }

        static void Reflect(AZ::ReflectContext* context)
        {
            if (AZ::BehaviorContext* behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
            {
                behaviorContext->Class<TestTupleMethods>("TestTupleMethods")->Method("Three", &TestTupleMethods::Three);
            }
        }
    };

}
