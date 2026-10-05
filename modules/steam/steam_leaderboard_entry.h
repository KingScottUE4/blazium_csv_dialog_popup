/**************************************************************************/
/*  steam_leaderboard_entry.h                                             */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             BLAZIUM ENGINE                             */
/*                          https://blazium.app                           */
/**************************************************************************/
/* Copyright (c) 2024-present Blazium Engine contributors.                */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#pragma once

#include "core/object/ref_counted.h"

// One row of a Steam leaderboard, as returned by Steam.download_leaderboard_entries().
class SteamLeaderboardEntry : public RefCounted {
	GDCLASS(SteamLeaderboardEntry, RefCounted);

	uint64_t steam_id = 0;
	String persona_name;
	int global_rank = 0;
	int score = 0;
	PackedInt32Array details;
	uint64_t ugc_handle = 0;

protected:
	static void _bind_methods();

public:
	void set_steam_id(uint64_t p_steam_id) { steam_id = p_steam_id; }
	uint64_t get_steam_id() const { return steam_id; }

	void set_persona_name(const String &p_name) { persona_name = p_name; }
	String get_persona_name() const { return persona_name; }

	void set_global_rank(int p_rank) { global_rank = p_rank; }
	int get_global_rank() const { return global_rank; }

	void set_score(int p_score) { score = p_score; }
	int get_score() const { return score; }

	void set_details(const PackedInt32Array &p_details) { details = p_details; }
	PackedInt32Array get_details() const { return details; }

	void set_ugc_handle(uint64_t p_handle) { ugc_handle = p_handle; }
	uint64_t get_ugc_handle() const { return ugc_handle; }

	bool is_local_user() const;
};
