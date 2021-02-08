// Copyright (c) UnnamedOrange. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#pragma once

#include <vector>
#include <array>
#include <stdexcept>

#include "kps_calculator.hpp"

/// <summary>
/// 按键管理器。用于保存当前按键数量、获取当前各按键。主要用于获取画图时需要的信息。
/// </summary>
class keys_manager
{
public:
	static constexpr size_t max_key_count = 10;
	static constexpr size_t default_key_count = 4;
private:
	std::recursive_mutex m;
private:
	kps::kps* src{};
private:
	std::array<std::vector<int>, max_key_count> keys;
	int crt_button_count{ default_key_count };
	struct key_info
	{
		kps::time_point previous{};
		int times{};
	};
	std::vector<key_info> extra_info{ default_key_count };
	// 其他统计信息。
private:
	int total_count{};
	int max_kps{};

public:
	/// <summary>
	/// 当某个键被按下时调用，用于更新信息。该方法可能运行在主线程或子线程。该方法不能花费过多 CPU 时间。
	/// </summary>
	/// <param name="key"></param>
	/// <param name="time"></param>
	void update_on_key_down(int key, kps::time_point time)
	{
		std::lock_guard _(m);
		const auto& crt_keys = keys[crt_button_count - 1];
		for (size_t i = 0; i < crt_keys.size(); i++)
			if (key == crt_keys[i])
			{
				extra_info[i].previous = time;
				extra_info[i].times++;
			}

		if (std::count(crt_keys.begin(), crt_keys.end(), key))
			++total_count;

		if (src)
			max_kps = std::max(max_kps, src->calc_kps_now(crt_keys));
	}

public:
	keys_manager()
	{
		constexpr int lbutton = 0x01;
		constexpr int space = 0x20;
		constexpr int rmenu = 0xA5;
		constexpr int oem1 = 0xBA; // ;
		keys[0] = { lbutton };
		keys[1] = { 'Z', 'X' };
		keys[2] = { 'Z', 'X', 'C' };
		keys[3] = { 'D', 'F', 'J', 'K' };
		keys[4] = { 'D', 'F', space, 'J', 'K' };
		keys[5] = { 'S', 'D', 'F', 'J', 'K', 'L' };
		keys[6] = { 'S', 'D', 'F', space, 'J', 'K', 'L' };
		keys[7] = { 'A', 'S', 'D', 'F', 'J', 'K', 'L', oem1 };
		keys[8] = { 'A', 'S', 'D', 'F', space, 'J', 'K', 'L', oem1 };
		keys[9] = { 'D', 'F', space, 'J', 'K', 'E', 'R', rmenu, 'U', 'I' };
	}
	keys_manager(kps::kps* source) : keys_manager()
	{
		src = source;
	}
	/// <summary>
	/// 获取当前按键数量。
	/// </summary>
	/// <returns></returns>
	int get_button_count() const { return crt_button_count; }
	/// <summary>
	/// 修改当前按键数量。该方法只能运行在主线程。
	/// </summary>
	/// <param name="new_button_count"></param>
	void set_button_count(int new_button_count)
	{
		std::lock_guard _(m);
		if (!(1 <= new_button_count && new_button_count <= static_cast<int>(keys.size())))
			throw std::invalid_argument("new_button_count should be in [1, keys.size()].");
		crt_button_count = new_button_count;
		extra_info.clear();
		extra_info.resize(crt_button_count);
	}
	/// <summary>
	/// 获取当前的按键集合（序列）。只有主线程才应在外部调用该函数。
	/// </summary>
	/// <returns></returns>
	const std::vector<int>& get_keys() const
	{
		return keys[static_cast<size_t>(crt_button_count) - 1];
	}

public:
	/// <returns>总按键次数。</returns>
	int get_total_count() const
	{
		return total_count;
	}
	/// <summary>
	/// 清空总按键次数。
	/// </summary>
	void clear_total_count()
	{
		std::lock_guard _(m);
		total_count = 0;
	}
	/// <returns>最大 KPS。</returns>
	int get_max_kps() const
	{
		return max_kps;
	}
	/// <summary>
	/// 清空最大 KPS。
	/// </summary>
	void clear_max_kps()
	{
		std::lock_guard _(m);
		max_kps = 0;
	}
};