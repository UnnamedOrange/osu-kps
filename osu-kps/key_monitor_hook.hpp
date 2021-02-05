// Copyright (c) UnnamedOrange. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#pragma once

#include <unordered_map>
#include <thread>
#include <semaphore>

#include <Windows.h>
#undef min
#undef max

#include "key_monitor_base.hpp"

namespace kps
{
	/// <summary>
	/// 使用钩子实现的按键监视器。同一线程只能创建一个这样的按键监视器，但考虑到只会有一个线程创建该对象，因此不保证线程安全。没有测试在两个线程上存在两个该实例是否能正确运行。
	/// </summary>
	class key_monitor_hook : public key_monitor_base
	{
	private:
		using key_monitor_base::_on_llkey_down;
		using key_monitor_base::_on_llkey_up;

	private:
		HHOOK hKeybd{};
		HHOOK hMouse{};
		// 线程 id 到线程对应按键监视器实例的映射。因此同一线程只能创建一个基于钩子的按键监视器。
		inline static std::unordered_map<std::thread::id, key_monitor_hook*> id2instance;
		static LRESULT CALLBACK VirtualKeybdProc(int nCode, WPARAM wParam, LPARAM lParam)
		{
			return id2instance[std::this_thread::get_id()]->KeybdProc(nCode, wParam, lParam);
		}
		LRESULT KeybdProc(int nCode, WPARAM wParam, LPARAM lParam)
		{
			if (nCode < 0)
				return CallNextHookEx(hKeybd, nCode, wParam, lParam);

			auto ks = PKBDLLHOOKSTRUCT(lParam);
			if (nCode == HC_ACTION)
			{
				if (wParam == WM_KEYDOWN)
					_on_llkey_down(ks->vkCode, clock::now());
				else if (wParam == WM_KEYUP)
					_on_llkey_up(ks->vkCode);
			}
			return CallNextHookEx(hKeybd, nCode, wParam, lParam);
		}
		static LRESULT CALLBACK VirtualMouseProc(int nCode, WPARAM wParam, LPARAM lParam)
		{
			return id2instance[std::this_thread::get_id()]->MouseProc(nCode, wParam, lParam);
		}
		LRESULT MouseProc(int nCode, WPARAM wParam, LPARAM lParam)
		{
			if (nCode < 0)
				return CallNextHookEx(hMouse, nCode, wParam, lParam);

			if (nCode == HC_ACTION)
			{
				if (wParam == WM_LBUTTONDOWN)
					_on_llkey_down(VK_LBUTTON, clock::now());
				else if (wParam == WM_LBUTTONUP)
					_on_llkey_up(VK_LBUTTON);
				else if (wParam == WM_RBUTTONDOWN)
					_on_llkey_down(VK_RBUTTON, clock::now());
				else if (wParam == WM_RBUTTONUP)
					_on_llkey_up(VK_RBUTTON);
			}
			return CallNextHookEx(hMouse, nCode, wParam, lParam);
		}
	public:
		key_monitor_hook()
		{
			if (id2instance.count(std::this_thread::get_id()))
				throw std::runtime_error("there has been another instance in this thread.");
			hKeybd = SetWindowsHookExW(WH_KEYBOARD_LL, &key_monitor_hook::VirtualKeybdProc,
				GetModuleHandleW(nullptr), NULL);
			if (!hKeybd)
				throw std::runtime_error("fail to SetWindowsHookExW(WH_KEYBOARD_LL, ...).");
			hMouse = SetWindowsHookExW(WH_MOUSE_LL, &key_monitor_hook::VirtualMouseProc,
				GetModuleHandleW(nullptr), NULL);
			if (!hMouse)
			{
				UnhookWindowsHookEx(hKeybd);
				throw std::runtime_error("fail to SetWindowsHookExW(WH_MOUSE_LL, ...).");
			}
			id2instance[std::this_thread::get_id()] = this;
		}
		~key_monitor_hook()
		{
			id2instance.erase(std::this_thread::get_id());
			UnhookWindowsHookEx(hMouse);
			UnhookWindowsHookEx(hKeybd);
		}
		key_monitor_hook(const key_monitor_hook&) = delete;
		key_monitor_hook(key_monitor_hook&&) = delete;
		key_monitor_hook& operator=(const key_monitor_hook&) = delete;
		key_monitor_hook& operator=(key_monitor_hook&&) = delete;
	};
}