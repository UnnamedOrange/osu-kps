// Copyright (c) UnnamedOrange. Licensed under the MIT Licence.
// See the LICENSE file in the repository root for full licence text.

#pragma once

#include <cctype>

#include <Windows.h>
#undef min
#undef max
#include "resource.h"

#include "utils/ConvertCode.hpp"
#include "utils/multi_language.hpp"
#include "utils/resource_loader.hpp"

class my_multi_language : public multi_language {
public:
    my_multi_language() {
        {
            auto t = resource_loader::load(MAKEINTRESOURCEW(IDR_JSON_EN_US), L"JSON");
            if (!load_language(std::string_view(reinterpret_cast<const char*>(t.data()))))
                throw std::runtime_error("fail to load_language.");
        }
        {
            auto t = resource_loader::load(MAKEINTRESOURCEW(IDR_JSON_ZH_CN), L"JSON");
            load_language(std::string_view(reinterpret_cast<const char*>(t.data())));
        }
    }

public:
    std::wstring operator[](std::string_view key) const {
        using namespace orange;
        return ConvertCode::to_wstring(multi_language::operator[](key));
    }
    void set_current_language_to_system_default() {
        using namespace orange;
        wchar_t locale[LOCALE_NAME_MAX_LENGTH];
        GetUserDefaultLocaleName(locale, LOCALE_NAME_MAX_LENGTH);
        set_current_language(ConvertCode::to_u8string(locale));
    }
};

inline my_multi_language lang;
