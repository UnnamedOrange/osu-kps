// Copyright (c) UnnamedOrange. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#pragma once

#include <vector>
#include <deque>
#include <algorithm>
#include <numeric>
#include <array>
#include <tuple>
#include <chrono>
#include <mutex>

namespace kps
{
	using clock = std::chrono::steady_clock;
	using time_point = std::chrono::time_point<clock>;
	using namespace std::literals;

	/// <summary>
	/// KPS 计算器。
	/// </summary>
	class kps_calculator
	{
	private:
		mutable std::recursive_mutex m; // 只允许一个线程运行各成员函数。
	private:
		static constexpr size_t cache_size = 8192; // 保存按键数量的最大值。
		static constexpr auto frame_length = 1s; // 计算 KPS 的帧长度。
		std::deque<std::tuple<int, time_point>> records; // 按键记录。按时间（第二个元素）升序。
		mutable size_t start_index{}; // 计算当前 KPS 时，最早按键记录的下标。
		mutable std::array<int, 256> sum{}; // 计算当前 KPS 时，各按键按键次数总和。
		// TODO: 增加记录最大值的功能。

	public:
		kps_calculator() = default;
		kps_calculator(const kps_calculator& another)
		{
			*this = another;
		}
		kps_calculator(kps_calculator&& another) noexcept
		{
			*this = std::move(another);
		}
		kps_calculator& operator=(const kps_calculator& another)
		{
			if (this != &another)
			{
				std::lock_guard _1(m);
				std::lock_guard _2(another.m);
				records = another.records;
				start_index = another.start_index;
				sum = another.sum;
			}
			return *this;
		}
		kps_calculator& operator=(kps_calculator&& another) noexcept
		{
			if (this != &another)
			{
				std::lock_guard _1(m);
				std::lock_guard _2(another.m);
				records = std::move(another.records);
				start_index = another.start_index;
				another.start_index = 0;
				sum = another.sum;
				std::fill(another.sum.begin(), another.sum.end(), 0);
			}
			return *this;
		}

	public:
		/// <summary>
		/// 清空按键记录。
		/// </summary>
		void clear()
		{
			std::lock_guard _(m);
			records.clear();
			start_index = 0;
			std::fill(sum.begin(), sum.end(), 0);
		}
		/// <summary>
		/// 监视器通知按键。
		/// </summary>
		/// <param name="key">按的键。</param>
		/// <param name="time">按键时间。</param>
		void notify_key_down(int key, time_point time)
		{
			std::lock_guard _(m);
			// TODO: 使用生产者-消费者模型。
			records.push_back({ key, time });
			sum[key]++;
		}

	public:
		/// <summary>
		/// 根据按键记录，计算当前某个键的 KPS。
		/// </summary>
		/// <param name="key">键。</param>
		/// <returns>KPS。</returns>
		int calc_kps_now(int key) const
		{
			std::lock_guard _(m);
			auto now = clock::now();
			while (start_index < records.size() &&
				now - std::get<1>(records[start_index]) > frame_length)
			{
				sum[std::get<0>(records[start_index])]--;
			}
			return sum[key];
		}
		/// <summary>
		/// 根据按键记录，计算当前某些键的 KPS 之和。
		/// </summary>
		/// <param name="keys">键。不会去重。</param>
		/// <returns>KPS。</returns>
		int calc_kps_now(const std::vector<int>& keys) const
		{
			std::lock_guard _(m);
			return std::accumulate(keys.begin(), keys.end(), 0,
				[this](int pre, int key) { return pre + calc_kps_now(key); });
		}

		// TODO: 构思根据按键记录计算一段时间内 KPS 序列的具体要求。

		/// <summary>
		/// 根据按键记录，计算某个键在一段时间内的 KPS 序列。可能花费较多时间。
		/// </summary>
		/// <param name="key">键。</param>
		/// <returns>KPS 序列。</returns>
		std::vector<int> calc_kps_all(int key) const;
		/// <summary>
		/// 根据按键记录，计算某些键在一段时间内的 KPS 之和的序列。可能花费较多时间。
		/// </summary>
		/// <param name="key">键。</param>
		/// <returns>KPS 序列。</returns>
		std::vector<int> calc_kps_all(const std::vector<int>& keys);
	};
}