// Copyright (c) UnnamedOrange. Licensed under the MIT Licence.
// See the LICENSE file in the repository root for full licence text.

#pragma once

#include <type_traits>
#include <functional>

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
			s_constructor.acquire();
			notify_key_down(key, time);
			if (callback)
				callback(key, time);
			s_constructor.release();
		}
	private:
		using callback_t = std::function<void(int, time_point)>;
		callback_t callback;
		// TODO: 潜在的线程安全问题。有可能 on_key_down 被调用时 s_constructor 尚未构造完成。
		std::binary_semaphore s_constructor{ 0 }; // 仅保证 callback 在被调用时状态正确。
	public:
		integrated_kps()
		{
			s_constructor.release();
		}
		integrated_kps(callback_t callback) : callback(callback)
		{
			s_constructor.release();
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