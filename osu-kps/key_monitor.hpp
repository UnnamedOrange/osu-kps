// Copyright (c) UnnamedOrange. Licensed under the MIT Licence.
// See the LICENSE file in the repository root for full licence text.

#pragma once

#include <memory>

#include "key_monitor_base.hpp"

namespace kps
{
	/// <summary>
	/// 按键监视器的管理端。允许更改实现方式。
	/// </summary>
	class key_monitor
	{
	private:
		std::recursive_mutex m; // 成员函数外部调用互斥。
	private:
		std::shared_ptr<key_monitor_base> implement;

	public:
		using callback_t = key_monitor_base::callback_t;
	private:
		callback_t callback;
	public:
		/// <summary>
		/// 设置回调函数。类型参见 callback_t。
		/// </summary>
		void set_callback(callback_t func)
		{
			std::lock_guard _(m);
			callback = func;
			if (implement)
				implement->set_callback(callback);
		}
		/// <summary>
		/// 清空回调函数。
		/// </summary>
		void reset_callback()
		{
			std::lock_guard _(m);
			callback = callback_t();
			if (implement)
				implement->reset_callback();
		}

	public:
		/// <summary>
		/// 设置实现方式。
		/// </summary>
		void set_implement(std::shared_ptr<key_monitor_base> p)
		{
			std::lock_guard _(m);
			if (implement)
				implement->reset_callback();
			p->reset_callback();
			implement = p;
			p->set_callback(callback);
		}

	public:
		key_monitor() = default;
		key_monitor(const key_monitor&) = delete;
		key_monitor(key_monitor&&) = default;
		key_monitor& operator=(const key_monitor&) = delete;
		key_monitor& operator=(key_monitor&&) = default;
	};
}