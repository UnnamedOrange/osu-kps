// Copyright (c) UnnamedOrange. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#pragma once

#include <vector>
#include <array>
#include <stdexcept>

/// <summary>
/// 按键管理器。用于保存当前按键数量、获取当前各按键。
/// </summary>
class keys_manager
{
	std::array<std::vector<int>, 10> keys;
	int crt_button_count{ 4 };

public:
	keys_manager()
	{
		constexpr int space = 0x20;
		constexpr int rmenu = 0xA5;
		keys[0] = { space };
		keys[1] = { 'Z', 'X' };
		keys[2] = { 'Z', 'X', 'C' };
		keys[3] = { 'D', 'F', 'J', 'K' };
		keys[4] = { 'D', 'F', space, 'J', 'K' };
		keys[5] = { 'S', 'D', 'F', 'J', 'K', 'L' };
		keys[6] = { 'S', 'D', 'F', space, 'J', 'K', 'L' };
		keys[7] = { 'A', 'S', 'D', 'F', 'J', 'K', 'L', ';' };
		keys[8] = { 'A', 'S', 'D', 'F', space, 'J', 'K', 'L', ';' };
		keys[9] = { 'D', 'F', space, 'J', 'K', 'E', 'R', rmenu,'U','I' };
	}
	int get_button_count() const { return crt_button_count; }
	void set_button_count(int new_button_count)
	{
		if (!(1 <= new_button_count && new_button_count <= static_cast<int>(keys.size())))
			throw std::invalid_argument("new_button_count should be in [1, keys.size()].");
		crt_button_count = new_button_count;
	}
	const std::vector<int>& get_keys() const
	{
		return keys[static_cast<size_t>(crt_button_count) - 1];
	}
};