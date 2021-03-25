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

#include <AzCore/RTTI/TypeSafeIntegral.h>
#include <AzCore/Utils/TypeHash.h>
#include <AzCore/std/containers/variant.h>
#include <AzCore/std/containers/vector.h>
#include <AzCore/std/string/string.h>
#include <AzCore/std/utils.h>

namespace AZ
{
    class ReflectContext;
}

namespace AzFramework
{
    AZ_TYPE_SAFE_INTEGRAL(SpawnableMetaDataArraySize, uint64_t);
    using SpawnableMetaDataArrayIndex = SpawnableMetaDataArraySize;

    //! Simple meta data that can be stored along with a spawnable.
    //! This class is designed to be read-only and is expected to be serialized. AzToolsFramework provides a class for
    //! constructing meta data.
    class SpawnableMetaData final
    {
    public:
        static inline constexpr uint64_t ArrayKeyRoot = 146223222818353; // Random prime number

        using TableKey = AZ::HashValue64;
        using TableValue = AZStd::variant<bool, uint64_t, int64_t, double, AZStd::string, SpawnableMetaDataArraySize>;
        using TableEntry = AZStd::pair<TableKey, TableValue>;
        using Table = AZStd::vector<TableEntry>;

        enum class ValueType
        {
            Unavailable,
            Boolean,
            UnsignedInteger,
            SignedInteger,
            FloatingPoint,
            String,
            ArraySize
        };

        AZ_TYPE_INFO(AZ::SpawnableMetaData, "{3832FA08-B10B-49AF-A81E-8E2FC1FF98B1}");

        SpawnableMetaData() = default;
        // This constructor exists only so externally constructed tables can be provided by tools.
        explicit SpawnableMetaData(Table table);

        bool Get(AZStd::string_view key, bool& value) const;
        bool Get(AZStd::string_view key, uint64_t& value) const;
        bool Get(AZStd::string_view key, int64_t& value) const;
        bool Get(AZStd::string_view key, double& value) const;
        bool Get(AZStd::string_view key, AZStd::string_view& value) const;
        bool Get(AZStd::string_view key, SpawnableMetaDataArraySize& value) const;

        bool Get(AZStd::string_view arrayKey, uint64_t index, bool& value) const;
        bool Get(AZStd::string_view arrayKey, uint64_t index, uint64_t& value) const;
        bool Get(AZStd::string_view arrayKey, uint64_t index, int64_t& value) const;
        bool Get(AZStd::string_view arrayKey, uint64_t index, double& value) const;
        bool Get(AZStd::string_view arrayKey, uint64_t index, AZStd::string_view& value) const;

        bool Get(AZStd::string_view arrayKey, SpawnableMetaDataArrayIndex index, bool& value) const;
        bool Get(AZStd::string_view arrayKey, SpawnableMetaDataArrayIndex index, uint64_t& value) const;
        bool Get(AZStd::string_view arrayKey, SpawnableMetaDataArrayIndex index, int64_t& value) const;
        bool Get(AZStd::string_view arrayKey, SpawnableMetaDataArrayIndex index, double& value) const;
        bool Get(AZStd::string_view arrayKey, SpawnableMetaDataArrayIndex index, AZStd::string_view& value) const;

        ValueType GetType(AZStd::string_view key) const;
        ValueType GetType(AZStd::string_view arrayKey, uint64_t index) const;
        ValueType GetType(AZStd::string_view arrayKey, SpawnableMetaDataArrayIndex index) const;

        static void Reflect(AZ::ReflectContext* context);

    private:
        TableKey GetKeyHash(AZStd::string_view key) const;
        TableKey GetKeyHash(AZStd::string_view arrayKey, uint64_t index) const;
        TableKey GetKeyHash(AZStd::string_view arrayKey, SpawnableMetaDataArrayIndex index) const;

        template<typename T>
        bool GetGeneric(AZ::HashValue64 key, T& value) const;

        ValueType GetTypeGeneric(AZ::HashValue64 key) const;

        AZStd::vector<TableKey> m_keys;
        AZStd::vector<TableValue> m_values;
    };
} // namespace AzFramework

AZ_TYPE_SAFE_INTEGRAL_SERIALIZEBINDING(AzFramework::SpawnableMetaDataArraySize);
