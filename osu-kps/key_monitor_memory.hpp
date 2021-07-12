// Copyright (c) UnnamedOrange. Licensed under the MIT Licence.
// See the LICENSE file in the repository root for full licence text.

#pragma once

#include <unordered_map>
#include <thread>
#include <semaphore>
#include <chrono>
#include <concepts>
using namespace std::literals;

#include <Windows.h>
#undef min
#undef max

#include <osu_memory/osu_memory.h>

#include "key_monitor_base.hpp"

namespace kps
{
	/// <summary>
	/// 使用 osu-memory 实现的按键监视器。不支持单一按键。
	/// </summary>
	class key_monitor_memory final : public key_monitor_base
	{
	private:
		using key_monitor_base::_on_llkey_down;
		using key_monitor_base::_on_llkey_up;

	private:
		bool exit{ false };
		std::thread t{ &key_monitor_memory::thread_proc, this };
		void thread_proc()
		{
			while (!exit)
			{
				using osu_memory::reader;

				auto now = clock::now();
				auto keys = reader::get_keys();
				if (!keys || (*keys).empty())
				{
					for (int i = 0; i < 256; i++)
						_on_llkey_up(i, now);
					continue;
				}

				for (const auto& t : *keys)
					if (t.second)
						_on_llkey_down(t.first, now);
					else
						_on_llkey_up(t.first, now);
			}
		}

	public:
		~key_monitor_memory()
		{
			exit = true;
			t.join();
		}
	};
}