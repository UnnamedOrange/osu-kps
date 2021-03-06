﻿// Copyright (c) UnnamedOrange. Licensed under the MIT Licence.
// See the LICENSE file in the repository root for full licence text.

#pragma once

#include <type_traits>
#include <functional>

#include "kps_calculator.hpp"
#include "key_monitor.hpp"
#include "key_monitor_hook.hpp"
#include "key_monitor_async.hpp"

namespace kps
{
	enum key_monitor_implement_type
	{
		monitor_implement_type_async,
		monitor_implement_type_hook,
	};

	class kps final : public kps_calculator
	{
	public:
		using callback_t = key_monitor::callback_t;
	private:
		callback_t callback;
		key_monitor monitor;
	private:
		void on_key(int key, time_point time, bool down)
		{
			if (down)
				notify_key_down(key, time);
			if (callback)
				callback(key, time, down);
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
			default:
				throw std::invalid_argument("type not supported.");
			}
		}

	public:
		kps(callback_t callback) : callback(callback)
		{
			monitor.set_callback(std::bind(&kps::on_key,
				this,
				std::placeholders::_1,
				std::placeholders::_2,
				std::placeholders::_3));
#ifdef _DEBUG
			change_monitor_implement_type(monitor_implement_type_async);
#else
			change_monitor_implement_type(monitor_implement_type_hook);
#endif
		}
	};
}