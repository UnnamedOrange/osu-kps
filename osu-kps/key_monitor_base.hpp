// Copyright (c) UnnamedOrange. Licensed under the MIT Licence.
// See the LICENSE file in the repository root for full licence text.

#pragma once

#include <array>

#include "kps_calculator.hpp"

namespace kps
{
	/// <summary>
	/// 按键监视器。基类本身不创建其他线程或钩子。
	/// </summary>
	class key_monitor_base
	{
	protected:
		/// <summary>
		/// 由使用者实现。使用者继承 key_monitor，然后 key_monitor 会自行调用该函数。不保证调用线程是主线程或其他线程。
		/// </summary>
		virtual void on_key_down(int key, time_point time) = 0;

	private:
		std::array<bool, 256> is_down{};
	protected:
		/// <summary>
		/// 由 key_monitor 调用。当得知某个键被按下时，即调用该函数。
		/// </summary>
		/// <param name="key">按下的键。</param>
		/// <param name="time">时间。</param>
		void _on_llkey_down(int key, time_point time)
		{
			if (!is_down[key])
			{
				is_down[key] = true;
				on_key_down(key, time);
			}
		}
		/// <summary>
		/// 由 key_monitor 调用。当得知某个键已抬起时，即调用该函数。按键抬起的时间点不重要。
		/// </summary>
		/// <param name="key">抬起的键。</param>
		void _on_llkey_up(int key)
		{
			is_down[key] = false;
		}
	};
}