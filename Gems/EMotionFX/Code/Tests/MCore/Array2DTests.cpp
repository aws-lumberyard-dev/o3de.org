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

#include <gtest/gtest.h>
#include <AzCore/UnitTest/TestTypes.h>

#include <MCore/Source/Array2D.h>

namespace MCore
{
    using Array2DFixture = UnitTest::ScopedAllocatorSetupFixture;

    // A non-owning array view, a simple C++20 std::span stand-in
    template<typename T>
    class span
    {
    public:
        using value_type = T;
        using iterator = T*;
        using const_iterator = const T*;

        span() = default;
        span(T* begin, size_t size)
            : m_begin(begin)
            , m_size(size)
        {}

        iterator begin() { return m_begin; }
        const_iterator begin() const { return m_begin; }
        iterator end() { return m_begin + m_size; }
        const_iterator end() const { return m_begin + m_size; }
        size_t size() const { return m_size; }

    private:
        T* m_begin;
        size_t m_size;
    };

    TEST_F(Array2DFixture, Array2D_Swap)
    {
        Array2D<int> ints(2);
        ints.Add(0, 5);
        ints.Add(0, 6);
        ints.Add(0, 7);

        ints.Add(1, 11);
        ints.Add(1, 21);
        ints.Add(1, 31);
        ints.Add(1, 41);

        EXPECT_THAT(
            span<int>(ints.GetElements(0), ints.GetNumElements(0)),
            ::testing::Pointwise(::testing::Eq(), AZStd::vector<int> {5, 6, 7})
        );
        EXPECT_THAT(
            span<int>(ints.GetElements(1), ints.GetNumElements(1)),
            ::testing::Pointwise(::testing::Eq(), AZStd::vector<int> {11, 21, 31, 41})
        );

        ints.Swap(0, 1);

        EXPECT_THAT(
            span<int>(ints.GetElements(0), ints.GetNumElements(0)),
            ::testing::Pointwise(::testing::Eq(), AZStd::vector<int> {11, 21, 31, 41})
        );
        EXPECT_THAT(
            span<int>(ints.GetElements(1), ints.GetNumElements(1)),
            ::testing::Pointwise(::testing::Eq(), AZStd::vector<int> {5, 6, 7})
        );

        ints.Swap(1, 0);

        EXPECT_THAT(
            span<int>(ints.GetElements(0), ints.GetNumElements(0)),
            ::testing::Pointwise(::testing::Eq(), AZStd::vector<int> {5, 6, 7})
        );
        EXPECT_THAT(
            span<int>(ints.GetElements(1), ints.GetNumElements(1)),
            ::testing::Pointwise(::testing::Eq(), AZStd::vector<int> {11, 21, 31, 41})
        );

    }
}
