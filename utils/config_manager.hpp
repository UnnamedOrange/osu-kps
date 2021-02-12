// Copyright (c) UnnamedOrange. Licensed under the MIT Licence.
// See the LICENSE file in the repository root for full licence text.

#pragma once

#include <cstdint>
#include <fstream>
#include <string>
#include <variant>
#include <filesystem>

#include "code_conv.hpp"
#include <json/json.h>

class config_manager
{
public:
	Json::Value root;

public:
	void clear() noexcept
	{
		root.clear();
	}
	void update(const Json::Value& json_template)
	{
		if (!json_template.isObject())
			throw std::invalid_argument("json_template must be an object.");

		for (auto it = json_template.begin(); it != json_template.end(); it++)
			root[it.name()] = *it;
	}
	void update(const std::filesystem::path& json_file_path)
	{
		std::ifstream ifs(json_file_path);
		std::string str{ std::istreambuf_iterator<char>(ifs),
			std::istreambuf_iterator<char>() }; // UTF-8
		Json::CharReaderBuilder builder;
		std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
		Json::Value json_template;
		if (!reader->parse(str.c_str(), str.c_str() + str.length(),
			&json_template, nullptr))
			throw std::runtime_error("parse error.");
		update(json_template);
	}

public:
	using value_t = std::variant<std::nullopt_t, bool, int64_t, uint64_t, double, std::u8string>;
	value_t get_value(std::u8string_view key)
	{
		auto value = root[reinterpret_cast<const char*>(key.data())];
		if (value.isNull())
			return std::nullopt;
		else if (value.isBool())
			return value.asBool();
		else if (value.isInt64())
			return value.asInt64();
		else if (value.isUInt64())
			return value.asUInt64();
		else if (value.isDouble())
			return value.asDouble();
		else if (value.isString())
			return std::u8string(reinterpret_cast<const char8_t*>(value.asCString()));
		// else
		throw std::runtime_error("type not supported.");
	}
	Json::Value& operator[](std::u8string_view key)
	{
		return root[reinterpret_cast<const char*>(key.data())];
	}
	const Json::Value& operator[](std::u8string_view key) const
	{
		return root[reinterpret_cast<const char*>(key.data())];
	}
};