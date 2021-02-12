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
	// TODO: 增加类型检查。
	void update(const Json::Value& json_template)
	{
		if (!json_template.isObject())
			throw std::invalid_argument("json_template must be an object.");

		for (auto it = json_template.begin(); it != json_template.end(); it++)
			root[it.name()] = *it;
	}
	void update(std::u8string_view json_content)
	{
		Json::CharReaderBuilder builder;
		std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
		Json::Value json_template;
		if (!reader->parse(
			reinterpret_cast<const char*>(json_content.data()),
			reinterpret_cast<const char*>(json_content.data() + json_content.length()),
			&json_template, nullptr))
			throw std::runtime_error("parse error.");
		update(json_template);
	}
	void update(const std::filesystem::path& json_file_path)
	{
		std::ifstream ifs(json_file_path);
		std::string str{ std::istreambuf_iterator<char>(ifs),
			std::istreambuf_iterator<char>() }; // UTF-8
		update(std::u8string_view(
			reinterpret_cast<const char8_t*>(str.data())));
	}

public:
	using value_t = std::variant<std::nullopt_t, bool, int64_t, uint64_t, double, std::u8string>;
	value_t get_value(std::u8string_view key) const
	{
		auto value = root[reinterpret_cast<const char*>(key.data())];
		switch (value.type())
		{
		case Json::ValueType::nullValue:
			return std::nullopt;
		case Json::ValueType::booleanValue:
			return value.asBool();
		case Json::ValueType::intValue:
			return value.asInt64();
		case Json::ValueType::uintValue:
			return value.asUInt64();
		case Json::ValueType::realValue:
			return value.asDouble();
		case Json::ValueType::stringValue:
			return std::u8string(reinterpret_cast<const char8_t*>(value.asCString()));
		default:
			throw std::runtime_error("type not supported.");
		}
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