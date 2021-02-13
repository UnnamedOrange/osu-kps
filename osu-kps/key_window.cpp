// Copyright (c) UnnamedOrange. Licensed under the MIT Licence.
// See the LICENSE file in the repository root for full licence text.

#include "key_window.h"

using namespace d2d_helper;

void key_window::init_d2d()
{
	if (FAILED(factory::d2d1()->CreateHwndRenderTarget(D2D1::RenderTargetProperties(),
		D2D1::HwndRenderTargetProperties(hwnd, D2D1::SizeU(width(), height())),
		pRenderTarget.reset_and_get_address())))
		throw std::runtime_error("Fail to CreateHwndRenderTarget.");
	pRenderTarget->SetDpi(USER_DEFAULT_SCREEN_DPI, USER_DEFAULT_SCREEN_DPI); // 自己处理高 DPI。

	build_indep_resource();
	build_scale_dep_resource();
}
void key_window::build_indep_resource()
{

}
void key_window::build_scale_dep_resource()
{
	double x = dpi();
}
void key_window::OnPaint(HWND)
{
	pRenderTarget->BeginDraw();
	pRenderTarget->SetTransform(D2D1::IdentityMatrix());
	pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Black));
	double x = dpi(); // 总比例因子。
	{

	}
	pRenderTarget->EndDraw();
	ValidateRect(hwnd, nullptr);
}