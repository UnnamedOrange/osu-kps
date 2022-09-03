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

#pragma region Future implementation of osu-memory;
#include <array>
namespace osu_memory
{
	/**
	 * @breif An optional uint8_t standing for a byte. Empty means any byte.
	 */
	struct pattern_unit_t
	{
		uint8_t byte;
		bool mask;
	};

	/**
	 * @brief Array of pattern_unit_t. Should be used as a template paramter.
	 */
	template <size_t raw_size>
		requires (raw_size % 3 == 0)
	class pattern_t : public std::array<pattern_unit_t, raw_size / 3>
	{
	private:
		/**
		 * @brief Convert a hexadecimal digit character to uint8_t.
		 * Returns 0 if invalid.
		 */
		static constexpr uint8_t to_digit(char ch) noexcept
		{
			if ('0' <= ch && ch <= '9')
				return ch - '0';
			else if ('A' <= ch && ch <= 'F')
				return ch - 'A' + 10;
			else if ('a' <= ch && ch <= 'f')
				return ch - 'a' + 10;
			else
				return 0;
		}
		/**
		 * @brief Concatenate two hexadecimal digits to one uint8_t.
		 * Returns 0 if invalid.
		 */
		static constexpr uint8_t concat_digit(uint8_t low_digit, uint8_t high_digit) noexcept
		{
			if (low_digit > 0xF || high_digit > 0xF)
				return 0;
			return (high_digit << 4) | low_digit;
		}

	public:
		constexpr pattern_t(const char(&pattern_str)[raw_size]) noexcept
		{
			for (size_t i = 0; i < this->size(); i++)
			{
				char high_ch = pattern_str[i * 3 + 0];
				char low_ch = pattern_str[i * 3 + 1];
				if (low_ch == '?' || high_ch == '?')
				{
					(*this)[i] = {
						.byte = 0,
						.mask = true
					};
				}
				else
				{
					(*this)[i] = {
						.byte = concat_digit(to_digit(low_ch), to_digit(high_ch)),
						.mask = false
					};
				}
			}
		}
	};

	template <size_t key_name_length>
	struct key_name_t
	{
		std::array<char, key_name_length> name;
		constexpr key_name_t(const char(&name)[key_name_length])
		{
			for (size_t i = 0; i < key_name_length; i++)
				this->name[i] = name[i];
		}
	};

	template <pattern_t pattern>
	class test
	{};

	/**
	 * @brief Scan the pattern given by template.
	 *
	 * @tparam p A pattern_t. Recommended to use string literal.
	 * @tparam pointer_t Pointer type. Default is uint32_t. Use uint64_t for a 64-bit process.
	 *
	 * @param process A process object with `read_memory` method.
	 * @return First address of the first byte of the pattern. Returns std::nullopt if failed for any reason.
	 */
	template <pattern_t pattern, typename pointer_t = uint32_t>
	inline std::optional<pointer_t> scan_pattern(const osu_memory::os::readable_process& process)
	{
		if (process.empty())
			return std::nullopt;

		PVOID min_address;
		PVOID max_address;
		{
			SYSTEM_INFO sys_info;
			GetSystemInfo(&sys_info);
			min_address = sys_info.lpMinimumApplicationAddress;
			max_address = sys_info.lpMaximumApplicationAddress;
		}

		PVOID crt_address = min_address;
		while (crt_address < max_address)
		{
			MEMORY_BASIC_INFORMATION mem_info;
			if (!VirtualQueryEx(process.native_handle(), crt_address, &mem_info, sizeof(mem_info)))
				return std::nullopt;

			do // Not a loop.
			{
				if ((mem_info.Protect & PAGE_EXECUTE_READ) && mem_info.State == MEM_COMMIT)
				{
					// Do matches here.
					auto region_data = process.read_memory(mem_info.BaseAddress, mem_info.RegionSize);
					if (!region_data)
						continue; // Jump out of do while (false).

					for (size_t i = 0; i + pattern.size() - 1 < (*region_data).size(); i++)
					{
						bool ok = true;
						for (size_t j = 0; j < pattern.size(); j++)
							if (!(pattern[j].mask || (*region_data)[i + j] == pattern[j].byte))
							{
								ok = false;
								break;
							}
						if (ok)
							return uintptr_t(mem_info.BaseAddress) + i;
					}
				}
			} while (false);
			crt_address = PVOID(uintptr_t(crt_address) + mem_info.RegionSize);
		}
		return std::nullopt;
	}
}
#pragma endregion

namespace kps
{
	/// <summary>
	/// 使用 osu-memory 的底层类实现的 Malody 按键监视器。不支持单一按键。
	/// </summary>
	class key_monitor_malody final : public key_monitor_base
	{
	private:
		using key_monitor_base::_on_llkey_down;
		using key_monitor_base::_on_llkey_up;

	private:
		static constexpr osu_memory::pattern_t pattern{ "8B 0D ?? ?? ?? ?? E8 ?? ?? ?? ?? 8B C8 E8" };
		static constexpr std::array offsets = std::to_array<int32_t>({ 0x2, 0x0, 0x228, 0x25C }); // 先加后读。
		static constexpr auto last_offset = 0x4B0;

	private:
		bool exit{ false };
		std::thread t;
		void thread_proc()
		{
			using namespace osu_memory;
			os::readable_process malody_process;
			uint32_t base_address{};
			while (!exit)
			{
				// 读按键状态。失败则停止。
				bool ok = false;
				std::array<bool, 9> key_down;
				do
				{
					// 更新 process 对象状态。
					if (!malody_process.still_active())
					{
						auto list = os::readable_process::open(L"malody.exe", PROCESS_VM_READ);
						if (list.empty())
						{
							using namespace std::literals;
							std::this_thread::sleep_for(100ms);
							break;
						}
						malody_process = std::move(list[0]);
						base_address = 0;
					}
					// 更新基地址。
					if (!base_address)
					{
						auto result = scan_pattern<pattern>(malody_process);
						if (!result)
							break;
						base_address = *result;
					}
					// 追溯数组地址。
					uint32_t array_address = base_address;
					for (auto offset : offsets)
					{
						auto result = malody_process.read_memory<uint32_t>(array_address + offset);
						if (!result || !*result)
							break;
						array_address = *result;
					}
					array_address += last_offset;

					// 读取按键状态。
					bool ok_all_keys = true;
					for (size_t i = 0; i < key_down.size(); i++)
					{
						auto result = malody_process.read_memory<uint8_t>(array_address + static_cast<int32_t>(i));
						if (result)
							key_down[i] = *result;
						else
						{
							ok_all_keys = false;
							break;
						}
					}
					if (!ok_all_keys)
						break;

					ok = true;
				} while (false);

				auto now = clock::now();
				if (!ok)
				{
					for (int i = 0; i < 256; i++)
						_on_llkey_up(i, now);
					continue;
				}

				for (size_t i = 0; i < key_down.size(); i++)
					if (key_down[i])
						_on_llkey_down(i, now);
					else
						_on_llkey_up(i, now);
			}
		}

	public:
		key_monitor_malody()
		{
			t = std::thread(&key_monitor_malody::thread_proc, this);
		}
		~key_monitor_malody()
		{
			exit = true;
			t.join();
		}
	};
}