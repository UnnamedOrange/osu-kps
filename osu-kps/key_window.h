// Copyright (c) UnnamedOrange. Licensed under the MIT Licence.
// See the LICENSE file in the repository root for full licence text.

#pragma once

#include <thread>
#include <format>

#include <utils/window.hpp>
#include <utils/d2d_helper.hpp>
#include <utils/keyboard_char.hpp>
#include <utils/resource_loader.hpp>

#include "config.hpp"
#include "my_multi_language.hpp"

class key_window : public window
{
	virtual INT_PTR WindowProc(HWND, UINT message, WPARAM wParam, LPARAM lParam) override
	{
		switch (message)
		{
			HANDLE_MSG(hwnd, WM_CREATE, OnCreate);
			HANDLE_MSG(hwnd, WM_DESTROY, OnDestroy);

			HANDLE_MSG(hwnd, WM_KEYDOWN, OnKey);
			HANDLE_MSG(hwnd, WM_SYSKEYDOWN, OnKey);
			HANDLE_MSG(hwnd, WM_LBUTTONDOWN, OnLButtonDown);
			HANDLE_MSG(hwnd, WM_RBUTTONDOWN, OnRButtonDown);

			HANDLE_MSG(hwnd, WM_PAINT, OnPaint);

		case WM_DPICHANGED:
		{
			RECT* const prcNewWindow = (RECT*)lParam;
			SetWindowPos(hwnd,
				NULL,
				prcNewWindow->left,
				prcNewWindow->top,
				prcNewWindow->right - prcNewWindow->left,
				prcNewWindow->bottom - prcNewWindow->top,
				SWP_NOZORDER | SWP_NOACTIVATE);
			build_scale_dep_resource();
			resize();
			break;
		}

		default:
			return DefWindowProcW(hwnd, message, wParam, lParam);
		}
		return 0;
	}
	BOOL OnCreate(HWND, LPCREATESTRUCT)
	{
		EnableWindow(GetParent(hwnd), false);

		// 窗口设置相关。
		{
			std::wstring buffer;
			buffer = std::vformat(lang["key_window.caption"], std::make_wformat_args(crt_keys));
			caption(buffer);
		}
		SetWindowLongW(hwnd, GWL_STYLE, WS_POPUPWINDOW | WS_CAPTION);
		{
			LONG ex_style;
			while (!((ex_style = GetWindowLongW(hwnd, GWL_EXSTYLE)) & WS_EX_TOPMOST))
			{
				SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
				SetWindowLongW(hwnd, GWL_EXSTYLE, ex_style | WS_EX_TOPMOST);
				std::this_thread::yield();
			}
		}

		// 绘图。
		init_d2d();

		// 调整窗口大小。
		resize();
		move_to_cursor();

		// 初始化按键。
		keys.clear();

		return TRUE;
	}
	void OnDestroy(HWND)
	{
		EnableWindow(GetParent(hwnd), true);
	}

	void OnKey(HWND hwnd, UINT vk, BOOL fDown, int cRepeat, UINT flags)
	{
		if (fDown && cRepeat == 1)
		{
			if (vk == VK_ESCAPE)
			{
				SendMessageW(hwnd, WM_CLOSE, 0, 0);
			}
			else if (cache.kc.is_supported(vk))
			{
				next(vk);
			}
			else
			{
				bool ok = true;
				switch (vk)
				{
				case VK_CONTROL:
					vk = VK_LCONTROL;
					break;
				case VK_SHIFT:
					vk = VK_LSHIFT;
					break;
				case VK_MENU:
					vk = VK_LMENU;
					break;
				default:
					ok = false;
				}
				if (ok)
				{
					if (vk != VK_LSHIFT)
					{
						// 判断是否是右功能键。
						constexpr auto EXTENDED_KEYMASK = 1 << 8;
						if (flags & EXTENDED_KEYMASK)
							vk++; // 从左键变成右键。
					}
					else
					{
						// flags = 54 表示是右 Shift。
						if (flags == 54)
							vk = VK_RSHIFT;
					}
					next(vk);
				}
			}
		}
	}
	void OnLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
	{
		next(VK_LBUTTON);
	}
	void OnRButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
	{
		next(VK_RBUTTON);
	}

	// 绘图。
	template <typename T>
	using com_ptr = d2d_helper::com_ptr<T>;
	com_ptr<ID2D1HwndRenderTarget> pRenderTarget{};
	struct cache_t
	{
		// indep
		static constexpr auto theme_color = d2d_helper::color(203u, 237u, 238u);
		static constexpr auto theme_color_half_trans = d2d_helper::color(203u, 237u, 238u, 191u);
		static constexpr auto light_active_color = d2d_helper::color(227u, 172u, 181u);
		static constexpr auto active_color = d2d_helper::color(255u, 104u, 143u);

		com_ptr<ID2D1SolidColorBrush> theme_brush;
		com_ptr<ID2D1SolidColorBrush> active_brush;

		d2d_helper::private_font_collection theme_font_collection;

		keyboard_char kc;

		// scale_dep
		com_ptr<IDWriteTextFormat> text_format_key_name;
		com_ptr<IDWriteTextFormat> text_format_key_name_small;
		com_ptr<IDWriteTextFormat> text_format_key_name_MDL2;

		void reset()
		{
			this->~cache_t();
			new(this) cache_t;
		}
	} cache;
	bool d2d_inited{};
	void init_d2d();
	void build_indep_resource();
	void build_scale_dep_resource();
	void OnPaint(HWND);

	// 绘图位置参数。
	inline static double cx_button = 52.0;
	inline static double cy_button = 72.0;
	inline static double cx_gap = 8.0;
	/// <summary>
	/// 计算窗口应有的大小。<remarks>计算时不考虑缩放，最后再乘以缩放。</remarks>
	/// </summary>
	/// <returns>(cx, cy)</returns>
	std::tuple<double, double> calc_size() const
	{
		double cx = cx_button * crt_keys + cx_gap * (crt_keys - 1);
		double cy = cy_button;
		double x = dpi();
		return { cx * x, cy * x };
	}
	/// <summary>
	/// 根据计算出的窗口大小重设窗口大小。
	/// </summary>
	void resize()
	{
		auto size = calc_size();
		auto window_rect = get_window_rect();
		auto client_rect = get_client_rect();
		int extra_width = (window_rect.right - window_rect.left) - (client_rect.right - client_rect.left);
		int extra_height = (window_rect.bottom - window_rect.top) - (client_rect.bottom - client_rect.top);
		width(std::get<0>(size) + extra_width);
		height(std::get<1>(size) + extra_height);
		pRenderTarget->Resize(D2D1::SizeU(cwidth(), cheight()));

		InvalidateRect(hwnd, nullptr, FALSE);
	}
	void move_to_cursor()
	{
		POINT p;
		GetCursorPos(&p);
		auto rect = get_window_rect();
		int left_shift = p.x - (rect.left + rect.right) / 2;
		int top_shift = p.y - (rect.top + rect.bottom) / 2;

		RECT rWorkArea = work_area();
		rWorkArea.right -= rect.right - rect.left;
		rWorkArea.bottom -= rect.bottom - rect.top;
		if (rect.left + left_shift > rWorkArea.right)
			left_shift = rWorkArea.right - rect.left;
		else if (rect.left + left_shift < rWorkArea.left)
			left_shift = rWorkArea.left - rect.left;
		if (rect.top + top_shift > rWorkArea.bottom)
			top_shift = rWorkArea.bottom - rect.top;
		else if (rect.top + top_shift < rWorkArea.top)
			top_shift = rWorkArea.top - rect.top;
		SetWindowPos(hwnd, nullptr,
			left() + left_shift, top() + top_shift,
			0, 0, SWP_NOSIZE | SWP_NOZORDER);
	}

private:
	config* cfg;
	keys_manager* k_manager;
	int crt_keys{ 4 };
private:
	std::vector<int> keys;
	void next(int key)
	{
		keys.push_back(key);
		InvalidateRect(hwnd, nullptr, FALSE);
		if (keys.size() == crt_keys)
		{
			for (unsigned i = 0; i < keys.size(); i++)
			{
				cfg->key_map(crt_keys, i, keys[i]);
				k_manager->modify_key(crt_keys, i, keys[i]);
			}
			SendMessageW(hwnd, WM_CLOSE, 0, 0);
		}
	}

public:
	key_window(config* cfg, keys_manager* k_manager) :
		cfg(cfg), k_manager(k_manager) {}

public:
	void set_crt_keys(int keys)
	{
		keys = std::max(1, std::min(static_cast<int>(keys_manager::max_key_count), keys));
		crt_keys = keys;
	}
};