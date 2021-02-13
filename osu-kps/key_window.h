// Copyright (c) UnnamedOrange. Licensed under the MIT Licence.
// See the LICENSE file in the repository root for full licence text.

#pragma once

#include <thread>

#include <utils/window.hpp>
#include <utils/d2d_helper.hpp>
#include <utils/keyboard_char.hpp>

class key_window : public window
{
	virtual INT_PTR WindowProc(HWND, UINT message, WPARAM wParam, LPARAM lParam) override
	{
		switch (message)
		{
			HANDLE_MSG(hwnd, WM_CREATE, OnCreate);
			HANDLE_MSG(hwnd, WM_DESTROY, OnDestroy);
		default:
			return DefWindowProcW(hwnd, message, wParam, lParam);
		}
		return 0;
	}
	BOOL OnCreate(HWND, LPCREATESTRUCT)
	{
		EnableWindow(GetParent(hwnd), false);

		{
			LONG ex_style;
			while (!((ex_style = GetWindowLongW(hwnd, GWL_EXSTYLE)) & WS_EX_TOPMOST))
			{
				SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
				SetWindowLongW(hwnd, GWL_EXSTYLE, ex_style | WS_EX_TOPMOST);
				std::this_thread::yield();
			}
		}

		return TRUE;
	}
	void OnDestroy(HWND)
	{
		EnableWindow(GetParent(hwnd), true);
	}
};