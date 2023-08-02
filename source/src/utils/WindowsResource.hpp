/**
 * @file WindowsResource.hpp
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
        static Self try_from(const wchar_t* resource_name, const wchar_t* type_name) noexcept {
            HINSTANCE hInstance = GetModuleHandleW(nullptr);
            HRSRC hResInfo = FindResourceW(hInstance, resource_name, type_name);
            if (!hResInfo)
                return {};
            auto size = SizeofResource(hInstance, hResInfo);
            if (!size)
                return {};
            HGLOBAL hResource = LoadResource(hInstance, hResInfo);
            if (!hResource)
                return {};

            void* p = LockResource(hResource);
            return {hResource, p, static_cast<size_t>(size)};
        }

    public:
        WindowsResource() noexcept = default;
        WindowsResource(const Self&) = delete;
        Self& operator=(const Self&) = delete;
        WindowsResource(Self&& other) noexcept
            : hResource(other.hResource), locked_address(other.locked_address), resource_size(other.resource_size) {
            other.hResource = nullptr;
            other.locked_address = nullptr;
            other.resource_size = 0;
        }
        Self& operator=(Self&& other) noexcept {
            if (this != &other) {
                reset();

                hResource = other.hResource;
                locked_address = other.locked_address;
                resource_size = other.resource_size;

                other.hResource = nullptr;
                other.locked_address = nullptr;
                other.resource_size = 0;
            }
            return *this;
        }

        ~WindowsResource() {
            reset();
        }

    private:
        WindowsResource(HGLOBAL hResource, const void* locked_address, size_t resource_size) noexcept
            : hResource{hResource}, locked_address{locked_address}, resource_size{resource_size} {}

    public:
        bool empty() const noexcept {
            return !locked_address;
        }
        size_t size() const noexcept {
            return resource_size;
        }
        void reset() noexcept {
            if (hResource) {
                FreeResource(hResource);
            }
            hResource = nullptr;
            locked_address = nullptr;
            resource_size = 0;
        }
        std::span<const std::byte> to_span() const noexcept {
            if (empty())
                return {};
            return {reinterpret_cast<const std::byte*>(locked_address), resource_size};
        }
        std::vector<std::byte> to_vector() const noexcept {
            if (empty())
                return {};
            return {reinterpret_cast<const std::byte*>(locked_address),
                    reinterpret_cast<const std::byte*>(locked_address) + resource_size};
        }
    };
} // namespace orange
