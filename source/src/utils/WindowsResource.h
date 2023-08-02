/**
 * @file WindowsResource.h
 * @author UnnamedOrange
 * @brief Alternative API for deprecated `resource_loader`.
 * @version 1.0.0
 * @date 2023-08-02
 *
 * @copyright Copyright (c) UnnamedOrange. Licensed under the MIT License.
 * See the LICENSE file in the repository root for full license text.
 */

#pragma once

#include <cstddef>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include <Windows.h>

namespace orange {
    class WindowsResource final {
        using Self = WindowsResource;

    private:
        HGLOBAL hResource{};
        const void* locked_address{};
        size_t resource_size{};

    public:
        /**
         * @brief Try constructing a @ref WindowsResource object
         * from a resource entry.
         *
         * @param resource_name See `FindResource`.
         * @param type_name See `FindResource`.
         * @return Self If the resource is found, a valid object is returned.
         * Otherwise, an empty object is returned.
         */
        static Self try_from(const wchar_t* resource_name, const wchar_t* type_name) noexcept;

    public:
        WindowsResource() noexcept = default;
        WindowsResource(const Self&) = delete;
        WindowsResource(Self&& other) noexcept;
        Self& operator=(Self other) noexcept;

        ~WindowsResource();

        friend void swap(Self& a, Self& b) noexcept {
            using std::swap;
            swap(a.hResource, b.hResource);
            swap(a.locked_address, b.locked_address);
            swap(a.resource_size, b.resource_size);
        }

    private:
        WindowsResource(HGLOBAL hResource, const void* locked_address, size_t resource_size) noexcept;

    public:
        bool empty() const noexcept;
        size_t size() const noexcept;
        void reset() noexcept;
        std::span<const std::byte> to_span() const noexcept;
        std::string_view to_string_view() const noexcept;
        std::u8string_view to_u8string_view() const noexcept;
        std::vector<std::byte> to_vector() const;
        std::string to_string() const;
        std::u8string to_u8string() const;
    };
} // namespace orange
