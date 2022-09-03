// Copyright (c) UnnamedOrange. Licensed under the MIT Licence.
// See the LICENSE file in the repository root for full licence text.

#pragma once

#include <type_traits>
#include <functional>

#include "kps_calculator.hpp"
#include "key_monitor.hpp"
#include "key_monitor_hook.hpp"
#include "key_monitor_async.hpp"
#include "key_monitor_memory.hpp"
#include "key_monitor_malody.hpp"

namespace kps
{
	enum key_monitor_implement_type
	{
		monitor_implement_type_async,
		monitor_implement_type_hook,
		monitor_implement_type_memory,
		monitor_implement_type_malody,
	};

	class kps final : public kps_calculator
	{
	public:
		using callback_t = key_monitor::callback_t;
	private:
		callback_t callback;
		key_monitor monitor;
		key_monitor_implement_type crt_type;
	private:
		void on_key(int key, time_point time, bool down, bool is_scan_code)
		{
			if (down)
				notify_key_down(key, time);
			if (callback)
				callback(key, time, down, is_scan_code);
		}
	private:
		void change_monitor_implement_type(std::shared_ptr<key_monitor_base> p)
		{
			monitor.set_implement(p);
		}
	public:
		void change_monitor_implement_type(key_monitor_implement_type type)
		{
			switch (type)
			{
			case monitor_implement_type_async:
				change_monitor_implement_type(std::make_shared<key_monitor_async>());
				break;
			case monitor_implement_type_hook:
				change_monitor_implement_type(std::make_shared<key_monitor_hook>());
				break;
			case monitor_implement_type_memory:
				change_monitor_implement_type(std::make_shared<key_monitor_memory>());
				break;
			case monitor_implement_type_malody:
				change_monitor_implement_type(std::make_shared<key_monitor_malody>());
				break;
			default:
				throw std::invalid_argument("type not supported.");
			}
			crt_type = type;
		}
		key_monitor_implement_type get_monitor_implement_type() const
		{
			return crt_type;
		}
		bool is_scan_code() const
		{
			return monitor.is_scan_code();
		}

	public:
		kps(callback_t callback) : callback(callback)
		{
			monitor.set_callback(std::bind_front(&kps::on_key, this));
#ifdef _DEBUG
			change_monitor_implement_type(monitor_implement_type_async);
			crt_type = monitor_implement_type_async;
#else
			change_monitor_implement_type(monitor_implement_type_hook);
			crt_type = monitor_implement_type_hook;
#endif
		}
	};
}