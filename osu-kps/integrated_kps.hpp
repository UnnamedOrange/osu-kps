// Copyright (c) UnnamedOrange. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#pragma once

#include <type_traits>

#include "kps_calculator.hpp"
#include "key_monitor_hook.hpp"
#include "key_monitor_async.hpp"

namespace kps
{
	template<bool hook_implement = false>
	class integrated_kps : public kps_calculator,
		public std::conditional_t<hook_implement, key_monitor_hook, key_monitor_async>
	{
	protected:
		virtual void on_key_down(int key, time_point time)
		{
			notify_key_down(key, time);
		}
	};
	using kps_debug = integrated_kps<false>;
	using kps_release = integrated_kps<true>;
#ifdef _DEBUG
	using kps = kps_debug;
#else
	using kps = kps_release;
#endif
}