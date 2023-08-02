/**
 * @file ConvertCode.hpp
 * @author UnnamedOrange
 * @brief Alternative API for deprecated `code_conv`.
 * @version 1.0.0
 * @date 2023-08-02
 *
 * @copyright Copyright (c) UnnamedOrange. Licensed under the MIT License.
 * See the LICENSE file in the repository root for full license text.
 */

#pragma once

#include <filesystem>
#include <string>
#include <string_view>

namespace orange {
    class ConvertCode final {
    private:
        using path = std::filesystem::path;

    public:
        template <typename T>
        [[nodiscard]] static auto to_string(const T& src) {
            return path(src).string();
        }

        template <typename T>
        [[nodiscard]] static auto to_wstring(const T& src) {
            return path(src).wstring();
        }

        template <typename T>
        [[nodiscard]] static auto to_u8string(const T& src) {
            return path(src).u8string();
        }

        template <typename T>
        [[nodiscard]] static auto to_u16string(const T& src) {
            return path(src).u16string();
        }

        template <typename T>
        [[nodiscard]] static auto to_u32string(const T& src) {
            return path(src).u32string();
        }
    };
} // namespace orange
