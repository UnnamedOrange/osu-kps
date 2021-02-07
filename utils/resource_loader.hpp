// Copyright (c) UnnamedOrange. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#pragma once

#include <vector>
#include <string_view>
#include <stdexcept>
#include <Windows.h>
#undef min
#undef max

class resource_loader
{
public:
	static auto load(std::wstring_view resource_name, std::wstring_view type_name)
	{
		HINSTANCE hInstance = GetModuleHandleW(nullptr);
		HRSRC hResInfo = FindResourceW(hInstance,
			resource_name.data(), type_name.data());
		if (!hResInfo)
			throw std::runtime_error("fail to FindResourceW.");
		HGLOBAL hResource = LoadResource(hInstance, hResInfo);
		if (!hResource)
			throw std::runtime_error("fail to LoadResource.");
		DWORD dwSize = SizeofResource(hInstance, hResInfo);
		if (!dwSize)
			throw std::runtime_error("fail to SizeofResource.");

		BYTE* p = reinterpret_cast<BYTE*>(LockResource(hResource));
		std::vector<BYTE> ret(p, p + dwSize);
		FreeResource(hResource);

		return ret;
	}
};