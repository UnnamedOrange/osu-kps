/**
 * @file TestSharedComPtr.cpp
 * @author UnnamedOrange
 * @brief Test `SharedComPtr`.
 * @version 1.0.0
 * @date 2023-08-02
 *
 * @copyright Copyright (c) UnnamedOrange. Licensed under the MIT License.
 * See the LICENSE file in the repository root for full license text.
 */

#include <Unknwnbase.h>
#include <d2d1_1.h>
#pragma comment(lib, "d2d1.lib")

#include <gtest/gtest.h>

#include <utils/d2d/SharedComPtr.hpp>

using namespace orange;

TEST(TestSharedComPtr, test_default_constructor) {
    constexpr SharedComPtr<IUnknown> p;
    EXPECT_TRUE(p.empty());
}
TEST(TestSharedComPtr, test_copy_constructor) {
    SharedComPtr<IUnknown> p1;
    if (FAILED(D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, __uuidof(ID2D1Factory1),
                                 reinterpret_cast<void**>(p1.reset_and_get_address())))) {
        return; // Skip.
    }
    auto p2 = p1;
    EXPECT_FALSE(p1.empty());
    EXPECT_EQ(p1.get(), p2.get());
}
TEST(TestSharedComPtr, test_copy_assignment) {
    SharedComPtr<IUnknown> p1;
    if (FAILED(D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, __uuidof(ID2D1Factory1),
                                 reinterpret_cast<void**>(p1.reset_and_get_address())))) {
        return; // Skip.
    }
    SharedComPtr<IUnknown> p2;
    p2 = p1;
    EXPECT_FALSE(p1.empty());
    EXPECT_EQ(p1.get(), p2.get());
}
TEST(TestSharedComPtr, test_move_constructor) {
    SharedComPtr<IUnknown> p1;
    if (FAILED(D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, __uuidof(ID2D1Factory1),
                                 reinterpret_cast<void**>(p1.reset_and_get_address())))) {
        return; // Skip.
    }
    auto p2 = std::move(p1);
    EXPECT_TRUE(p1.empty());
    EXPECT_FALSE(p2.empty());
}
TEST(TestSharedComPtr, test_move_assignment) {
    SharedComPtr<IUnknown> p1;
    if (FAILED(D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, __uuidof(ID2D1Factory1),
                                 reinterpret_cast<void**>(p1.reset_and_get_address())))) {
        return; // Skip.
    }
    SharedComPtr<IUnknown> p2;
    p2 = std::move(p1);
    EXPECT_TRUE(p1.empty());
    EXPECT_FALSE(p2.empty());
}
