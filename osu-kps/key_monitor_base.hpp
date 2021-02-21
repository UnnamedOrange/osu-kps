// Copyright (c) UnnamedOrange. Licensed under the MIT Licence.
// See the LICENSE file in the repository root for full licence text.

#pragma once

#include <array>
#include <functional>
#include <mutex>

#include "kps_calculator.hpp"

namespace kps
{
	/// <summary>
	/// 按键监视器。基类本身不创建其他线程或钩子。
	/// </summary>
	class key_monitor_base
	{
	public:
		/// <summary>
		/// 回调函数的类型。使用 set_callback 注册回调函数。
		/// </summary>
		using callback_t = std::function<void(int key, time_point time, bool is_down)>;
	private:
		std::mutex mutex_callback;
		callback_t callback;
	public:
		/// <summary>
		/// 设置回调函数。类型参见 callback_t。
		/// </summary>
		void set_callback(callback_t func)
		{
			std::lock_guard _(mutex_callback);
			callback = func;
		}
		/// <summary>
		/// 清空回调函数。
		/// </summary>
		void reset_callback()
		{
			std::lock_guard _(mutex_callback);
			callback = callback_t();
		}

	private:
		std::array<bool, 256> is_down{};
	protected:
		/// <summary>
		/// 由 key_monitor_base 的子类调用。当得知某个键被按下时，即调用该函数。
		/// </summary>
		/// <param name="key">按下的键。</param>
		/// <param name="time">时间。</param>
		void _on_llkey_down(int key, time_point time)
		{
			if (!is_down[key])
			{
				is_down[key] = true;
				std::lock_guard _(mutex_callback);
				if (callback)
					callback(key, time, true);
			}
		}
		/// <summary>
		/// 由 key_monitor_base 的子类调用。当得知某个键已抬起时，即调用该函数。
		/// </summary>
		/// <param name="key">抬起的键。</param>
		/// <param name="time">时间。</param>
		void _on_llkey_up(int key, time_point time)
		{
			is_down[key] = false;
			if (callback)
				callback(key, time, false);
		}

	public:
		key_monitor_base() = default;
		key_monitor_base(const key_monitor_base&) = delete;
		key_monitor_base(key_monitor_base&&) = delete;
		key_monitor_base& operator=(const key_monitor_base&) = delete;
		key_monitor_base& operator=(key_monitor_base&&) = delete;
	};
}