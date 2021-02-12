// Copyright (c) UnnamedOrange. Licensed under the MIT Licence.
// See the LICENSE file in the repository root for full licence text.

#pragma once

#include <utils/config_manager.hpp>

class config : public config_manager
{
public:
	double scale() const
	{
		return std::get<double>(get_value(u8"scale"));
	}
	void scale(double new_scale)
	{
		(*this)[u8"scale"] = new_scale;
	}
};