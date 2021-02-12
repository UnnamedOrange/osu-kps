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

/// <summary>
/// 配置系统。需要 JsonCpp。不保证线程安全。
/// </summary>
class config_manager
{
public:
	Json::Value root;

public:
	/// <summary>
	/// 完全清空所有配置。
	/// </summary>
	void clear() noexcept
	{
		root.clear();
	}
	/// <summary>
	/// 根据给定的 Json 更新配置系统，将给定的键值对插入或使用它覆盖配置系统中的键-值对。
	/// </summary>
	/// <param name="is_base">提供的内容是否是最底层的模板。如果不是，将忽略类型不匹配的值或不存在的键。</param>
	void update(const Json::Value& json_template, bool is_base = true)
	{
		if (!json_template.isObject())
			throw std::invalid_argument("json_template must be an object.");

		for (auto it = json_template.begin(); it != json_template.end(); it++)
		{
			auto value = *it;
			if (!is_base)
			{
				if (!root.isMember(it.name()))
					continue;
				if (!value.isConvertibleTo(root[it.name()].type()))
					continue;

				switch (root[it.name()].type())
				{
				case Json::ValueType::nullValue:
					value = Json::Value(Json::ValueType::nullValue);
					break;
				case Json::ValueType::booleanValue:
					value = value.asBool();
					break;
				case Json::ValueType::intValue:
					value = value.asInt64();
					break;
				case Json::ValueType::uintValue:
					value = value.asUInt64();
					break;
				case Json::ValueType::realValue:
					value = value.asDouble();
					break;
				default: // 其余无需转换。
					break;
				}
			}

			root[it.name()] = value;
		}
	}
	void update(std::u8string_view json_content, bool is_base = true)
	{
		Json::CharReaderBuilder builder;
		std::unique_ptr<Json::CharReader> const reader(builder.newCharReader());
		Json::Value json_template;
		Json::String error;
		if (!reader->parse(
			reinterpret_cast<const char*>(json_content.data()),
			reinterpret_cast<const char*>(json_content.data() + json_content.length()),
			&json_template, &error) || error.size())
			throw std::runtime_error("parse error.");
		update(json_template, is_base);
	}
	void update(const std::filesystem::path& json_file_path, bool is_base = true)
	{
		std::ifstream ifs(json_file_path);
		std::string str{ std::istreambuf_iterator<char>(ifs),
			std::istreambuf_iterator<char>() }; // UTF-8
		update(std::u8string_view(
			reinterpret_cast<const char8_t*>(str.data())), is_base);
	}

public:
	using value_t = std::variant<std::nullopt_t, bool, int64_t, uint64_t, double, std::u8string>;
	/// <summary>
	/// 获取配置系统中的值。
	/// </summary>
	/// <param name="key">键。</param>
	/// <returns>值。类型是 value_t 中支持的类型之一。如果不支持，将会抛出错误。</returns>
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