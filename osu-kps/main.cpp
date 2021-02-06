// Copyright (c) UnnamedOrange. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#include <functional>

#include <Windows.h>
#undef min
#undef max

#include <utils/code_conv.hpp>
#include <utils/window.hpp>
#include <utils/timer_thread.hpp>
#include <utils/d2d_helper.hpp>

#include "integrated_kps.hpp"
#include "keys_manager.hpp"

/* void OnMoving(HWND hwnd, RECT* pRect) */
#define HANDLE_WM_MOVING(hwnd, wParam, lParam, fn)\
	((fn)((hwnd), (RECT*)(lParam)), 0L)
#define FORWARD_WM_MOVING(hwnd, pRect, fn)\
	(void)(fn)((hwnd), WM_MOVING, 0L, (LPARAM)(pRect))

class main_window : public window
{
	virtual INT_PTR WindowProc(HWND, UINT message, WPARAM wParam, LPARAM lParam) override
	{
		switch (message)
		{
			HANDLE_MSG(hwnd, WM_CREATE, OnCreate);
			HANDLE_MSG(hwnd, WM_DESTROY, OnDestroy);

			HANDLE_MSG(hwnd, WM_LBUTTONDOWN, OnLButtonDown);
			HANDLE_MSG(hwnd, WM_NCLBUTTONDOWN, OnNCLButtonDown);
			HANDLE_MSG(hwnd, WM_MOVING, OnMoving);

			HANDLE_MSG(hwnd, WM_RBUTTONDOWN, OnRButtonDown);
			HANDLE_MSG(hwnd, WM_COMMAND, OnCommand);

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
		// 窗口信息相关。
		caption(L"osu!kps");
		SetWindowLongW(hwnd, GWL_STYLE, WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_POPUP | WS_SYSMENU); // 无边框窗口。
		SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

		// 右键菜单。
		create_menu();

		// 绘图。
		init_d2d();

		// 调整窗口大小。
		resize();

		return TRUE;
	}
	void OnDestroy(HWND)
	{
		// 右键菜单。
		DestroyMenu(hMenu);

		// 绘图。
		destruct_d2d();

		PostQuitMessage(0);
	}

	// 禁止窗口移出屏幕相关信息。
	mutable POINT pMouse{};
	mutable RECT rectWnd{};
	void OnLButtonDown(HWND, BOOL fDoubleClick, int x, int y, UINT keyFlags)
	{
		UNREFERENCED_PARAMETER(keyFlags);
		if (fDoubleClick)
		{
			// TODO: 实现双击修改配置等功能。
		}
		else
		{
			PostMessageW(hwnd, WM_NCLBUTTONDOWN, HTCAPTION, MAKELPARAM(x, y));
		}
	}
	void OnNCLButtonDown(HWND, BOOL fDoubleClick, int x, int y, UINT codeHitTest)
	{
		// 禁止窗口移出屏幕相关信息。
		GetWindowRect(hwnd, &rectWnd);
		GetCursorPos(&pMouse);
		FORWARD_WM_NCLBUTTONDOWN(hwnd, fDoubleClick, x, y, codeHitTest, DefWindowProcW);
		PostMessageW(hwnd, WM_LBUTTONUP, 0, 0);
	}
	void OnMoving(HWND, RECT* pRect)
	{
		RECT& r = *pRect;
		POINT p;
		GetCursorPos(&p);
		RECT rWorkArea = work_area();
		int nWidth = r.right - r.left;
		int nHeight = r.bottom - r.top; // 获取窗口大小。
		rWorkArea.right -= nWidth; // 将窗口可能出现的位置更改为窗口左上角的点可能出现的位置。
		rWorkArea.bottom -= nHeight;

		r.left = rectWnd.left + (p.x - pMouse.x); // 根据鼠标位置初步得出窗口左上角位置。
		r.top = rectWnd.top + (p.y - pMouse.y);

		POINT pCorner = { r.left, r.top }; // 窗口左上角的点。
		if (pCorner.x < rWorkArea.left)
		{
			r.left = rWorkArea.left;
		}
		if (pCorner.y < rWorkArea.top)
		{
			r.top = rWorkArea.top;
		}
		if (pCorner.x > rWorkArea.right)
		{
			r.left = rWorkArea.right;
		}
		if (pCorner.y > rWorkArea.bottom)
		{
			r.top = rWorkArea.bottom;
		}
		r.right = r.left + nWidth; // 根据左上角位置和大小调整右下角。
		r.bottom = r.top + nHeight;
	}

	// 右键菜单。
	HMENU hMenu{};
	HMENU menus_button_count{};
	/// <summary>
	/// 创建或重建菜单。
	/// </summary>
	void create_menu()
	{
		if (hMenu)
			DestroyMenu(hMenu);

		hMenu = CreateMenu();
		HMENU hMenuPopup = CreateMenu();
		AppendMenuW(hMenu, MF_POPUP, reinterpret_cast<UINT_PTR>(hMenuPopup), nullptr);
		// menus_button_count
		{
			menus_button_count = CreateMenu();
			for (int i = 1; i <= id_max_button_count; i++)
				AppendMenuW(menus_button_count, MF_STRING, i,
					(std::to_wstring(i) + L"\tCtrl + Shift + " + std::to_wstring(i % 10)).c_str());

			AppendMenuW(hMenuPopup, MF_POPUP, reinterpret_cast<UINT_PTR>(menus_button_count), L"Button count");
		}
		// exit
		{
			AppendMenuW(hMenuPopup, MF_STRING, id_exit, L"Exit");
		}

		// 勾选当前按键数量。
		CheckMenuItem(hMenuPopup, k_manager.get_button_count(), MF_CHECKED);
	}
	void OnRButtonDown(HWND, BOOL fDoubleClick, int x, int y, UINT keyFlags)
	{
		UNREFERENCED_PARAMETER(keyFlags);
		if (!fDoubleClick)
		{
			POINT p{ x, y };
			ClientToScreen(hwnd, &p);
			TrackPopupMenu(GetSubMenu(hMenu, 0), TPM_RIGHTBUTTON, p.x, p.y,
				NULL, hwnd, nullptr);
		}
	}
	void OnCommand(HWND, int id, HWND hwndCtl, UINT codeNotify)
	{
		UNREFERENCED_PARAMETER(codeNotify);
		if (!hwndCtl) // 如果是菜单。
		{
			if (1 <= id && id <= id_max_button_count) // 修改按键个数。
				change_button_count(id);
			else
				switch (id)
				{
				case id_exit:
				{
					PostMessageW(hwnd, WM_CLOSE, 0, 0);
					break;
				}
				default:
					break;
				}
		}
	}

	// 绘图。
	ID2D1HwndRenderTarget* pRenderTarget{};
	void init_d2d()
	{
		if (FAILED(d2d_helper::factory::d2d1()->CreateHwndRenderTarget(D2D1::RenderTargetProperties(),
			D2D1::HwndRenderTargetProperties(hwnd, D2D1::SizeU(width(), height())),
			&pRenderTarget)))
			throw std::runtime_error("Fail to CreateHwndRenderTarget.");
		pRenderTarget->SetDpi(USER_DEFAULT_SCREEN_DPI, USER_DEFAULT_SCREEN_DPI); // 自己处理高 DPI。
	}
	void destruct_d2d()
	{
		d2d_helper::release(pRenderTarget);
	}
	timer_thread _tt{ [this] { InvalidateRect(hwnd, nullptr, FALSE); }, 1000 / 60 };
	void OnPaint(HWND);

	// 绘图位置参数。
	inline static double cx_button = 52.0;
	inline static double cy_button = 72.0;
	inline static double cx_gap = 8.0;
	inline static double cy_separator = 8.0;
	inline static double cx_statistics = 232.0;
	inline static double cy_statistics = 120.0;
	/// <summary>
	/// 计算窗口应有的大小。<remarks>计算时不考虑缩放，最后再乘以缩放。</remarks>
	/// </summary>
	/// <returns>(cx, cy)</returns>
	std::tuple<double, double> calc_size() const
	{
		double cx = cx_button * k_manager.get_button_count() +
			cx_gap * (k_manager.get_button_count() - 1);
		double cy = cy_button;
		cx = std::max(cx, cx_statistics);
		cy += cy_separator + cy_statistics;
		return { cx * dpi() * scale, cy * dpi() * scale };
	}
	/// <summary>
	/// 根据计算出的窗口大小重设窗口大小。优先保证左上角不动。
	/// </summary>
	void resize()
	{
		auto size = calc_size();
		width(static_cast<int>(std::get<0>(size)));
		height(static_cast<int>(std::get<1>(size)));
		pRenderTarget->Resize(D2D1::SizeU(width(), height()));

		RECT rWorkArea = work_area();
		rWorkArea.right -= width();
		rWorkArea.bottom -= height();
		int left_shift{};
		int top_shift{};
		if (left() > rWorkArea.right)
			left_shift = left() - rWorkArea.right;
		if (top() > rWorkArea.bottom)
			top_shift = top() - rWorkArea.bottom;
		SetWindowPos(hwnd, nullptr,
			left() - left_shift, top() - top_shift,
			0, 0, SWP_NOSIZE | SWP_NOZORDER);

		InvalidateRect(hwnd, nullptr, FALSE);
	}

public:
	// 菜单项 id。
	enum
	{
		id_max_button_count = keys_manager::max_key_count,
		id_exit,
	};

public:
	keys_manager k_manager;
	kps::kps kps{ std::bind(&keys_manager::update_on_key_down, &k_manager, std::placeholders::_1, std::placeholders::_2) };
	// 改变当前按键个数。
	void change_button_count(int new_count)
	{
		CheckMenuItem(hMenu, k_manager.get_button_count(), MF_UNCHECKED);
		k_manager.set_button_count(new_count);
		CheckMenuItem(hMenu, k_manager.get_button_count(), MF_CHECKED);
		resize();
	}

public:
	double scale{ 1 }; // 绘图时的额外比例因子。
};

void main_window::OnPaint(HWND)
{
	pRenderTarget->BeginDraw();
	pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Black));
	double x = dpi() * scale; // 总比例因子。
	auto theme_color = D2D1::ColorF(203.0 / 255, 237.0 / 255, 238.0 / 255);
	{
		using namespace d2d_helper;
		// 画按键框。
		for (int i = 0; i < k_manager.get_button_count(); i++)
		{
			auto original_rect = D2D1::RectF(
				i * (cx_button + cx_gap) * x,
				0.0F,
				(i * (cx_button + cx_gap) + cx_button) * x,
				cy_button * x); // 框对应矩形。
			double stroke_width = 2 * x;
			auto draw_rect = original_rect; // 使用 DrawRectangle 时对应的矩形。
			draw_rect.left += stroke_width / 2;
			draw_rect.right -= stroke_width / 2;
			draw_rect.top += stroke_width / 2;
			draw_rect.bottom -= stroke_width / 2;
			auto draw_rounded_rect = D2D1::RoundedRect(draw_rect, 4.0 * x, 4.0 * x);

			auto number_rect = original_rect; // 每个框的数字对应的矩形。
			number_rect.top += (number_rect.bottom - number_rect.top) * 5 / 8;

			// 按键的发光效果。

			// 写字。
			{
				IDWriteTextFormat* text_format_number{};
				ID2D1SolidColorBrush* brush{};
				factory::dwrite()->CreateTextFormat(
					L"Segoe UI",
					nullptr,
					DWRITE_FONT_WEIGHT_REGULAR,
					DWRITE_FONT_STYLE_NORMAL,
					DWRITE_FONT_STRETCH_NORMAL,
					20.0 * x,
					L"",
					&text_format_number);
				text_format_number->SetTextAlignment(DWRITE_TEXT_ALIGNMENT::DWRITE_TEXT_ALIGNMENT_CENTER);
				text_format_number->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
				pRenderTarget->CreateSolidColorBrush(theme_color, &brush);

				int k_now = kps.calc_kps_now(k_manager.get_keys()[i]); // 当前框对应 kps。
				auto str = std::to_wstring(k_now);

				pRenderTarget->DrawTextW(str.c_str(), str.length(), text_format_number,
					number_rect, brush);
				release(brush);
				release(text_format_number);
			}

			// 最外层的框。
			{
				ID2D1SolidColorBrush* brush{};
				pRenderTarget->CreateSolidColorBrush(theme_color, &brush);
				pRenderTarget->DrawRoundedRectangle(draw_rounded_rect, brush, stroke_width);
				release(brush);
			}
		}
	}
	pRenderTarget->EndDraw();
	ValidateRect(hwnd, nullptr);
}

int APIENTRY wWinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPWSTR, _In_ int)
{
	// 设置高 DPI 支持。
	SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
	// 单例。
	HANDLE hMutex = CreateMutexW(nullptr, FALSE, L"Global\\osu!kps-302F3137-DEA2-472F-9BC5-F078331E5576");
	DWORD last_error = GetLastError();
	if (last_error == ERROR_ALREADY_EXISTS ||
		last_error == ERROR_ACCESS_DENIED && !hMutex)
	{
		if (hMutex)
			CloseHandle(hMutex);
		return 0;
	}
	else if (!hMutex)
		return 1;

	// 窗口与主循环。
	{
		main_window wnd;
		wnd.create();
		wnd.message_loop();
	}

	// 关闭单例中用到的句柄。
	CloseHandle(hMutex);
	return 0;
}