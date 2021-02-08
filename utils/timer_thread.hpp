// Copyright (c) UnnamedOrange. Licensed under the MIT Licence.
// See the LICENSE file in the repository root for full licence text.

#pragma once

#include <functional>
#include <thread>
#include <semaphore>
#include <chrono>

/// <summary>
/// 每过一定时间执行一次例程的时钟线程。不可修改要执行的例程。可以随时修改等待时间，修改将在下一次例程执行后生效。
/// </summary>
class timer_thread
{
private:
	std::function<void()> proc;
	std::thread thread_timer;
	std::binary_semaphore bs_exit{ 0 };
	void thread_routine()
	{
		while (true)
		{
			using namespace std::literals;
			if (bs_exit.try_acquire_for(std::chrono::milliseconds(elapse)))
				break;

			proc();
		}
	}
public:
	unsigned elapse;
private:
	std::thread thread_work;

public:
	/// <param name="proc">要执行的例程。</param>
	/// <param name="elapse">起始等待时间，以毫秒为单位。例程不会立即执行，而是会在 elapse 毫秒后首次执行。</param>
	timer_thread(std::function<void()> proc, unsigned elapse = 1000) : proc(proc), elapse(elapse),
		thread_work(&timer_thread::thread_routine, this)
	{
	}
	~timer_thread()
	{
		bs_exit.release();
		thread_work.join();
	}
};