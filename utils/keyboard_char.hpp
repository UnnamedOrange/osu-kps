// Copyright (c) UnnamedOrange. Licensed under the MIT Licence.
// See the LICENSE file in the repository root for full licence text.

#pragma once

#include <string>
#include <map>

class keyboard_char
{
public:
	enum vk
	{
		vk_lbutton = 0x01,
		vk_rbutton = 0x02,
		vk_mbutton = 0x04,
		vk_escape = 0x1B,
		vk_space = 0x20,
		vk_left = 0x25,
		vk_up = 0x26,
		vk_right = 0x27,
		vk_down = 0x28,
		vk_lwin = 0x5B,
		vk_rwin = 0x5C,
		vk_numpad0 = 0x60,
		vk_numpad1,
		vk_numpad2,
		vk_numpad3,
		vk_numpad4,
		vk_numpad5,
		vk_numpad6,
		vk_numpad7,
		vk_numpad8,
		vk_numpad9,
		vk_multiply, // Numpad *
		vk_add, // Numpad +
		vk_separator, // ???
		vk_subtract, // Numpad -
		vk_decimal, // Numpad .
		vk_divide, // Numpad /
		vk_f1 = 0x70,
		vk_f2,
		vk_f3,
		vk_f4,
		vk_f5,
		vk_f6,
		vk_f7,
		vk_f8,
		vk_f9,
		vk_f10,
		vk_f11,
		vk_f12,
		vk_lshift = 0xA0,
		vk_rshift,
		vk_lcontrol,
		vk_rcontrol,
		vk_lalt,
		vk_ralt,
		vk_semicolon = 0xBA, // ;:
		vk_plus,
		vk_comma, // ,
		vk_minus,
		vk_period, // .
		vk_slash, // /?
		vk_lsbracket = 0xDB, // [{
		vk_backslash, // \|'
		vk_rsbracket, // ]}
		vk_quotes, // '"
	};

public:
	struct short_form
	{
		bool need_MDL2{};
		bool is_single{};
		std::u8string key;
	};
private:
	std::map<int, short_form> int_to_short;
	std::map<int, std::u8string> int_to_full;
	std::map<std::u8string, int> full_to_int;
public:
	keyboard_char()
	{
		int_to_short[0] = { false, true, u8" " };
		for (char8_t ch = u8'0'; ch <= u8'9'; ch++)
			int_to_short[ch] = { false, true, std::u8string(1, ch) };
		for (char8_t ch = u8'A'; ch <= u8'Z'; ch++)
			int_to_short[ch] = { false, true, std::u8string(1, ch) };
		for (int ch = vk_numpad0; ch <= vk_numpad9; ch++)
			int_to_short[ch] = { false, false, u8"Num" + std::u8string(1, u8'0' + static_cast<char8_t>(ch - vk_numpad0)) };

		int_to_short[vk_space] = { true, true, u8"\xE75D" };
		int_to_short[vk_left] = { true, false, u8"\xE00E" };
		int_to_short[vk_up] = { true, false, u8"\xE010" };
		int_to_short[vk_right] = { true, false, u8"\xE00F" };
		int_to_short[vk_down] = { true, false, u8"\xE011" };
		int_to_short[vk_multiply] = { false, false, u8"Num*" };
		int_to_short[vk_add] = { false, false, u8"Num+" };
		int_to_short[vk_subtract] = { false, false, u8"Num-" };
		int_to_short[vk_decimal] = { false, false, u8"Num." };
		int_to_short[vk_divide] = { false, false, u8"Num/" };
		int_to_short[vk_lshift] = { false, false, u8"lShift" };
		int_to_short[vk_rshift] = { false, false, u8"rShift" };
		int_to_short[vk_lcontrol] = { false, false, u8"lCtrl" };
		int_to_short[vk_rcontrol] = { false, false, u8"rCtrl" };
		int_to_short[vk_lalt] = { false, false, u8"lAlt" };
		int_to_short[vk_ralt] = { false, false, u8"rAlt" };
		int_to_short[vk_semicolon] = { false, true, u8";" };
		int_to_short[vk_plus] = { false, true, u8"+" };
		int_to_short[vk_comma] = { false, true, u8"," };
		int_to_short[vk_minus] = { false, true, u8"-" };
		int_to_short[vk_period] = { false, true, u8"." };
		int_to_short[vk_slash] = { false, true, u8"/" };
		int_to_short[vk_lsbracket] = { false, true, u8"[" };
		int_to_short[vk_backslash] = { false, true, u8"\\" };
		int_to_short[vk_rsbracket] = { false, true, u8"]" };
		int_to_short[vk_quotes] = { false, true, u8"\'" };
		int_to_short[vk_lbutton] = { true, true, u8"\xE962" };
		int_to_short[vk_rbutton] = { true, true, u8"\xE962" };

		int_to_full[0] = u8"null";
		for (char8_t ch = u8'0'; ch <= u8'9'; ch++)
			int_to_full[ch] = std::u8string(1, ch);
		for (char8_t ch = u8'A'; ch <= u8'Z'; ch++)
			int_to_full[ch] = std::u8string(1, ch);
		for (int ch = vk_numpad0; ch <= vk_numpad9; ch++)
			int_to_full[ch] = u8"Numpad " + std::u8string(1, u8'0' + static_cast<char8_t>(ch - vk_numpad0));
		int_to_full[vk_space] = u8"Space";
		int_to_full[vk_left] = u8"Left";
		int_to_full[vk_up] = u8"Up";
		int_to_full[vk_right] = u8"Right";
		int_to_full[vk_down] = u8"Down";
		int_to_full[vk_multiply] = u8"Numpad *";
		int_to_full[vk_add] = u8"Numpad +";
		int_to_full[vk_subtract] = u8"Numpad -";
		int_to_full[vk_decimal] = u8"Numpad .";
		int_to_full[vk_divide] = u8"Numpad /";
		int_to_full[vk_lshift] = u8"left Shift";
		int_to_full[vk_rshift] = u8"right Shift";
		int_to_full[vk_lcontrol] = u8"left Ctrl";
		int_to_full[vk_rcontrol] = u8"right Ctrl";
		int_to_full[vk_lalt] = u8"left Alt";
		int_to_full[vk_ralt] = u8"right Alt";
		int_to_full[vk_semicolon] = u8";";
		int_to_full[vk_plus] = u8"+";
		int_to_full[vk_comma] = u8",";
		int_to_full[vk_minus] = u8"-";
		int_to_full[vk_period] = u8".";
		int_to_full[vk_slash] = u8"/";
		int_to_full[vk_lsbracket] = u8"[";
		int_to_full[vk_backslash] = u8"\\";
		int_to_full[vk_rsbracket] = u8"]";
		int_to_full[vk_quotes] = u8"\'";
		int_to_full[vk_lbutton] = u8"left mouse button";
		int_to_full[vk_rbutton] = u8"right mouse button";

		for (const auto& [key, value] : int_to_full)
			full_to_int[value] = key;
	}
public:
	/// <summary>
	/// 检查按键是否被该头文件支持。
	/// </summary>
	/// <param name="vk">按键虚拟码。</param>
	bool is_supported(int vk) const
	{
		return int_to_full.count(vk);
	}
	/// <summary>
	/// 检查按键是否被该头文件支持。
	/// </summary>
	/// <param name="full">按键全称。</param>
	bool is_supported(const std::u8string full) const
	{
		return full_to_int.count(full);
	}
	/// <summary>
	/// 获取按键的简写。
	/// </summary>
	/// <param name="vk">按键虚拟码。</param>
	/// <returns>一个 short_form 对象。</returns>
	auto to_short(int vk) const
	{
		return int_to_short.at(vk);
	}
	/// <summary>
	/// 获取按键的全称。
	/// </summary>
	/// <param name="vk">按键虚拟码。</param>
	/// <returns>一个 u8string。</returns>
	auto to_full(int vk) const
	{
		return int_to_full.at(vk);
	}
	/// <summary>
	/// 获取按键的虚拟码
	/// </summary>
	/// <param name="full">按键全称。</param>
	auto to_int(const std::u8string& full) const
	{
		return full_to_int.at(full);
	}
};