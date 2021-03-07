// Copyright (c) UnnamedOrange. Licensed under the MIT Licence.
// See the LICENSE file in the repository root for full licence text.

#include "resource.h"

#include "key_window.h"

using namespace d2d_helper;

void key_window::init_d2d()
{
	cache.reset();

	if (FAILED(factory::d2d1()->CreateHwndRenderTarget(D2D1::RenderTargetProperties(),
		D2D1::HwndRenderTargetProperties(hwnd, D2D1::SizeU(cwidth(), cheight())),
		pRenderTarget.reset_and_get_address())))
		throw std::runtime_error("Fail to CreateHwndRenderTarget.");
	pRenderTarget->SetDpi(USER_DEFAULT_SCREEN_DPI, USER_DEFAULT_SCREEN_DPI); // 自己处理高 DPI。

	build_indep_resource();
	build_scale_dep_resource();
	d2d_inited = true;
}
void key_window::build_indep_resource()
{
	pRenderTarget->CreateSolidColorBrush(cache.theme_color,
		cache.theme_brush.reset_and_get_address());
	pRenderTarget->CreateSolidColorBrush(cache.light_active_color,
		cache.active_brush.reset_and_get_address());

	cache.theme_font_collection.from_font_data(
		resource_loader::load(MAKEINTRESOURCEW(IDR_Exo2_Regular), L"OTF"));
}
void key_window::build_scale_dep_resource()
{
	double x = dpi();
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
}
void key_window::OnPaint(HWND)
{
	if (!d2d_inited)
		init_d2d();

	pRenderTarget->BeginDraw();
	pRenderTarget->SetTransform(D2D1::IdentityMatrix());
	pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Black));
	double x = dpi(); // 总比例因子。
	{
		for (unsigned i = 0; i < crt_keys; i++)
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

			// 写字。
			if (i < keys.size())
			{
				auto s = cache.kc.to_short(keys[i]);
				auto str = code_conv<char8_t, wchar_t>::convert(s.key);
				pRenderTarget->DrawTextW(str.c_str(), str.length(),
					s.need_MDL2 ? cache.text_format_key_name_MDL2 :
					(s.is_single ? cache.text_format_key_name : cache.text_format_key_name_small),
					key_name_rect, cache.theme_brush);
			}

			// 最外层的框。
			{
				pRenderTarget->DrawRoundedRectangle(draw_rounded_rect,
					i == keys.size() ? cache.active_brush : cache.theme_brush, stroke_width);
			}
		}
	}
	HRESULT hr = pRenderTarget->EndDraw();
	if (SUCCEEDED(hr))
		ValidateRect(hwnd, nullptr);
	else
		d2d_inited = false;
}