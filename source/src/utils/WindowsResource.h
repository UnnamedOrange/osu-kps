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
        Self& operator=(const Self&) = delete;
        WindowsResource(Self&& other) noexcept;
        Self& operator=(Self&& other) noexcept;

        ~WindowsResource();

    private:
        WindowsResource(HGLOBAL hResource, const void* locked_address, size_t resource_size) noexcept;

    public:
        bool empty() const noexcept;
        size_t size() const noexcept;
        void reset() noexcept;
        std::span<const std::byte> to_span() const noexcept;
        std::vector<std::byte> to_vector() const noexcept;
    };
} // namespace orange
