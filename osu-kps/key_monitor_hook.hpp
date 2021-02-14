// Copyright (c) UnnamedOrange. Licensed under the MIT Licence.
// See the LICENSE file in the repository root for full licence text.

#pragma once

#include <unordered_map>
#include <thread>
#include <semaphore>

#include <utils/hook.hpp>

#include <Windows.h>
#undef min
#undef max

#include "key_monitor_base.hpp"

namespace kps
{
	/// <summary>
	/// 使用钩子实现的按键监视器。
	/// </summary>
	class key_monitor_hook : public key_monitor_base
	{
	private:
		using key_monitor_base::_on_llkey_down;
		using key_monitor_base::_on_llkey_up;

	private:
		keyboard_hook k_hook;
		bool keyboard_proc(bool down, int vk, std::chrono::steady_clock::time_point time)
		{
			UNREFERENCED_PARAMETER(time);
			if (down)
				_on_llkey_down(vk, time);
			else
				_on_llkey_up(vk);
			return false;
		}
		mouse_hook m_hook;
		bool mouse_proc(UINT message, PMSLLHOOKSTRUCT ms, std::chrono::steady_clock::time_point time)
		{
			UNREFERENCED_PARAMETER(ms);
			UNREFERENCED_PARAMETER(time);
			if (message == WM_LBUTTONDOWN)
				_on_llkey_down(VK_LBUTTON, clock::now());
			else if (message == WM_LBUTTONUP)
				_on_llkey_up(VK_LBUTTON);
			else if (message == WM_RBUTTONDOWN)
				_on_llkey_down(VK_RBUTTON, clock::now());
			else if (message == WM_RBUTTONUP)
				_on_llkey_up(VK_RBUTTON);
			return false;
		}
	public:
		key_monitor_hook()
		{
			k_hook.set_callback(std::bind(&key_monitor_hook::keyboard_proc,
				this,
				std::placeholders::_1,
				std::placeholders::_2,
				std::placeholders::_3));
			m_hook.set_callback(std::bind(&key_monitor_hook::mouse_proc,
				this,
				std::placeholders::_1,
				std::placeholders::_2,
				std::placeholders::_3));
		}
	};
}