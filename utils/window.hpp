// Copyright (c) UnnamedOrange. Licensed under the MIT Licence.
// See the LICENCE file in the repository root for full licence text.

#pragma once

#include <vector>
#include <string>
#include <string_view>
#include <unordered_set>
#include <optional>
#include <semaphore>
#include <stdexcept>
#include <typeinfo>
#include <concepts>
#include <Windows.h>
#undef min
#undef max
#include <windowsx.h>

#include "code_conv.hpp"

/// <summary>
/// window 基类。
/// window 类的生命周期必须包含窗口的生命周期。
/// window 不可复制，但可以移动。不可在窗口过程中进行移动操作。
/// </summary>
class window
{
private:
	HWND __hwnd{};
	std::atomic<int> __in_proc{}; // 是否在窗口过程中。
public:
	const HWND& hwnd{ __hwnd }; // 窗口句柄的常值引用。
public:
	window() = default;
	window(const window&) = delete;
	window(window&& another) noexcept : __hwnd{}, hwnd{ __hwnd }, __in_proc{}
	{
		*this = std::move(another);
	}
	window& operator=(const window&) = delete;
	window& operator=(window&& another) noexcept
	{
		if (this != &another)
		{
			if (hwnd)
				DestroyWindow(hwnd);
			__hwnd = another.hwnd;
			__in_proc = 0;
			if (hwnd)
			{
				another.__hwnd = nullptr;
				SetWindowLongPtrW(hwnd, GWLP_USERDATA,
					reinterpret_cast<LONG_PTR>(this));
			}
		}
		return *this;
	}

private:
	RECT get_window_rect() const
	{
		if (!hwnd)
			throw std::runtime_error("hwnd is nullptr.");
		RECT ret;
		if (!GetWindowRect(hwnd, &ret))
			throw std::runtime_error("fail to GetWindowRect.");
		return ret;
	}
public:
	/// <returns>
	/// 窗口左部坐标。
	/// </returns>
	int left() const
	{
		return get_window_rect().left;
	}
	/// <returns>
	/// 窗口顶部坐标。
	/// </returns>
	int top() const
	{
		return get_window_rect().top;
	}
	/// <returns>
	/// 窗口右部坐标。
	/// </returns>
	int right() const
	{
		return get_window_rect().right;
	}
	/// <returns>
	/// 窗口底部坐标。
	/// </returns>
	int bottom() const
	{
		return get_window_rect().bottom;
	}
	/// <returns>
	/// 窗口宽度。
	/// </returns>
	int width() const
	{
		auto t = get_window_rect();
		return t.right - t.left;
	}
	/// <returns>
	/// 窗口高度。
	/// </returns>
	int height() const
	{
		auto t = get_window_rect();
		return t.bottom - t.top;
	}
private:
	void set_window_rect(const RECT& r)
	{
		if (!hwnd)
			throw std::runtime_error("hwnd is nullptr.");
		if (!MoveWindow(hwnd, r.left, r.top, r.right - r.left, r.bottom - r.top, true))
			throw std::runtime_error("fail to MoveWindow.");
	}
public:
	/// <summary>
	/// 设置窗口左部坐标。
	/// </summary>
	void left(int x)
	{
		auto r = get_window_rect();
		r.left = x;
		set_window_rect(r);
	}
	/// <summary>
	/// 设置窗口顶部坐标。
	/// </summary>
	void top(int y)
	{
		auto r = get_window_rect();
		r.top = y;
		set_window_rect(r);
	}
	/// <summary>
	/// 设置窗口右部坐标。
	/// </summary>
	void right(int x)
	{
		auto r = get_window_rect();
		r.right = x;
		set_window_rect(r);
	}
	/// <summary>
	/// 设置窗口底部坐标。
	/// </summary>
	void bottom(int y)
	{
		auto r = get_window_rect();
		r.bottom = y;
		set_window_rect(r);
	}
	/// <summary>
	/// 设置窗口宽度，保持窗口左部不移动。
	/// </summary>
	void width(int cx)
	{
		auto r = get_window_rect();
		r.right = r.left + cx;
		set_window_rect(r);
	}
	/// <summary>
	/// 设置窗口高度，保持窗口顶部不移动。
	/// </summary>
	void height(int cy)
	{
		auto r = get_window_rect();
		r.bottom = r.top + cy;
		set_window_rect(r);
	}
private:
	/// <returns>
	/// 窗口客户区在屏幕上对应的矩形。
	/// </returns>
	RECT get_client_rect() const
	{
		if (!hwnd)
			throw std::runtime_error("hwnd is nullptr.");
		RECT ret;
		if (!GetClientRect(hwnd, &ret))
			throw std::runtime_error("fail to GetWindowRect.");
		POINT pt{};
		if (!ClientToScreen(hwnd, &pt))
			throw std::runtime_error("fail to ClientToScreen.");
		ret.left += pt.x;
		ret.right += pt.x;
		ret.top += pt.y;
		ret.bottom += pt.y;
		return ret;
	}
public:
	/// <returns>
	/// 窗口客户区左部坐标。
	/// </returns>
	int cleft() const
	{
		return get_client_rect().left;
	}
	/// <returns>
	/// 窗口客户区顶部坐标。
	/// </returns>
	int ctop() const
	{
		return get_client_rect().top;
	}
	/// <returns>
	/// 窗口客户区右部坐标。
	/// </returns>
	int cright() const
	{
		return get_client_rect().right;
	}
	/// <returns>
	/// 窗口客户区底部坐标。
	/// </returns>
	int cbottom() const
	{
		return get_client_rect().bottom;
	}
	/// <returns>
	/// 窗口客户区宽度。
	/// </returns>
	int cwidth() const
	{
		auto t = get_client_rect();
		return t.right - t.left;
	}
	/// <returns>
	/// 窗口客户区高度。
	/// </returns>
	int cheight() const
	{
		auto t = get_client_rect();
		return t.bottom - t.top;
	}

public:
	/// <returns>
	/// 设置窗口标题（ANSI）。
	/// </returns>
	void caption(std::string_view s)
	{
		if (!hwnd)
			throw std::runtime_error("hwnd is nullptr.");
		if (!SetWindowTextA(hwnd, s.data()))
			throw std::runtime_error("fail to SetWindowTextA.");
	}
	/// <returns>
	/// 设置窗口标题（宽字符）。
	/// </returns>
	void caption(std::wstring_view s)
	{
		if (!hwnd)
			throw std::runtime_error("hwnd is nullptr.");
		if (!SetWindowTextW(hwnd, s.data()))
			throw std::runtime_error("fail to SetWindowTextW.");
	}
	/// <returns>
	/// 窗口标题。
	/// </returns>
	std::wstring caption() const
	{
		if (!hwnd)
			throw std::runtime_error("hwnd is nullptr.");
		std::wstring ret(GetWindowTextLengthW(hwnd), 0);
		GetWindowTextW(hwnd, ret.data(), int(ret.size()));
		return ret;
	}

public:
	/// <summary>
	/// 获取窗口根据显示器设置的 DPI 应缩放的比例。
	/// </summary>
	/// <returns>浮点数，代表比例。</returns>
	double dpi() const
	{
		if (!hwnd)
			throw std::runtime_error("hwnd is nullptr.");
		return static_cast<double>(GetDpiForWindow(hwnd)) / USER_DEFAULT_SCREEN_DPI;
	}
	/// <summary>
	/// 根据窗口的 DPI 计算缩放后的整数。
	/// </summary>
	/// <typeparam name="int_t">整数类型。</typeparam>
	/// <param name="val">要缩放的值。</param>
	/// <returns>缩放后的值，保持类型不变。</returns>
	template <std::integral int_t>
	auto dpi(int_t val) const
	{
		if (!hwnd)
			throw std::runtime_error("hwnd is nullptr.");
		return static_cast<std::decay_t<int_t>>(static_cast<double>(val) * dpi());
	}
	/// <summary>
	/// 根据窗口的 DPI 计算缩放后的浮点数。
	/// </summary>
	/// <typeparam name="int_t">浮点数类型。</typeparam>
	/// <param name="val">要缩放的值。</param>
	/// <returns>缩放后的值，保持类型不变。</returns>
	template <std::floating_point float_t>
	auto dpi(float_t val) const
	{
		if (!hwnd)
			throw std::runtime_error("hwnd is nullptr.");
		return static_cast<std::decay_t<float_t>>(static_cast<double>(val) * dpi());
	}

public:
	/// <summary>
	/// 获取窗口所在显示器的工作区域（Work Area）。
	/// </summary>
	/// <returns>工作区域对应的矩形（RECT）。</returns>
	RECT work_area() const
	{
		if (!hwnd)
			throw std::runtime_error("hwnd is nullptr.");
		MONITORINFO mi{ sizeof(mi) };
		HMONITOR hMonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
		GetMonitorInfoW(hMonitor, &mi);
		return mi.rcWork;
	}

public:
	/// <returns>
	/// 注册窗口时（如果需要）的窗口类名。可以用作窗口的标识。
	/// </returns>
	std::wstring get_class_name() const
	{
		std::string_view name = typeid(*this).name();
		std::string ret;
		bool st = false;
		for (size_t i = name.length() - 1; ~i; i--)
			if (name[i] == ' ')
			{
				if (st)	break;
			}
			else
			{
				st = true;
				ret.push_back(name[i]);
			}
		std::reverse(ret.begin(), ret.end());
		return code_conv<char, wchar_t>::convert(ret);
	}

private:
	inline static std::binary_semaphore semaphore_create{ 1 }; // 创建窗口时所用的信号量，创建窗口的过程必须是互斥的。
	inline static window* storage{}; // 创建窗口时所用的全局变量。

private:
	inline static std::binary_semaphore semaphore_register{ 1 }; // 注册窗口的过程必须是互斥的。
	inline static std::unordered_set<std::wstring> registered; // 已注册的窗口类名。
	void register_class()
	{
		semaphore_register.acquire();
		auto name = get_class_name();
		if (registered.count(name))
			return;

		WNDCLASSEXW wcex{ sizeof(WNDCLASSEXW) };
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = VirtualWindowProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = GetModuleHandleA(nullptr);
		wcex.hIcon = nullptr;
		wcex.hIconSm = nullptr;
		wcex.hCursor = LoadCursorW(nullptr, IDC_ARROW);
		wcex.hbrBackground = GetSysColorBrush(COLOR_WINDOW);
		wcex.lpszMenuName = nullptr;
		wcex.lpszClassName = name.c_str();
		if (!RegisterClassExW(&wcex))
		{
			semaphore_register.release();
			throw std::runtime_error("fail to RegisterClassExW.");
		}
		registered.insert(name);
		semaphore_register.release();
	}
	static LRESULT CALLBACK VirtualWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		window* p = reinterpret_cast<window*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));

		if (!p)
		{
			if (!storage)
				throw std::runtime_error("storage shouldn't be nullptr.");
			p = storage;
			SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)p);
			p->__hwnd = hwnd;

			storage = nullptr;
			semaphore_create.release();
		}
		++p->__in_proc;
		LRESULT ret = p->WindowProc(hwnd, message, wParam, lParam);
		--p->__in_proc;

		if (p && message == WM_NCDESTROY)
			p->__hwnd = nullptr;

		return ret;
	}

protected:
	virtual INT_PTR WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) = 0;

public:
	/// <summary>
	/// 使用 CreateWindowExW 创建窗口。注意需要调用 DefWindowProcW。
	/// </summary>
	/// <returns>
	/// 创建的窗口的句柄。
	/// </returns>
	HWND create(HWND hwndParent = nullptr)
	{
		semaphore_create.acquire();
		storage = this;

		register_class();
		HWND ret = CreateWindowExW(0,
			get_class_name().c_str(),
			nullptr,
			WS_OVERLAPPEDWINDOW | WS_VISIBLE,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			hwndParent,
			nullptr,
			GetModuleHandleW(nullptr),
			nullptr);
		if (!ret)
			semaphore_create.release();
		return ret;
	}

public:
	/// <summary>
	/// 开始窗口消息循环。
	/// </summary>
	/// <returns>
	/// 退出消息的返回值。
	/// </returns>
	static int message_loop()
	{
		MSG msg;
		BOOL ret;
		while ((ret = GetMessageW(&msg, nullptr, 0, 0)) != 0)
		{
			if (~ret)
			{
				TranslateMessage(&msg);
				DispatchMessageW(&msg);
			}
			else
			{
				throw std::runtime_error("fatal application error.");
			}
		}
		return (int)msg.wParam;
	}
};