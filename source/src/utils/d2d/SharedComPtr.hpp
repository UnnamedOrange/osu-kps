/**
 * @file SharedComPtr.hpp
 * @author UnnamedOrange
 * @brief Pointer for COM objects.
 * @version 1.0.0
 * @date 2023-08-02
 *
 * @copyright Copyright (c) UnnamedOrange. Licensed under the MIT License.
 * See the LICENSE file in the repository root for full license text.
 */

#pragma once

#include <concepts>
#include <utility>

#include <Unknwnbase.h>

namespace orange {
    template <typename T>
    concept TCom = requires(T* obj) { std::derived_from<T, IUnknown>; };

    template <TCom ComType>
    class SharedComPtr final {
        using Self = SharedComPtr<ComType>;

    private:
        ComType* ptr{};

    public:
        constexpr SharedComPtr() noexcept = default;
        SharedComPtr(const Self& other) : SharedComPtr() {
            auto new_ptr = other.ptr;
            if (new_ptr) {
                new_ptr->AddRef(); // Assume an exception may be thrown.
            }
            ptr = new_ptr;
        }
        SharedComPtr(Self&& other) noexcept : SharedComPtr() {
            swap(*this, other);
        }
        Self& operator=(Self other) noexcept {
            swap(*this, other);
            return *this;
        }

        constexpr ~SharedComPtr() {
            reset();
        }

        friend void swap(Self& a, Self& b) noexcept {
            using std::swap;
            swap(a.ptr, b.ptr);
        }

    public:
        constexpr bool empty() const noexcept {
            return !ptr;
        }
        constexpr ComType* get() const noexcept {
            return ptr;
        }
        ComType** reset_and_get_address() noexcept {
            reset();
            return &ptr;
        }
        constexpr void reset() noexcept {
            if (ptr) {
                ptr->Release(); // Assume an exception may be thrown.
                ptr = nullptr;
            }
        }
        ComType* operator->() const noexcept {
            return ptr;
        }
        explicit constexpr operator bool() const noexcept {
            return ptr;
        }
        constexpr operator ComType*() const noexcept {
            return ptr;
        }
    };
} // namespace orange
