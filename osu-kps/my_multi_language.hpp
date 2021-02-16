// Copyright (c) UnnamedOrange. Licensed under the MIT Licence.
// See the LICENSE file in the repository root for full licence text.

#pragma once

#include "resource.h"

#include <utils/resource_loader.hpp>
#include <utils/multi_language.hpp>

class my_multi_language : public multi_language
{
public:
	my_multi_language()
	{
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
	std::wstring operator[](std::string_view key) const
	{
		return code_conv<char8_t, wchar_t>::convert(multi_language::operator[](key));
	}
};

inline my_multi_language lang;