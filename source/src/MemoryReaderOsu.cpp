// Copyright (c) UnnamedOrange. Licensed under the MIT Licence.
// See the LICENSE file in the repository root for full licence text.

#include "MemoryReaderOsu.h"

#include <ranges>
#include <span>

using namespace orange;
using namespace orange::memory_reader;

using Self = MemoryReaderOsu;

// Note osu!.exe is 32-bit.
template <std::intptr_t... R>
using POffsets = PtrOffsets<PtrWidth::IS_32, R...>;
template <typename T, std::intptr_t... R>
using VOffsets = ValueOffsets<PtrWidth::IS_32, T, R...>;

struct Self::Hub final {
    SingleProcessDaemon process{"osu!.exe"};

    Signature<"7D 15 A1 ?? ?? ?? ?? 85 C0"> //
        sig_rulesets;

    // General C# array size.
    VOffsets<std::uint32_t, 0xC> //
        offsets_array_size;
    // General C# array base.
    POffsets<0x4> //
        offsets_array_base;

    // Keys in osu!mania.
    POffsets<-0xB, 0x4, 0xD0, 0x94, 0x4, 0x44> //
        offsets_mania_keys_array_object;
    VOffsets<std::int32_t, 0x30> //
        offsets_mania_key_code;
    VOffsets<bool, 0x3B> //
        offsets_mania_key_is_down;

    // Autoplay in osu!mania.
    VOffsets<bool, -0xB, 0x4, 0x48, 0x0, 0x30, 0xC, 0x11C, 0x0> //
        offsets_mania_is_autoplay;
};

Self::MemoryReaderOsu() noexcept : pimpl{std::make_unique<Hub>()} {}
Self::~MemoryReaderOsu() = default; // Required for pimpl.

std::optional<std::vector<std::pair<int, bool>>> Self::get_mania_keys() noexcept {
    const auto rulesets = pimpl->sig_rulesets.scan(pimpl->process);
    if (!rulesets) {
        return std::nullopt;
    }

    const auto array_object_base = pimpl->offsets_mania_keys_array_object.read(pimpl->process, *rulesets);
    if (!array_object_base) {
        return std::nullopt;
    }

    const auto array_size = pimpl->offsets_array_size.read(pimpl->process, *array_object_base);
    if (!array_size || !*array_size || *array_size > 20u) {
        return std::nullopt;
    }

    const auto array_base = pimpl->offsets_array_base.read(pimpl->process, *array_object_base);
    if (!array_object_base) {
        return std::nullopt;
    }

    const auto key_addrs = pimpl->process.read_bytes(*array_base + 8, *array_size * sizeof(uint32_t));
    if (key_addrs.empty()) {
        return std::nullopt;
    }

    const auto key_addrs_span = std::span(reinterpret_cast<const std::uint32_t*>(key_addrs.data()), *array_size);

    std::vector<std::pair<int, bool>> ret;
    for (auto i : std::views::iota(size_t{0}, key_addrs_span.size())) {
        auto key_code = pimpl->offsets_mania_key_code.read(pimpl->process, key_addrs_span[i]);
        if (!key_code)
            return std::nullopt;
        auto is_key_down = pimpl->offsets_mania_key_is_down.read(pimpl->process, key_addrs_span[i]);
        if (!is_key_down)
            return std::nullopt;
        ret.emplace_back(*key_code, *is_key_down);
    }
    return ret;
}
std::optional<bool> Self::get_is_autoplay() noexcept {
    const auto rulesets = pimpl->sig_rulesets.scan(pimpl->process);
    if (!rulesets) {
        return std::nullopt;
    }
    return pimpl->offsets_mania_is_autoplay.read(pimpl->process, *rulesets);
}
