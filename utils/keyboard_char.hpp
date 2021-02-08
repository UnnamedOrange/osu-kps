// Copyright (c) UnnamedOrange. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

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
		vk_slash = 0xBF, // /?
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
public:
	keyboard_char()
	{
		for (char8_t ch = u8'0'; ch <= u8'9'; ch++)
			int_to_short[ch] = { false, true, std::u8string(1, ch) };
		for (char8_t ch = u8'A'; ch <= u8'Z'; ch++)
			int_to_short[ch] = { false, true, std::u8string(1, ch) };
		for (int ch = vk_numpad0; ch <= vk_numpad9; ch++)
			int_to_short[ch] = { false, true, std::u8string(1, u8'0' + (ch - vk_numpad0)) };
		int_to_short[vk_space] = { true, true, u8"\xE75D" };
		int_to_short[vk_lshift] = { false, false, u8"lShift" };
		int_to_short[vk_rshift] = { false, false, u8"rShift" };
		int_to_short[vk_lcontrol] = { false, false, u8"lCtrl" };
		int_to_short[vk_rcontrol] = { false, false, u8"rCtrl" };
		int_to_short[vk_lalt] = { false, false, u8"lAlt" };
		int_to_short[vk_ralt] = { false, false, u8"rAlt" };
		int_to_short[vk_semicolon] = { false, true, u8";" };
		int_to_short[vk_slash] = { false, true, u8"/" };
		int_to_short[vk_lsbracket] = { false, true, u8"[" };
		int_to_short[vk_backslash] = { false, true, u8"\\" };
		int_to_short[vk_rsbracket] = { false, true, u8"]" };
		int_to_short[vk_quotes] = { false, true, u8"\'" };
		int_to_short[vk_lbutton] = { true, true, u8"\xE962" };
		int_to_short[vk_rbutton] = { true, true, u8"\xE962" };
	}
public:
	/// <summary>
	/// 获取按键的简写。
	/// </summary>
	/// <param name="vk">按键虚拟码。</param>
	/// <returns>一个 short_form 对象。</returns>
	auto to_short(int vk) const
	{
		return int_to_short.at(vk);
	}
};