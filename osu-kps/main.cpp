// Copyright (c) UnnamedOrange. Licensed under the MIT Licence.
// See the LICENSE file in the repository root for full licence text.

#include <functional>

#include <Windows.h>
#undef min
#undef max
#include "resource.h"

#include <utils/code_conv.hpp>
#include <utils/window.hpp>
#include <utils/timer_thread.hpp>
#include <utils/d2d_helper.hpp>
#include <utils/resource_loader.hpp>
#include <utils/keyboard_char.hpp>

#include "integrated_kps.hpp"
#include "keys_manager.hpp"

/* void OnMoving(HWND hwnd, RECT* pRect) */
#define HANDLE_WM_MOVING(hwnd, wParam, lParam, fn)\
	((fn)((hwnd), (RECT*)(lParam)), 0L)
#define FORWARD_WM_MOVING(hwnd, pRect, fn)\
	(void)(fn)((hwnd), WM_MOVING, 0L, (LPARAM)(pRect))

using namespace d2d_helper;

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
		// 窗口信息相关。
		caption(L"osu-kps");
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
		RECT rWorkArea = monitor_area();
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
	// 菜单项 id。
	enum
	{
		id_max_button_count = keys_manager::max_key_count,
		id_reset_total,
		id_reset_max,
		id_reset_graph,
		id_reset_all,
		id_exit,
		id_zoom_half,
		id_zoom_1,
		id_zoom_2,
		id_zoom_3,
		id_method_hard,
		id_show_buttons,
		id_show_statistics,
		id_show_graph,
	};
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
			HMENU menus_button_count = CreateMenu();
			for (int i = 1; i <= id_max_button_count; i++)
				AppendMenuW(menus_button_count, MF_STRING, i,
					std::to_wstring(i).c_str());

			AppendMenuW(hMenuPopup, MF_POPUP, reinterpret_cast<UINT_PTR>(menus_button_count), L"Button count");
		}
		// menus_reset
		{
			HMENU menus_reset = CreateMenu();
			AppendMenuW(menus_reset, MF_STRING, id_reset_total, L"Total keys");
			AppendMenuW(menus_reset, MF_STRING, id_reset_max, L"Max KPS");
			AppendMenuW(menus_reset, MF_STRING, id_reset_graph, L"KPS graph");
			AppendMenuW(menus_reset, MF_STRING, id_reset_all, L"All");

			AppendMenuW(hMenuPopup, MF_POPUP, reinterpret_cast<UINT_PTR>(menus_reset), L"Reset");
		}
		// menus_kps_method
		{
			HMENU menus_kps_method = CreateMenu();
			AppendMenuW(menus_kps_method, MF_STRING, id_method_hard, L"Hard");

			AppendMenuW(hMenuPopup, MF_POPUP, reinterpret_cast<UINT_PTR>(menus_kps_method), L"KPS method");
		}
		AppendMenuW(hMenuPopup, MF_SEPARATOR, NULL, nullptr);
		// menus_show
		{
			HMENU menus_show = CreateMenu();
			AppendMenuW(menus_show, MF_STRING, id_show_buttons, L"Buttons");
			AppendMenuW(menus_show, MF_STRING, id_show_statistics, L"Statistics");
			AppendMenuW(menus_show, MF_STRING, id_show_graph, L"Graph");

			AppendMenuW(hMenuPopup, MF_POPUP, reinterpret_cast<UINT_PTR>(menus_show), L"Show");
		}
		// menus_zoom
		{
			HMENU menus_zoom = CreateMenu();
			AppendMenuW(menus_zoom, MF_STRING, id_zoom_half, L"0.75x");
			AppendMenuW(menus_zoom, MF_STRING, id_zoom_1, L"1x");
			AppendMenuW(menus_zoom, MF_STRING, id_zoom_2, L"2x");
			AppendMenuW(menus_zoom, MF_STRING, id_zoom_3, L"3x");

			AppendMenuW(hMenuPopup, MF_POPUP, reinterpret_cast<UINT_PTR>(menus_zoom), L"Zoom");
		}
		// exit
		{
			AppendMenuW(hMenuPopup, MF_STRING, id_exit, L"Exit");
		}

		// 勾选当前按键数量。
		CheckMenuItem(hMenu, k_manager.get_button_count(), MF_CHECKED);
		// 勾选当前实现方式。
		switch (kps.implement_type())
		{
		case kps::kps_implement_type::kps_implement_type_hard:
		{
			CheckMenuItem(hMenu, id_method_hard, MF_CHECKED);
			break;
		}
		}
		// 勾选当前显示内容。
		if (show_buttons)
			CheckMenuItem(hMenu, id_show_buttons, MF_CHECKED);
		if (show_statistics)
			CheckMenuItem(hMenu, id_show_statistics, MF_CHECKED);
		if (show_graph)
			CheckMenuItem(hMenu, id_show_graph, MF_CHECKED);
		// 勾选当前缩放比例。
		if (scale == 1)
			CheckMenuItem(hMenu, id_zoom_1, MF_CHECKED);
		else if (scale == 2)
			CheckMenuItem(hMenu, id_zoom_2, MF_CHECKED);
		else if (scale == 3)
			CheckMenuItem(hMenu, id_zoom_3, MF_CHECKED);
		else
			CheckMenuItem(hMenu, id_zoom_half, MF_CHECKED);
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
			else if (id_zoom_half <= id && id <= id_zoom_3)
			{
				constexpr double t[]{ 0.75, 1, 2, 3 };
				change_scale(t[id - id_zoom_half]);
			}
			else if (id_method_hard <= id && id <= id_method_hard)
				change_implement(static_cast<kps::kps_implement_type>(id - id_method_hard));
			else
				switch (id)
				{
				case id_reset_total:
				{
					k_manager.clear_total_count();
					break;
				}
				case id_reset_max:
				{
					k_manager.clear_max_kps();
					break;
				}
				case id_reset_graph:
				{
					kps.clear();
					break;
				}
				case id_reset_all:
				{
					k_manager.clear_total_count();
					k_manager.clear_max_kps();
					kps.clear();
					break;
				}
				case id_show_buttons:
				{
					change_show_buttons(!show_buttons);
					break;
				}
				case id_show_statistics:
				{
					change_show_statistics(!show_statistics);
					break;
				}
				case id_show_graph:
				{
					change_show_graph(!show_graph);
					break;
				}
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
	com_ptr<ID2D1HwndRenderTarget> pRenderTarget{};
	struct cache_type
	{
		// indep
		static constexpr auto theme_color = color(203u, 237u, 238u);
		static constexpr auto light_active_color = color(227u, 172u, 181u);
		static constexpr auto active_color = color(255u, 104u, 143u);

		keyboard_char kc;

		com_ptr<ID2D1SolidColorBrush> theme_brush;
		private_font_collection theme_font_collection;

		// scale_dep
		com_ptr<IDWriteTextFormat> text_format_key_name;
		com_ptr<IDWriteTextFormat> text_format_key_name_small;
		com_ptr<IDWriteTextFormat> text_format_key_name_MDL2;
		com_ptr<IDWriteTextFormat> text_format_number;
		com_ptr<IDWriteTextFormat> text_format_statistics;
		com_ptr<IDWriteTextFormat> text_format_total_keys;

		com_ptr<ID2D1LinearGradientBrush> graph_brush;
	} cache;
	void init_d2d()
	{
		if (FAILED(factory::d2d1()->CreateHwndRenderTarget(D2D1::RenderTargetProperties(),
			D2D1::HwndRenderTargetProperties(hwnd, D2D1::SizeU(width(), height())),
			pRenderTarget.reset_and_get_address())))
			throw std::runtime_error("Fail to CreateHwndRenderTarget.");
		pRenderTarget->SetDpi(USER_DEFAULT_SCREEN_DPI, USER_DEFAULT_SCREEN_DPI); // 自己处理高 DPI。

		build_indep_resource();
		build_scale_dep_resource();
	}
	void build_indep_resource()
	{
		pRenderTarget->CreateSolidColorBrush(cache.theme_color,
			cache.theme_brush.reset_and_get_address());

		cache.theme_font_collection.from_font_data(
			resource_loader::load(MAKEINTRESOURCEW(IDR_Exo2_Regular), L"OTF"));
	}
	void build_scale_dep_resource()
	{
		double x = dpi() * scale;

		auto create_text_format = [&](
			const wchar_t* font_family_name,
			IDWriteFontCollection* font_collection,
			FLOAT fontSize,
			IDWriteTextFormat** text_format)
		{
			return factory::dwrite()->CreateTextFormat(
				font_family_name,
				font_collection,
				DWRITE_FONT_WEIGHT_REGULAR,
				DWRITE_FONT_STYLE_NORMAL,
				DWRITE_FONT_STRETCH_NORMAL,
				fontSize,
				L"",
				text_format);
		};
		// text_format_number
		{
			create_text_format(
				cache.theme_font_collection.get_family_names()[0].c_str(),
				cache.theme_font_collection.get(),
				18.0 * x,
				cache.text_format_number.reset_and_get_address());
			cache.text_format_number->SetTextAlignment(DWRITE_TEXT_ALIGNMENT::DWRITE_TEXT_ALIGNMENT_CENTER);
			cache.text_format_number->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
		}
		// text_format_key_name
		{
			create_text_format(
				cache.theme_font_collection.get_family_names()[0].c_str(),
				cache.theme_font_collection.get(),
				24.0 * x,
				cache.text_format_key_name.reset_and_get_address());
			cache.text_format_key_name->SetTextAlignment(DWRITE_TEXT_ALIGNMENT::DWRITE_TEXT_ALIGNMENT_CENTER);
			cache.text_format_key_name->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

			create_text_format(
				cache.theme_font_collection.get_family_names()[0].c_str(),
				cache.theme_font_collection.get(),
				16.0 * x,
				cache.text_format_key_name_small.reset_and_get_address());
			cache.text_format_key_name_small->SetTextAlignment(DWRITE_TEXT_ALIGNMENT::DWRITE_TEXT_ALIGNMENT_CENTER);
			cache.text_format_key_name_small->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

			create_text_format(
				L"Segoe MDL2 Assets",
				nullptr,
				24.0 * x,
				cache.text_format_key_name_MDL2.reset_and_get_address());
			cache.text_format_key_name_MDL2->SetTextAlignment(DWRITE_TEXT_ALIGNMENT::DWRITE_TEXT_ALIGNMENT_CENTER);
			cache.text_format_key_name_MDL2->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
		}
		// text_format_statistics, text_format_total_keys
		{
			create_text_format(
				cache.theme_font_collection.get_family_names()[0].c_str(),
				cache.theme_font_collection.get(),
				20.0 * x,
				cache.text_format_statistics.reset_and_get_address());
			create_text_format(
				cache.theme_font_collection.get_family_names()[0].c_str(),
				cache.theme_font_collection.get(),
				15.0 * x,
				cache.text_format_total_keys.reset_and_get_address());
			cache.text_format_total_keys->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);
		}

		// graph_brush
		{
			com_ptr<ID2D1GradientStopCollection> gradient_stops;
			D2D1_GRADIENT_STOP stops[] =
			{
				{ 0.f, cache.theme_color },
				{ 0.3f, cache.light_active_color },
				{ 1.f, cache.active_color },
			};
			pRenderTarget->CreateGradientStopCollection(
				stops,
				std::size(stops),
				gradient_stops.reset_and_get_address()
			);
			pRenderTarget->CreateLinearGradientBrush(
				D2D1::LinearGradientBrushProperties(
					D2D1::Point2F(0, cy_graph * x),
					D2D1::Point2F(0, 0)),
				D2D1::BrushProperties(),
				gradient_stops,
				cache.graph_brush.reset_and_get_address()
			);
		}
	}
	timer_thread _tt{ [this] {
		if (hwnd)
		{
			InvalidateRect(hwnd, nullptr, FALSE);
			UpdateWindow(hwnd);
		}
	}, 1000 / 60 };
	/// <summary>
	/// 将颜色根据参数进行插值。
	/// crt 从 0 到 threshold 1 之间，总是 from。
	/// crt 接近 threshold2 时，接近 to（0.9）。
	/// crt 趋于正无穷时，为 to。当 crt 为负数时，行为未定义。
	/// </summary>
	/// <typeparam name="param_t"></typeparam>
	/// <param name="from"></param>
	/// <param name="to"></param>
	/// <param name="crt">当前参数。</param>
	/// <param name="threshold1">阈值 1。</param>
	/// <returns></returns>
	template <typename param_t>
	static color _interpolate(color from, color to, param_t crt, param_t threshold1, param_t threshold2)
	{
		if (crt < 0 || threshold1 < 0 || threshold2 < threshold1)
			throw std::invalid_argument("invalid argument.");

		double ratio{};
		if (crt >= threshold1)
		{
			crt -= threshold1;
			auto sigmoid = [](double x) {return 1 / (1 + std::exp(-x)); };
			constexpr double ratio_t2 = 0.9;
			double a = -std::log(1 / ratio_t2 - 1) / std::log(std::exp(1)) / (threshold2 - threshold1);
			ratio = (sigmoid(a * crt) - 0.5) * 2;
		}
		return color::linear_interpolation(from, to, ratio);
	}
	void OnPaint(HWND);

	// 绘图位置参数。
	inline static double cx_button = 52.0;
	inline static double cy_button = 72.0;
	inline static double cx_gap = 8.0;
	inline static double cy_separator = 8.0;
	inline static double cx_statistics = 232.0;
	inline static double cx_kps_number = 24.0;
	inline static double cx_kps_text = 45.0;
	inline static double cx_total_number = 40.0;
	inline static double cy_statistics = 20.0;
	inline static double cy_total_keys = 15.0;
	inline static double cy_graph = 80.0;
	bool show_buttons{ true };
	bool show_statistics{ true };
	bool show_graph{ true };
	/// <summary>
	/// 计算窗口应有的大小。<remarks>计算时不考虑缩放，最后再乘以缩放。</remarks>
	/// </summary>
	/// <returns>(cx, cy)</returns>
	std::tuple<double, double> calc_size() const
	{
		double cx = cx_button * k_manager.get_button_count() +
			cx_gap * (k_manager.get_button_count() - 1);
		cx = std::max(cx, cx_statistics);
		double cy = 0;
		if (show_buttons)
			cy += cy_button + cy_separator;
		if (show_statistics)
			cy += cy_statistics + 0.5 * cy_separator + cy_separator;
		if (show_graph)
			cy += cy_graph + cy_separator;
		cy -= cy_separator;
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

		RECT rWorkArea = monitor_area();
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

	// KPS。
public:
	keys_manager k_manager{ &kps };
	kps::kps kps{ std::bind(&keys_manager::update_on_key_down, &k_manager, std::placeholders::_1, std::placeholders::_2) };
	// 选项。
public:
	/// <summary>
	/// 改变当前按键个数。
	/// </summary>
	void change_button_count(int new_count)
	{
		CheckMenuItem(hMenu, k_manager.get_button_count(), MF_UNCHECKED);
		k_manager.set_button_count(new_count);
		CheckMenuItem(hMenu, k_manager.get_button_count(), MF_CHECKED);
		resize();
	}
	/// <summary>
	/// 改变当前缩放比例。
	/// </summary>
	void change_scale(double new_scale)
	{
		constexpr double t[]{ 0.75, 1, 2, 3 };
		scale = new_scale;
		build_scale_dep_resource();
		resize();

		for (int i = id_zoom_half; i <= id_zoom_3; i++)
			CheckMenuItem(hMenu, i, MF_UNCHECKED);
		if (scale == 1)
			CheckMenuItem(hMenu, id_zoom_1, MF_CHECKED);
		else if (scale == 2)
			CheckMenuItem(hMenu, id_zoom_2, MF_CHECKED);
		else if (scale == 3)
			CheckMenuItem(hMenu, id_zoom_3, MF_CHECKED);
		else
			CheckMenuItem(hMenu, id_zoom_half, MF_CHECKED);
	}
	/// <summary>
	/// 改变当前 KPS 计算方式。
	/// </summary>
	/// <param name="new_type"></param>
	void change_implement(kps::kps_implement_type new_type)
	{
		kps.change_implement_type(new_type);

		for (int i = id_method_hard; i <= id_method_hard; i++)
			CheckMenuItem(hMenu, i, MF_UNCHECKED);

		switch (kps.implement_type())
		{
		case kps::kps_implement_type::kps_implement_type_hard:
		{
			CheckMenuItem(hMenu, id_method_hard, MF_CHECKED);
			break;
		}
		}
	}
	bool is_any_shown()
	{
		return show_buttons || show_statistics || show_graph;
	}
	void change_show_buttons(bool show)
	{
		show_buttons = show;
		if (!is_any_shown())
			show_buttons = true;
		CheckMenuItem(hMenu, id_show_buttons, show_buttons ? MF_CHECKED : MF_UNCHECKED);
		resize();
	}
	void change_show_statistics(bool show)
	{
		show_statistics = show;
		if (!is_any_shown())
			show_statistics = true;
		CheckMenuItem(hMenu, id_show_statistics, show_statistics ? MF_CHECKED : MF_UNCHECKED);
		resize();
	}
	void change_show_graph(bool show)
	{
		show_graph = show;
		if (!is_any_shown())
			show_graph = true;
		CheckMenuItem(hMenu, id_show_graph, show_graph ? MF_CHECKED : MF_UNCHECKED);
		resize();
	}

private:
	double scale{ 1 }; // 绘图时的额外比例因子。
};

void main_window::OnPaint(HWND)
{
	pRenderTarget->BeginDraw();
	pRenderTarget->SetTransform(D2D1::IdentityMatrix());
	pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Black));
	double x = dpi() * scale; // 总比例因子。
	{
		// 画按键框。
		if (show_buttons)
		{
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

				auto key_name_rect = original_rect; // 每个框的键名对应的矩形。
				key_name_rect.bottom -= (key_name_rect.bottom - key_name_rect.top) * 2 / 5;
				auto number_rect = original_rect; // 每个框的数字对应的矩形。
				number_rect.top += (number_rect.bottom - number_rect.top) * 2 / 5;

				// 按键的发光效果。
				{
					auto alpha_param = std::chrono::duration_cast<decltype(0.0s)>(
						kps::clock::now() - k_manager.previous_by_index(i));
					color brush_color = _interpolate(cache.theme_color, cache.light_active_color,
						kps.calc_kps_now(k_manager.get_keys()[i]), 2.0, 4.0);
					brush_color.a = 0.75;
					color brush_color_transparent = brush_color;
					brush_color_transparent.a = 0;
					brush_color = _interpolate(brush_color, brush_color_transparent,
						alpha_param.count(), (0.1s).count(), (0.5s).count());

					if (brush_color.a > 1e-6)
					{
						com_ptr<ID2D1SolidColorBrush> brush;
						pRenderTarget->CreateSolidColorBrush(brush_color, brush.reset_and_get_address());
						pRenderTarget->FillRoundedRectangle(draw_rounded_rect, brush);
					}
				}

				// 写字。
				{
					auto s = cache.kc.to_short(k_manager.get_keys()[i]);
					auto str = code_conv<char8_t, wchar_t>::convert(s.key);
					pRenderTarget->DrawTextW(str.c_str(), str.length(),
						s.need_MDL2 ? cache.text_format_key_name_MDL2 :
						(s.is_single ? cache.text_format_key_name : cache.text_format_key_name_small),
						key_name_rect, cache.theme_brush);
				}
				{
					double k_now = kps.calc_kps_now(k_manager.get_keys()[i]); // 当前框对应 kps。
					auto str = std::to_wstring(static_cast<int>(k_now + 0.5));

					pRenderTarget->DrawTextW(str.c_str(), str.length(), cache.text_format_number,
						number_rect, cache.theme_brush);
				}

				// 最外层的框。
				{
					color text_color = _interpolate(cache.theme_color, cache.light_active_color,
						kps.calc_kps_now(k_manager.get_keys()[i]), 2.0, 4.0);
					com_ptr<ID2D1SolidColorBrush> brush;
					pRenderTarget->CreateSolidColorBrush(text_color, brush.reset_and_get_address());
					pRenderTarget->DrawRoundedRectangle(draw_rounded_rect, brush, stroke_width);
				}
			}
			// 平移绘制区域，使得逻辑坐标从 (0, 0) 开始。
			{
				D2D1_MATRIX_3X2_F transform;
				pRenderTarget->GetTransform(&transform);
				auto move = D2D1::Matrix3x2F::Translation(0, (cy_button + cy_separator) * x);
				pRenderTarget->SetTransform(transform * move);
			}
		}

		// 画统计信息。
		if (show_statistics)
		{
			auto kps_number_rect = D2D1::RectF(
				cx_gap * x, 0,
				(cx_gap + cx_kps_number) * x,
				cy_statistics * x); // kps 数值矩形。
			auto kps_text_rect = D2D1::RectF(
				kps_number_rect.right + cx_gap * x, 0,
				kps_number_rect.right + (cx_gap + cx_kps_text) * x,
				cy_statistics * x); // kps 文字矩形。
			auto max_number_rect = D2D1::RectF(
				kps_text_rect.right + cx_gap * x, 0,
				kps_text_rect.right + (cx_gap + cx_kps_number) * x,
				cy_statistics * x); // 最大 kps 数值矩形。
			auto max_text_rect = D2D1::RectF(
				max_number_rect.right + cx_gap * x, 0,
				max_number_rect.right + (cx_gap + cx_kps_text) * x,
				cy_statistics * x); // 最大 kps 文字矩形。
			auto total_number_rect = D2D1::RectF(
				max_text_rect.right + cx_gap * x, (cy_statistics - cy_total_keys) * x,
				(cx_statistics - cx_gap) * x,
				cy_statistics * x); // 总按键数值矩形。

			wchar_t buffer[256];
			{
				double kps_now = kps.calc_kps_now(k_manager.get_keys());
				std::swprintf(buffer, std::size(buffer), L"%d",
					static_cast<int>(kps_now + 0.5));
				auto str = std::wstring(buffer);

				color text_color = _interpolate(cache.theme_color, cache.active_color, kps_now, 6.0, 13.0);
				com_ptr<ID2D1SolidColorBrush> brush;
				pRenderTarget->CreateSolidColorBrush(text_color, brush.reset_and_get_address());
				cache.text_format_statistics->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);
				pRenderTarget->DrawTextW(str.c_str(), str.length(), cache.text_format_statistics,
					kps_number_rect, brush);
			}
			{
				std::swprintf(buffer, std::size(buffer), L"KPS");
				auto str = std::wstring(buffer);

				cache.text_format_statistics->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
				pRenderTarget->DrawTextW(str.c_str(), str.length(), cache.text_format_statistics,
					kps_text_rect, cache.theme_brush);
			}
			{
				std::swprintf(buffer, std::size(buffer), L"%d",
					static_cast<int>(k_manager.get_max_kps() + 0.5));
				auto str = std::wstring(buffer);

				cache.text_format_statistics->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);
				pRenderTarget->DrawTextW(str.c_str(), str.length(), cache.text_format_statistics,
					max_number_rect, cache.theme_brush);
			}
			{
				std::swprintf(buffer, std::size(buffer), L"max");
				auto str = std::wstring(buffer);

				cache.text_format_statistics->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
				pRenderTarget->DrawTextW(str.c_str(), str.length(), cache.text_format_statistics,
					max_text_rect, cache.theme_brush);
			}
			{
				std::swprintf(buffer, std::size(buffer), L"%d",
					k_manager.get_total_count());
				auto str = std::wstring(buffer);

				pRenderTarget->DrawTextW(str.c_str(), str.length(), cache.text_format_total_keys,
					total_number_rect, cache.theme_brush);
			}

			// 平移绘制区域，使得逻辑坐标从 (0, 0) 开始。
			{
				D2D1_MATRIX_3X2_F transform;
				pRenderTarget->GetTransform(&transform);
				auto move = D2D1::Matrix3x2F::Translation(0, (cy_statistics + 0.5 * cy_separator + cy_separator) * x);
				pRenderTarget->SetTransform(transform * move);
			}
		}

		// 画图
		if (show_graph)
		{
			auto graph_rect = D2D1::RectF(
				cx_gap * x, 0,
				width() - cx_gap * x,
				cy_graph * x); // 图形矩形。
			double stroke_width = 1 * x;
			auto draw_rect = graph_rect; // 使用 DrawRectangle 时对应的矩形。
			draw_rect.left += stroke_width / 2;
			draw_rect.right -= stroke_width / 2;
			draw_rect.top += stroke_width / 2;
			draw_rect.bottom -= stroke_width / 2;

			// 具体图形。
			{
				com_ptr<ID2D1PathGeometry> geometry;
				factory::d2d1()->CreatePathGeometry(geometry.reset_and_get_address());
				com_ptr<ID2D1GeometrySink> sink;
				geometry->Open(sink.reset_and_get_address());
				{
					sink->BeginFigure(
						D2D1::Point2F(draw_rect.right, draw_rect.bottom),
						D2D1_FIGURE_BEGIN_FILLED
					);
					sink->AddLine(D2D1::Point2F(draw_rect.left, draw_rect.bottom));
					auto value = kps.calc_kps_recent(k_manager.get_keys());
					double ceil_height = std::max(5.0,
						1.2 * std::max(k_manager.get_max_kps(),
							*std::max_element(value.begin(), value.end())));
					double graph_width = draw_rect.right - draw_rect.left;
					double graph_height = draw_rect.bottom - draw_rect.top;
					for (size_t i = 0; i < kps::history_count; i++)
					{
						sink->AddLine(D2D1::Point2F(
							draw_rect.left + graph_width * i / (kps::history_count - 1),
							draw_rect.bottom - graph_height * (value[i] / ceil_height)
						));
					}
					sink->AddLine(D2D1::Point2F(draw_rect.right, draw_rect.bottom));
					sink->EndFigure(D2D1_FIGURE_END_CLOSED);
				}
				sink->Close();

				pRenderTarget->FillGeometry(geometry, cache.graph_brush);
			}
			// 外边框。
			{
				pRenderTarget->DrawRectangle(draw_rect, cache.theme_brush, stroke_width);
			}

			// 平移绘制区域，使得逻辑坐标从 (0, 0) 开始。
			{
				D2D1_MATRIX_3X2_F transform;
				pRenderTarget->GetTransform(&transform);
				auto move = D2D1::Matrix3x2F::Translation(0, (cy_graph + cy_separator) * x);
				pRenderTarget->SetTransform(transform * move);
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