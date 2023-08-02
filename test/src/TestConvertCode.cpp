/**
 * @file TestConvertCode.cpp
 * @author UnnamedOrange
 * @brief Test `ConvertCode`.
 * @version 1.0.0
 * @date 2023-08-02
 *
 * @copyright Copyright (c) UnnamedOrange. Licensed under the MIT License.
 * See the LICENSE file in the repository root for full license text.
 */

#include <string>

#include <gtest/gtest.h>

#include <utils/ConvertCode.hpp>

using namespace orange;

// `string` behaves differently on different platforms,
// so it should not be used.
// Printing `u8string` is not well supported, so it is not tested.
constexpr const wchar_t* literal_wstring = L"你好，こんにちは！";
constexpr const char8_t* literal_u8string = u8"你好，こんにちは！";
constexpr const char16_t* literal_u16string = u"你好，こんにちは！";
constexpr const char32_t* literal_u32string = U"你好，こんにちは！";
static const std::wstring object_wstring = L"你好，こんにちは！";
static const std::u8string object_u8string = u8"你好，こんにちは！";
static const std::u16string object_u16string = u"你好，こんにちは！";
static const std::u32string object_u32string = U"你好，こんにちは！";

TEST(TestConvertCode, test_to_wstring) {
    ASSERT_EQ(ConvertCode::to_wstring(literal_wstring), object_wstring);
    ASSERT_EQ(ConvertCode::to_wstring(literal_u8string), object_wstring);
    ASSERT_EQ(ConvertCode::to_wstring(literal_u16string), object_wstring);
    ASSERT_EQ(ConvertCode::to_wstring(literal_u32string), object_wstring);
    ASSERT_EQ(ConvertCode::to_wstring(object_wstring), literal_wstring);
    ASSERT_EQ(ConvertCode::to_wstring(object_u8string), literal_wstring);
    ASSERT_EQ(ConvertCode::to_wstring(object_u16string), literal_wstring);
    ASSERT_EQ(ConvertCode::to_wstring(object_u32string), literal_wstring);
}
TEST(TestConvertCode, test_to_u16string) {
    ASSERT_EQ(ConvertCode::to_u16string(literal_wstring), object_u16string);
    ASSERT_EQ(ConvertCode::to_u16string(literal_u8string), object_u16string);
    ASSERT_EQ(ConvertCode::to_u16string(literal_u16string), object_u16string);
    ASSERT_EQ(ConvertCode::to_u16string(literal_u32string), object_u16string);
    ASSERT_EQ(ConvertCode::to_u16string(object_wstring), literal_u16string);
    ASSERT_EQ(ConvertCode::to_u16string(object_u8string), literal_u16string);
    ASSERT_EQ(ConvertCode::to_u16string(object_u16string), literal_u16string);
    ASSERT_EQ(ConvertCode::to_u16string(object_u32string), literal_u16string);
}
TEST(TestConvertCode, test_to_u32string) {
    ASSERT_EQ(ConvertCode::to_u32string(literal_wstring), object_u32string);
    ASSERT_EQ(ConvertCode::to_u32string(literal_u8string), object_u32string);
    ASSERT_EQ(ConvertCode::to_u32string(literal_u16string), object_u32string);
    ASSERT_EQ(ConvertCode::to_u32string(literal_u32string), object_u32string);
    ASSERT_EQ(ConvertCode::to_u32string(object_wstring), literal_u32string);
    ASSERT_EQ(ConvertCode::to_u32string(object_u8string), literal_u32string);
    ASSERT_EQ(ConvertCode::to_u32string(object_u16string), literal_u32string);
    ASSERT_EQ(ConvertCode::to_u32string(object_u32string), literal_u32string);
}
