// Copyright (c) UnnamedOrange. Licensed under the MIT Licence.
// See the LICENSE file in the repository root for full licence text.

#pragma once

#include <utils/config_manager.hpp>

#include "integrated_kps.hpp"
#include "keys_manager.hpp"

class config : public config_manager
{
public:
	/// <summary>
	/// 将所有参数合法化。调用时首先假设所有参数的类型正确。
	/// </summary>
	void regulate()
	{
		scale(std::max(0.5, std::min(3.0, scale())));
		button_count(std::max(1, std::min(static_cast<int>(keys_manager::max_key_count), button_count())));
		kps_method(static_cast<kps::kps_implement_type>(std::max(0, std::min(0, static_cast<int>(kps_method())))));
	}

public:
	double scale() const
	{
		return std::get<double>(get_value(u8"scale"));
	}
	void scale(double new_scale)
	{
		(*this)[u8"scale"] = new_scale;
	}

	bool show_buttons() const
	{
		return std::get<bool>(get_value(u8"show.buttons"));
	}
	void show_buttons(bool show)
	{
		(*this)[u8"show.buttons"] = show;
	}

	bool show_statistics() const
	{
		return std::get<bool>(get_value(u8"show.statistics"));
	}
	void show_statistics(bool show)
	{
		(*this)[u8"show.statistics"] = show;
	}

	bool show_graph() const
	{
		return std::get<bool>(get_value(u8"show.graph"));
	}
	void show_graph(bool show)
	{
		(*this)[u8"show.graph"] = show;
	}

	int button_count() const
	{
		return static_cast<int>(std::get<int64_t>(get_value(u8"kps.button_count")));
	}
	void button_count(int count)
	{
		(*this)[u8"button_count"] = count;
	}

	kps::kps_implement_type kps_method() const
	{
		return static_cast<kps::kps_implement_type>(std::get<int64_t>(get_value(u8"kps.method")));
	}
	void kps_method(kps::kps_implement_type type)
	{
		(*this)[u8"kps.method"] = static_cast<int>(type);
	}
};