/**
 * @file WindowsResource.cpp
 * @author UnnamedOrange
 * @brief Alternative API for deprecated `resource_loader`.
 * @version 1.0.0
 * @date 2023-08-02
 *
 * @copyright Copyright (c) UnnamedOrange. Licensed under the MIT License.
 * See the LICENSE file in the repository root for full license text.
 */

#include "WindowsResource.h"

#include <utility>

using namespace orange;

using Self = WindowsResource;

Self Self::try_from(const wchar_t* resource_name, const wchar_t* type_name) noexcept {
    HINSTANCE hInstance = GetModuleHandleW(nullptr);
    HRSRC hResInfo = FindResourceW(hInstance, resource_name, type_name);
    if (!hResInfo) {
        return {};
    }
    auto size = SizeofResource(hInstance, hResInfo);
    if (!size) {
        return {};
    }
    HGLOBAL hResource = LoadResource(hInstance, hResInfo);
    if (!hResource) {
        return {};
    }

    void* p = LockResource(hResource);
    return {hResource, p, static_cast<size_t>(size)};
}

Self::WindowsResource(Self&& other) noexcept : WindowsResource() {
    swap(*this, other);
}
Self& Self::operator=(Self other) noexcept {
    swap(*this, other);
    return *this;
}

Self::~WindowsResource() {
    reset();
}

Self::WindowsResource(HGLOBAL hResource, const void* locked_address, size_t resource_size) noexcept
    : hResource{hResource}, locked_address{locked_address}, resource_size{resource_size} {}

bool Self::empty() const noexcept {
    return !locked_address;
}
size_t Self::size() const noexcept {
    return resource_size;
}
void Self::reset() noexcept {
    if (hResource) {
        FreeResource(hResource);
    }
    hResource = nullptr;
    locked_address = nullptr;
    resource_size = 0;
}
std::span<const std::byte> Self::to_span() const noexcept {
    if (empty()) {
        return {};
    }
    return {reinterpret_cast<const std::byte*>(locked_address), resource_size};
}
std::string_view Self::to_string_view() const noexcept {
    if (empty()) {
        return {};
    }
    return {reinterpret_cast<const char*>(locked_address), resource_size};
}
std::u8string_view Self::to_u8string_view() const noexcept {
    if (empty()) {
        return {};
    }
    return {reinterpret_cast<const char8_t*>(locked_address), resource_size};
}
std::vector<std::byte> Self::to_vector() const {
    if (empty()) {
        return {};
    }
    return {reinterpret_cast<const std::byte*>(locked_address),
            reinterpret_cast<const std::byte*>(locked_address) + resource_size};
}
std::string Self::to_string() const {
    if (empty()) {
        return {};
    }
    return {reinterpret_cast<const char*>(locked_address),
            reinterpret_cast<const char*>(locked_address) + resource_size};
}

std::u8string Self::to_u8string() const {
    if (empty()) {
        return {};
    }
    return {reinterpret_cast<const char8_t*>(locked_address),
            reinterpret_cast<const char8_t*>(locked_address) + resource_size};
}
