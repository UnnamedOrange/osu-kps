// Copyright (c) UnnamedOrange. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#pragma once

#include <vector>
#include <deque>
#include <algorithm>
#include <numeric>
#include <array>
#include <tuple>
#include <memory>
#include <chrono>
#include <mutex>

namespace kps
{
	using clock = std::chrono::steady_clock;
	using time_point = std::chrono::time_point<clock>;
	using namespace std::literals;

	class kps_interface
	{
	private:
		mutable std::mutex m; // 只允许一个线程运行各成员函数。
	public:
		using deque_t = std::deque<std::tuple<int, time_point>>;
	private:
		deque_t records; // 按键记录。按时间（第二个元素）升序。
	public:
		using callback_t = std::function<void(int, time_point)>;
	private:
		callback_t callback; // 按键时的回调函数。

	private:
		/// <summary>
		/// 注册回调函数。按键时会被调用。
		/// 调用线程与监视器线程相同。保证调用该函数时已上锁。不要再次上锁！
		/// 会覆盖之前注册的回调函数。
		/// </summary>
		/// <param name="func"></param>
		void register_callback(callback_t func)
		{
			std::lock_guard _(m);
			callback = func;
		}
		/// <summary>
		/// 取消注册回调函数。
		/// 有可能调用该函数时回调函数正在被调用，此时将在回调函数运行结束后再取消注册。
		/// </summary>
		void unregister_callback()
		{
			std::lock_guard _(m);
			callback = callback_t();
		}

	public:
		kps_interface() = default;
		kps_interface(const kps_interface&) = delete;
		kps_interface(kps_interface&&) = delete;
		kps_interface& operator=(const kps_interface&) = delete;
		kps_interface& operator=(kps_interface&&) = delete;

	public:
		/// <summary>
		/// 清空按键记录。
		/// </summary>
		void clear()
		{
			std::lock_guard _(m);
			records.clear();
		}
	protected:
		/// <summary>
		/// 监视器通知按键。
		/// </summary>
		/// <param name="key">按的键。</param>
		/// <param name="time">按键时间。</param>
		void notify_key_down(int key, time_point time)
		{
			std::lock_guard _(m);
			records.push_back({ key, time });
			if (callback)
				callback(key, time);
		}

		friend class kps_implement_base;
		friend class kps_implement_hard;
	};

	class kps_implement_base
	{
	protected:
		kps_interface* src;
	public:
		kps_implement_base(kps_interface* src) : src(src) {}
		kps_implement_base(const kps_implement_base&) = delete;
		kps_implement_base(kps_implement_base&&) = delete;
		kps_implement_base& operator=(const kps_implement_base&) = delete;
		kps_implement_base& operator=(kps_implement_base&&) = delete;

	private:
		/// <summary>
		/// 计算当前 kps。通过 src 访问数据。保证在被调用时 src 不会发生写操作。
		/// </summary>
		/// <param name="key"></param>
		/// <returns></returns>
		virtual double calc_kps_now_implement(int key) const = 0;
	public:
		double calc_kps_now(int key) const
		{
			std::lock_guard _(src->m);
			return calc_kps_now_implement(key);
		}
		double calc_kps_now(const std::vector<int>& keys) const
		{
			std::lock_guard _(src->m);
			return std::accumulate(keys.begin(), keys.end(), 0.0,
				[this](double pre, int key) { return pre + calc_kps_now_implement(key); });
		}
	};

	class kps_implement_hard : public kps_implement_base
	{
		static constexpr auto frame_length = 1s; // 计算 KPS 的帧长度。
		mutable size_t start_index{}; // 计算当前 KPS 时，最早按键记录的下标。
		mutable std::array<int, 256> sum{}; // 计算当前 KPS 时，各按键按键次数总和。
	public:
		kps_implement_hard(kps_interface* src) : kps_implement_base(src)
		{
			src->register_callback(
				std::bind(&kps_implement_hard::on_key_down, this,
					std::placeholders::_1, std::placeholders::_2));
			std::lock_guard _(src->m);
			for (const auto& [key, time] : src->records)
				sum[key]++;
		}
		~kps_implement_hard()
		{
			src->unregister_callback();
		}
	private:
		void on_key_down(int key, time_point)
		{
			sum[key]++;
		}
		virtual double calc_kps_now_implement(int key) const override
		{
			auto now = clock::now();
			while (start_index < src->records.size() &&
				now - std::get<1>(src->records[start_index]) > frame_length)
			{
				sum[std::get<0>(src->records[start_index])]--;
				start_index++;
			}
			return sum[key];
		}
	};

	enum class kps_implement_type
	{
		kps_implement_type_hard,
	};

	class kps_calculator : public kps_interface
	{
	private:
		mutable std::mutex m;
	private:
		std::shared_ptr<kps_implement_base> implement;
	public:
		kps_calculator(kps_implement_type implement_type = kps_implement_type::kps_implement_type_hard)
		{
			change_implement_type(implement_type);
		}
		void change_implement_type(kps_implement_type implement_type)
		{
			std::lock_guard _(m);
			switch (implement_type)
			{
			case kps_implement_type::kps_implement_type_hard:
			{
				implement = std::make_shared<kps_implement_hard>(this);
				break;
			}
			}
		}

	public:
		double calc_kps_now(int key) const
		{
			std::lock_guard _(m);
			return implement->calc_kps_now(key);
		}
		double calc_kps_now(const std::vector<int>& keys) const
		{
			std::lock_guard _(m);
			return implement->calc_kps_now(keys);
		}
	};
}