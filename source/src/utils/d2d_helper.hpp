// Copyright (c) UnnamedOrange. Licensed under the MIT Licence.
// See the LICENSE file in the repository root for full licence text.

#pragma once

#include <atomic>
#include <stdexcept>
#include <vector>

#include <d2d1.h>
#include <d2d1_1.h>
#include <d2d1_1helper.h>
#include <d2d1_2.h>
#include <d2d1_2helper.h>
#include <d2d1_3.h>
#include <d2d1_3helper.h>
#include <d2d1helper.h>
#pragma comment(lib, "d2d1.lib")
#include <dwrite.h>
#include <dwrite_1.h>
#include <dwrite_2.h>
#include <dwrite_3.h>
#pragma comment(lib, "dwrite.lib")

#include "d2d/SharedComPtr.hpp"

namespace d2d_helper {
    /// <summary>
    /// 自动创建并销毁工厂的单例对象。
    /// </summary>
    class factory {
        struct _RAII_factory {
            orange::SharedComPtr<ID2D1Factory7> d2d1_factory;
            orange::SharedComPtr<IDWriteFactory7> dwrite_factory;
            _RAII_factory() {
                if (FAILED(D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, __uuidof(ID2D1Factory7),
                                             reinterpret_cast<void**>(d2d1_factory.reset_and_get_address()))))
                    throw std::runtime_error("Fail to D2D1CreateFactory.");
                if (FAILED(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory7),
                                               reinterpret_cast<IUnknown**>(dwrite_factory.reset_and_get_address()))))
                    throw std::runtime_error("Fail to DWriteCreateFactory.");
            }
        };
        static _RAII_factory& singleton() // _factory 仅在使用时才构造。
        {
            static _RAII_factory _factory;
            return _factory;
        }

    public:
        /// <returns>ID2D1Factory 工厂。</returns>
        static auto d2d1() {
            return singleton().d2d1_factory;
        }
        static auto dwrite() {
            return singleton().dwrite_factory;
        }
    };

    class _memory_font_file_loader {
        orange::SharedComPtr<IDWriteInMemoryFontFileLoader> in_memory_font_file_loader;

    public:
        _memory_font_file_loader() {
            factory::dwrite()->CreateInMemoryFontFileLoader(in_memory_font_file_loader.reset_and_get_address());
            factory::dwrite()->RegisterFontFileLoader(in_memory_font_file_loader);
        }
        ~_memory_font_file_loader() {
            factory::dwrite()->UnregisterFontFileLoader(in_memory_font_file_loader);
        }
        void reset() {
            this->~_memory_font_file_loader();
            new (this) _memory_font_file_loader();
        }
        auto operator->() const {
            return in_memory_font_file_loader.operator->();
        }
    };

    /// <summary>
    /// 私有字体序列。一个该对象只能加载一个私有字体。
    /// </summary>
    class private_font_collection {
        _memory_font_file_loader in_memory_font_file_loader;
        orange::SharedComPtr<IDWriteFontCollection1> collection;
        std::vector<std::wstring> family_names;

    public:
        private_font_collection() = default;
        private_font_collection(const std::vector<BYTE>& font_data) {
            from_font_data(font_data);
        }

    public:
        void from_font_data(const std::vector<BYTE>& font_data) {
            using namespace orange;

            collection.reset();
            in_memory_font_file_loader.reset();

            SharedComPtr<IDWriteFontSetBuilder2> font_set_builder;
            factory::dwrite()->CreateFontSetBuilder(font_set_builder.reset_and_get_address());
            {
                SharedComPtr<IDWriteFontFile> font_file_reference;
                in_memory_font_file_loader->CreateInMemoryFontFileReference(
                    factory::dwrite(), font_data.data(), font_data.size(), nullptr,
                    font_file_reference.reset_and_get_address());
                font_set_builder->AddFontFile(font_file_reference);
            }
            SharedComPtr<IDWriteFontSet> font_set;
            font_set_builder->CreateFontSet(font_set.reset_and_get_address());
            factory::dwrite()->CreateFontCollectionFromFontSet(font_set, collection.reset_and_get_address());

            family_names.clear();
            unsigned to = collection->GetFontFamilyCount();
            family_names.reserve(to);
            for (unsigned i = 0; i < to; i++) {
                SharedComPtr<IDWriteFontFamily> family;
                collection->GetFontFamily(i, family.reset_and_get_address());
                SharedComPtr<IDWriteLocalizedStrings> names;
                family->GetFamilyNames(names.reset_and_get_address());
                unsigned length{};
                names->GetStringLength(0, &length);
                std::wstring temp(length, L'\0');
                names->GetString(0, temp.data(), length + 1);
                family_names.emplace_back(std::move(temp));
            }
        }

    public:
        auto get() const {
            return collection.get();
        }
        const auto& get_family_names() const {
            return family_names;
        }
    };

    /// <summary>
    /// 颜色类。
    /// </summary>
    class color {
    public:
        FLOAT r{};
        FLOAT g{};
        FLOAT b{};
        FLOAT a{1};

    public:
        static constexpr unsigned red_shift = 16;
        static constexpr unsigned green_shift = 8;
        static constexpr unsigned blue_shift = 0;
        static constexpr unsigned alpha_shift = 24;
        static constexpr unsigned red_mask = 0xff << red_shift;
        static constexpr unsigned green_mask = 0xff << green_shift;
        static constexpr unsigned blue_mask = 0xff << blue_shift;
        static constexpr unsigned alpha_mask = 0xff << alpha_shift;

    public:
        constexpr color() = default;
        constexpr color(unsigned int rgb_or_argb, unsigned char a = 0)
            : r{static_cast<FLOAT>((rgb_or_argb & red_mask) >> red_shift) / 255.f},
              g{static_cast<FLOAT>((rgb_or_argb & green_mask) >> green_shift) / 255.f},
              b{static_cast<FLOAT>((rgb_or_argb & blue_mask) >> blue_shift) / 255.f} {
            if (a)
                this->a = static_cast<FLOAT>(a / 255.f);
            else
                this->a = static_cast<FLOAT>((rgb_or_argb & alpha_mask) >> alpha_shift) / 255.f;
        }
        constexpr color(FLOAT r, FLOAT g, FLOAT b, FLOAT a = 1.f) : r(r), g(g), b(b), a(a) {}
        constexpr color(unsigned r, unsigned g, unsigned b, unsigned a = 255)
            : r(static_cast<FLOAT>(r) / 255.f), g(static_cast<FLOAT>(g) / 255.f), b(static_cast<FLOAT>(b) / 255.f),
              a(static_cast<FLOAT>(a) / 255.f) {}

    public:
        static constexpr color linear_interpolation(const color& c0, const color& c1, FLOAT ratio) {
            color ret;
            ret.r = c0.r * (1 - ratio) + c1.r * ratio;
            ret.g = c0.g * (1 - ratio) + c1.g * ratio;
            ret.b = c0.b * (1 - ratio) + c1.b * ratio;
            ret.a = c0.a * (1 - ratio) + c1.a * ratio;
            return ret;
        }

    public:
        operator D2D1::ColorF() const {
            return D2D1::ColorF(r, g, b, a);
        }
    };
} // namespace d2d_helper
