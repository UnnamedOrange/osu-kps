// Copyright (c) UnnamedOrange. Licensed under the MIT Licence.
// See the LICENSE file in the repository root for full licence text.

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
		mutable std::recursive_mutex m; // 只允许一个线程运行各成员函数。
	public:
		using deque_t = std::deque<std::tuple<int, time_point>>;
	private:
		deque_t records; // 按键记录。按时间（第二个元素）升序。
		std::array<deque_t, 256> records_individual; // 各按键的按键记录。
	public:
		using callback_t = std::function<void(int, time_point)>;
	private:
		callback_t callback; // 按键时的回调函数。
	public:
		using id_t = unsigned long long;
	private:
		unsigned long long callback_id; // 回调函数的标志 id。

	private:
		/// <summary>
		/// 注册回调函数。按键时会被调用。
		/// 调用线程与监视器线程相同。保证调用该函数时已上锁。不要再次上锁！
		/// 会覆盖之前注册的回调函数。
		/// </summary>
		/// <param name="func">注册的回调函数。</param>
		/// <param name="id">回调函数的标志 id。需要在取消注册时提供。</param>
		void register_callback(callback_t func, id_t id)
		{
			std::lock_guard _(m);
			callback = func;
			callback_id = id;
		}
		/// <summary>
		/// 取消注册回调函数。
		/// 有可能调用该函数时回调函数正在被调用，此时将在回调函数运行结束后再取消注册。
		/// </summary>
		/// <param name="id">用于标志的 id。仅当 id 与注册时的相同时，取消注册才会成功。</param>
		/// <returns>是否取消注册成功。即 id 是否匹配。</returns>
		bool unregister_callback(id_t id)
		{
			std::lock_guard _(m);
			if (callback_id == id)
			{
				callback = callback_t();
				id = id_t();
				return true;
			}
			return false;
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
			for (auto& t : records_individual)
				t.clear();
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
			records_individual[key].push_back({ key, time });
			if (callback)
				callback(key, time);
		}

		friend class kps_implement_base;
		friend class kps_implement_hard;
	};

	enum class kps_implement_type
	{
		kps_implement_type_hard,
	};

	inline constexpr size_t history_count = 300; // 历史记录个数，一秒记录一次。
	using history_array = std::array<double, history_count>;

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

	public:
		virtual kps_implement_type type() const = 0;
		void clear()
		{
			std::lock_guard _(src->m);
			src->clear();
			clear_implement();
		}

	private:
		/// <summary>
		/// 清空子类临时数据。
		/// </summary>
		virtual void clear_implement() = 0;
		/// <summary>
		/// 计算当前 kps。通过 src 访问数据。保证在被调用时 src 不会发生写操作。
		/// </summary>
		/// <param name="key"></param>
		/// <returns></returns>
		virtual double calc_kps_now_implement(int key) = 0;
		/// <summary>
		/// 计算最近的 kps。通过 src 访问数据。保证在被调用时 src 不会发生写操作。
		/// </summary>
		/// <param name="key"></param>
		/// <returns></returns>
		virtual history_array calc_kps_recent_implement(int key) = 0;
	public:
		double calc_kps_now(int key)
		{
			std::lock_guard _(src->m);
			return calc_kps_now_implement(key);
		}
		double calc_kps_now(const std::vector<int>& keys)
		{
			std::lock_guard _(src->m);
			return std::accumulate(keys.begin(), keys.end(), 0.0,
				[this](double pre, int key) { return pre + calc_kps_now_implement(key); });
		}
		history_array calc_kps_recent(int key)
		{
			std::lock_guard _(src->m);
			return calc_kps_recent_implement(key);
		}
		history_array calc_kps_recent(const std::vector<int>& keys)
		{
			std::lock_guard _(src->m);
			history_array ret{};
			for (int key : keys)
			{
				auto t = calc_kps_recent_implement(key);
				for (size_t i = 0; i < ret.size(); i++)
					ret[i] += t[i];
			}
			return ret;
		}
	};

	class kps_implement_hard : public kps_implement_base
	{
		static constexpr auto frame_length = 1s; // 计算 KPS 的帧长度。
		size_t start_index{}; // 计算当前 KPS 时，最早按键记录的下标。
		std::array<int, 256> sum{}; // 计算当前 KPS 时，各按键按键次数总和。

		std::array<size_t, 256> recent_pivot{}; // 要考虑的历史 KPS 内最靠前的下标。
	public:
		virtual kps_implement_type type() const override
		{
			return kps_implement_type::kps_implement_type_hard;
		}

	public:
		kps_implement_hard(kps_interface* src) : kps_implement_base(src)
		{
			src->register_callback(
				std::bind(&kps_implement_hard::on_key_down, this,
					std::placeholders::_1, std::placeholders::_2),
				reinterpret_cast<kps_interface::id_t>(this));

			std::lock_guard _(src->m);
			for (const auto& [key, time] : src->records)
				sum[key]++;
		}
		~kps_implement_hard()
		{
			src->unregister_callback(reinterpret_cast<kps_interface::id_t>(this));
		}
	private:
		void on_key_down(int key, time_point)
		{
			sum[key]++;
		}
		virtual void clear_implement() override
		{
			start_index = 0;
			std::fill(sum.begin(), sum.end(), int());
			std::fill(recent_pivot.begin(), recent_pivot.end(), size_t());
		}
		virtual double calc_kps_now_implement(int key) override
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
		virtual history_array calc_kps_recent_implement(int key) override
		{
			auto get_time = [&](size_t idx) {return std::get<1>(src->records_individual[key][idx]); };
			auto now = clock::now();
			long long integral_second = std::ceil(
				std::chrono::duration_cast<std::chrono::duration<double>>(now.time_since_epoch()).count());
			now = clock::time_point(
				std::chrono::duration_cast<clock::duration>(std::chrono::seconds(integral_second)));
			while (recent_pivot[key] < src->records_individual[key].size() &&
				now - get_time(recent_pivot[key]) > 1s * (history_count + 1)) // 对答案有贡献的点的最远位置。
				recent_pivot[key]++;
			history_array ret{};
			// 双指针法。
			size_t left = recent_pivot[key];
			size_t right = recent_pivot[key];
			while (right < src->records_individual[key].size() &&
				get_time(right) < now - std::chrono::seconds(history_count))
				right++;
			for (size_t crt_time = 0; crt_time < history_count; crt_time++)
			{
				auto real_time_point_left = now - std::chrono::seconds(history_count - crt_time);
				auto real_time_point_right = real_time_point_left + 1s;
				// 初始条件：hard KPS 区间的右端点为当前时间区间的左端点。
				// 注意此时假设 right 在不处于当前时间区间的最右边。
				while (left < right && real_time_point_left - get_time(left) > 1s)
					left++;
				ret[crt_time] = right - left;
				// 用当前时间区间内的点更新。
				while (right < src->records_individual[key].size() &&
					get_time(right) < real_time_point_right)
				{
					right++;
					while (left < right && get_time(right - 1) - get_time(left) > 1s)
						left++;
					if (real_time_point_left <= get_time(right - 1))
						ret[crt_time] = std::max(ret[crt_time], static_cast<double>(right - left));
				}
			}
			return ret;
		}
	};

	class kps_calculator : public kps_interface
	{
	private:
		mutable std::mutex m;
	private:
		std::shared_ptr<kps_implement_base> implement;
	public:
		auto implement_type() const
		{
			return implement->type();
		}
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
		void clear()
		{
			std::lock_guard _(m);
			implement->clear();
		}

	public:
		/// <summary>
		/// 计算当前的 KPS。其含义取决于具体实现。
		/// </summary>
		double calc_kps_now(int key) const
		{
			std::lock_guard _(m);
			return implement->calc_kps_now(key);
		}
		/// <summary>
		/// 计算当前的 KPS。其含义取决于具体实现。
		/// </summary>
		double calc_kps_now(const std::vector<int>& keys) const
		{
			std::lock_guard _(m);
			return implement->calc_kps_now(keys);
		}
		/// <summary>
		/// 计算最近的 KPS。其含义取决于具体实现。
		/// </summary>
		history_array calc_kps_recent(int key) const
		{
			std::lock_guard _(m);
			return implement->calc_kps_recent(key);
		}
		/// <summary>
		/// 计算最近的 KPS。其含义取决于具体实现。
		/// </summary>
		history_array calc_kps_recent(const std::vector<int>& keys) const
		{
			std::lock_guard _(m);
			return implement->calc_kps_recent(keys);
		}
	};
}