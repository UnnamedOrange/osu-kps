/**
 * @file TestWindowsResource.cpp
 * @author UnnamedOrange
 * @brief Test `WindowsResource`.
 * @version 1.0.0
 * @date 2023-08-02
 *
 * @copyright Copyright (c) UnnamedOrange. Licensed under the MIT License.
 * See the LICENSE file in the repository root for full license text.
 */

#include <string>

#include <Windows.h>

#include <gtest/gtest.h>

#include <resource.h>
#include <utils/WindowsResource.hpp>

using namespace orange;

TEST(TestWindowsResource, test_default_constructor) {
    WindowsResource resource;
    EXPECT_TRUE(resource.empty());
    EXPECT_EQ(resource.size(), 0);
    EXPECT_TRUE(resource.to_span().empty());
    EXPECT_TRUE(resource.to_vector().empty());
}
TEST(TestWindowsResource, test_wrong_resource_name) {
    auto resource = WindowsResource::try_from(L"114", L"514");
    EXPECT_TRUE(resource.empty());
    EXPECT_EQ(resource.size(), 0);
    EXPECT_TRUE(resource.to_span().empty());
    EXPECT_TRUE(resource.to_vector().empty());
}
TEST(TestWindowsResource, test_real_resource) {
    auto resource = WindowsResource::try_from(MAKEINTRESOURCEW(IDR_JSON_DEFAULT_CFG), L"JSON");
    EXPECT_FALSE(resource.empty());
    EXPECT_GT(resource.size(), 0);
    EXPECT_FALSE(resource.to_span().empty());
    EXPECT_FALSE(resource.to_vector().empty());
}
TEST(TestWindowsResource, test_movement_assignment) {
    auto resource = WindowsResource::try_from(MAKEINTRESOURCEW(IDR_JSON_DEFAULT_CFG), L"JSON");
    if constexpr ([[maybe_unused]] auto _ = std::move(resource); true)
        ;
    EXPECT_TRUE(resource.empty());
    EXPECT_EQ(resource.size(), 0);
    EXPECT_TRUE(resource.to_span().empty());
    EXPECT_TRUE(resource.to_vector().empty());
}
