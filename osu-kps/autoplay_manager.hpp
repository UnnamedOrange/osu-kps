// Copyright (c) UnnamedOrange. Licensed under the MIT Licence.
// See the LICENSE file in the repository root for full licence text.

#pragma once

#include <osu_memory/osu_memory.h>

#include "integrated_kps.hpp"
#include "utils/timer_thread.hpp"

class autoplay_manager
{
private:
	mutable osu_memory::reader r;
	const kps::kps* backend;

public:
	enum class status_t
	{
		manual_play,
		auto_play,
		standby,
	};
private:
	status_t get_status() const
	{
		if (backend->get_monitor_implement_type() != kps::key_monitor_implement_type::monitor_implement_type_memory)
			return status_t::manual_play;

		auto t = r.get_is_autoplay();
		if (!t)
			return status_t::standby;
		else if (*t)
			return status_t::auto_play;
		else
			return status_t::manual_play;
	}
	status_t crt_valid_status{ status_t::manual_play }; // 不会是 standy。
	status_t crt_status{ status_t::manual_play };
	status_t previous_status{};
private:
	void update_current_status()
	{
		status_t crt = get_status();
		previous_status = crt_status;
		crt_status = crt;
		if (crt_status != status_t::standby)
			crt_valid_status = crt_status;
	}

private:
	std::function<void(status_t)> callback;
	timer_thread tt{ [this] {
		update_current_status();
		if (crt_status != status_t::standby && crt_status != previous_status)
			callback(crt_valid_status);
	}, 10 };

public:
	autoplay_manager(const kps::kps* backend, std::function<void(status_t)> callback) :
		backend(backend), callback(callback) {}
};