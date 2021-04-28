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
		int pre{};
	private:
		std::binary_semaphore s_exit{ 0 };
		std::thread t{ &key_monitor_memory::thread_proc, this };
		void thread_proc()
		{
			while (true)
			{
				if (s_exit.try_acquire_for(10ms))
					break;

				{
					using osu_memory::reader;
					auto t1 = reader::get_50();
					auto t2 = reader::get_100();
					auto t3 = reader::get_200();
					auto t4 = reader::get_300();
					auto t5 = reader::get_perfect();
					if (!(t1 && t2 && t3 && t4 && t5))
					{
						pre = 0;
						continue;
					}

					int crt = *t1 + *t2 + *t3 + *t4 + *t5;
					if (pre <= crt && crt <= pre + 10)
						for (int i = 0, to = crt - pre; i < to; i++)
						{
							_on_llkey_down(0, clock::now());
							_on_llkey_up(0, clock::now());
						}
					pre = crt;
				}
			}
		}

	public:
		~key_monitor_memory()
		{
			s_exit.release();
			t.join();
		}
	};
}