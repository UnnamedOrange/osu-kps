// Copyright (c) UnnamedOrange. Licensed under the MIT Licence.
// See the LICENSE file in the repository root for full licence text.

#pragma once

#include <cstdint>
#include <memory>
#include <utility>

#include <memory-reader/all.h>

namespace orange {
    class MemoryReaderOsu final {
        struct Hub;

    private:
        std::unique_ptr<Hub> pimpl;

    public:
        MemoryReaderOsu() noexcept;
        ~MemoryReaderOsu();

    public:
        std::optional<std::vector<std::pair<int, bool>>> get_mania_keys() noexcept;
        std::optional<bool> get_is_autoplay() noexcept;
    };
} // namespace orange
