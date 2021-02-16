// Copyright (c) UnnamedOrange. Licensed under the MIT Licence.
// See the LICENSE file in the repository root for full licence text.

// This header needs JsonCpp.

#pragma once

#include <fstream>
#include <string>
#include <map>
#include <mutex>
#include <filesystem>

#include <json/json.h>

#include "code_conv.hpp"

class language_not_exists_error : public std::runtime_error
{
public:
	using std::runtime_error::runtime_error;
};

class multi_language
{
private:
	mutable std::recursive_mutex m;
	std::map<int, Json::Value> langs;
	int current_language{};

private:
	static std::u8string string(const Json::Value& v)
	{
		return reinterpret_cast<const char8_t*>(v.asCString());
	}

public:
	/// <summary>
	/// Load the language file. The first loaded language will be the base language.
	/// If there is no translation in some language, the text of the base language will be used.
	/// The function does not check whether the locale name has already existed.
	/// </summary>
	/// <param name="path"></param>
	/// <returns>If the file is loaded successfully, returns true. Otherwise returns false.</returns>
	bool load_language(std::string_view json_content)
	{
		std::lock_guard _(m);

		Json::CharReaderBuilder builder;
		std::unique_ptr<Json::CharReader> const reader(builder.newCharReader());
		Json::Value loaded_json;
		Json::String error;
		if (!reader->parse(
			reinterpret_cast<const char*>(json_content.data()),
			reinterpret_cast<const char*>(json_content.data() + json_content.length()),
			&loaded_json, &error) || error.size())
			return false;

		if (!loaded_json.isMember("id") || !loaded_json["id"].isInt())
			return false;
		if (!loaded_json.isMember("locale") || !loaded_json["locale"].isString())
			return false;
		if (!loaded_json.isMember("language_name") || !loaded_json["language_name"].isString())
			return false;

		if (langs.empty())
			current_language = loaded_json["id"].asInt();
		langs[loaded_json["id"].asInt()] = loaded_json;
		return true;
	}
	/// <summary>
	/// Load the language file. The first loaded language will be the base language.
	/// If there is no translation in some language, the text of the base language will be used.
	/// The function does not check whether the locale name has already existed.
	/// </summary>
	/// <param name="path"></param>
	/// <returns>If the file is loaded successfully, returns true. Otherwise returns false.</returns>
	bool load_language(const std::filesystem::path& path)
	{
		std::lock_guard _(m);

		std::string json_content;
		try
		{
			std::ifstream ifs(path);
			json_content = std::string(std::istreambuf_iterator<char>(ifs),
				std::istreambuf_iterator<char>());
		}
		catch (...)
		{
			return false;
		}

		return load_language(std::string_view(json_content));
	}

public:
	/// <summary>
	/// Get a list that enumerates all supported language in this multi_language object.
	/// Every element of the list is a string standing for a locale name.
	/// However, the local name is from the corresponding language file, equaling to the value of the key "locale".
	/// </summary>
	std::vector<std::u8string> enumerate_supported_language() const
	{
		std::lock_guard _(m);
		std::vector<std::u8string> ret;
		for (const auto& [k, v] : langs)
			ret.push_back(string(v["locale"]));
		return ret;
	}
	/// <summary>
	/// Check whether the language specified by a locale name is supported by this multi_language object.
	/// <remarks>See enumerate_supported_language.</remarks>
	/// </summary>
	bool is_language_supported(std::u8string_view locale) const
	{
		std::lock_guard _(m);
		for (const auto& [k, v] : langs)
			if (string(v["locale"]) == locale)
				return true;
		return false;
	}
	/// <summary>
	/// Get the name of a language according to the locale name.
	/// The name of a language equals to the value of the key "language_name".
	/// </summary>
	std::u8string query_language_name(std::u8string_view locale) const
	{
		std::lock_guard _(m);
		for (const auto& [k, v] : langs)
			if (string(v["locale"]) == locale)
				return string(v["language_name"]);
		throw language_not_exists_error("");
	}
	/// <summary>
	/// Query the id of the specific language.
	/// </summary>
	int query_language_id(std::u8string_view locale) const
	{
		std::lock_guard _(m);
		for (const auto& [k, v] : langs)
			if (string(v["locale"]) == locale)
				return v["id"].asInt();
		throw language_not_exists_error("");
	}

public:
	/// <summary>
	/// Query the current set language.
	/// </summary>
	/// <returns>A locale name. <remarks>See enumerate_supported_language.</remarks></returns>
	std::u8string query_current_language() const
	{
		std::lock_guard _(m);
		return string(langs.at(current_language)["locale"]);
	}
	/// <summary>
	/// Query the name of the current set language. For example, "English" may be returned.
	/// </summary>
	/// <returns>A locale name. <remarks>See enumerate_supported_language.</remarks></returns>
	std::u8string query_current_language_name() const
	{
		std::lock_guard _(m);
		return string(langs.at(current_language)["language_name"]);
	}
	/// <summary>
	/// Query the id of the current set language.
	/// </summary>
	int query_current_language_id() const
	{
		return current_language;
	}

public:
	/// <summary>
	/// Set the current language.
	/// </summary>
	/// <param name="locale">A locale name. <remarks>See enumerate_supported_language.</remarks></param>
	/// <returns>If the language does not exist in this multi_language object, returns false. Otherwise returns true.</returns>
	bool set_current_language(std::u8string_view locale)
	{
		std::lock_guard _(m);
		if (!is_language_supported(locale))
			return false;
		current_language = query_language_id(locale);
		return true;
	}
	/// <summary>
	/// Set the current language.
	/// </summary>
	/// <param name="id">The id.</param>
	/// <returns>If the language does not exist in this multi_language object, returns false. Otherwise returns true.</returns>
	bool set_current_language(int id)
	{
		std::lock_guard _(m);
		if (!langs.count(id))
			return false;
		current_language = id;
		return true;
	}

public:
	/// <summary>
	/// Get the text in a specific language according to the key.
	/// </summary>
	std::u8string get_text(int id, std::string_view key) const
	{
		std::lock_guard _(m);
		if (!langs.count(id))
			throw language_not_exists_error("");
		return string(langs.at(id)[key.data()]);
	}
	/// <summary>
	/// Get the text according to the key. The current language will be used.
	/// </summary>
	std::u8string operator[](std::string_view key) const
	{
		std::lock_guard _(m);
		return get_text(current_language, key);
	}
};