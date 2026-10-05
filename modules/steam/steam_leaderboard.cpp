/**************************************************************************/
/*  steam_leaderboard.cpp                                                 */
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

// Steam leaderboard (ISteamUserStats) support for the Steam singleton.
//
// Like the stats methods, these block until Steam answers (or a timeout
// elapses) so they can be used directly from game code, e.g.
//     var board := Steam.find_leaderboard("track_1_time")
//     Steam.upload_leaderboard_score(board, lap_ms)

#include "steam.h"
#include "steam_types.h"

#include "core/object/class_db.h"
#include "core/os/os.h"
#include "core/os/time.h"

#include <cstring>

namespace {

constexpr double LEADERBOARD_CALL_TIMEOUT_SEC = 10.0;
// Steam rejects DownloadLeaderboardEntriesForUsers with more users than this.
constexpr int LEADERBOARD_MAX_USERS_PER_REQUEST = 100;

} // namespace

void Steam::_bind_leaderboard_methods() {
	ClassDB::bind_method(D_METHOD("has_leaderboard_support"), &Steam::has_leaderboard_support);
	ClassDB::bind_method(D_METHOD("find_leaderboard", "name", "create", "sort", "display"), &Steam::find_leaderboard, DEFVAL(false), DEFVAL(LEADERBOARD_SORT_ASCENDING), DEFVAL(LEADERBOARD_DISPLAY_TIME_MILLISECONDS));
	ClassDB::bind_method(D_METHOD("upload_leaderboard_score", "handle", "score", "keep_best", "details"), &Steam::upload_leaderboard_score, DEFVAL(true), DEFVAL(PackedInt32Array()));
	ClassDB::bind_method(D_METHOD("download_leaderboard_entries", "handle", "request", "range_start", "range_end"), &Steam::download_leaderboard_entries, DEFVAL(LEADERBOARD_GLOBAL), DEFVAL(1), DEFVAL(10));
	ClassDB::bind_method(D_METHOD("download_leaderboard_entries_for_users", "handle", "steam_ids"), &Steam::download_leaderboard_entries_for_users);
	ClassDB::bind_method(D_METHOD("get_leaderboard_entry_count", "handle"), &Steam::get_leaderboard_entry_count);
	ClassDB::bind_method(D_METHOD("get_leaderboard_name", "handle"), &Steam::get_leaderboard_name);
	ClassDB::bind_method(D_METHOD("get_leaderboard_sort_method", "handle"), &Steam::get_leaderboard_sort_method);
	ClassDB::bind_method(D_METHOD("get_leaderboard_display_type", "handle"), &Steam::get_leaderboard_display_type);

	BIND_ENUM_CONSTANT(LEADERBOARD_SORT_NONE);
	BIND_ENUM_CONSTANT(LEADERBOARD_SORT_ASCENDING);
	BIND_ENUM_CONSTANT(LEADERBOARD_SORT_DESCENDING);

	BIND_ENUM_CONSTANT(LEADERBOARD_DISPLAY_NONE);
	BIND_ENUM_CONSTANT(LEADERBOARD_DISPLAY_NUMERIC);
	BIND_ENUM_CONSTANT(LEADERBOARD_DISPLAY_TIME_SECONDS);
	BIND_ENUM_CONSTANT(LEADERBOARD_DISPLAY_TIME_MILLISECONDS);

	BIND_ENUM_CONSTANT(LEADERBOARD_GLOBAL);
	BIND_ENUM_CONSTANT(LEADERBOARD_GLOBAL_AROUND_USER);
	BIND_ENUM_CONSTANT(LEADERBOARD_FRIENDS);
}

bool Steam::_ensure_leaderboards_ready() const {
	return _ensure_stats_ready() && loader.has_leaderboard_support();
}

bool Steam::_wait_for_leaderboard_call(SteamAPICallHandle_t p_call, int p_callback_id, int p_min_size, Vector<uint8_t> &r_data) {
	r_data.clear();
	if (p_call == STEAM_API_CALL_INVALID) {
		return false;
	}
	if (dispatching_callbacks) {
		// Results are delivered by _dispatch_callbacks(), which can't re-enter
		// itself. This happens when called from inside a Steam signal handler.
		ERR_PRINT("Steam leaderboard calls can't be made from inside a Steam signal handler. Use call_deferred() instead.");
		return false;
	}

	leaderboard_calls[p_call] = LeaderboardCallResult();

	const uint64_t start_usec = Time::get_singleton()->get_ticks_usec();
	while (true) {
		_dispatch_callbacks();

		const LeaderboardCallResult *call = leaderboard_calls.getptr(p_call);
		if (call == nullptr) {
			// Steam was shut down while waiting.
			return false;
		}
		if (call->done) {
			const LeaderboardCallResult result = *call;
			leaderboard_calls.erase(p_call);
			if (result.failed) {
				_log_debug(vformat("Leaderboard call %d failed (IO failure)", (int64_t)p_call));
				return false;
			}
			if (result.callback_id != p_callback_id || result.data.size() < p_min_size) {
				_log_debug(vformat("Leaderboard call %d returned unexpected callback %d (%d bytes)", (int64_t)p_call, result.callback_id, result.data.size()));
				return false;
			}
			r_data = result.data;
			return true;
		}

		if ((Time::get_singleton()->get_ticks_usec() - start_usec) / 1000000.0 > LEADERBOARD_CALL_TIMEOUT_SEC) {
			_log_debug(vformat("Timed out waiting for leaderboard call %d", (int64_t)p_call));
			leaderboard_calls.erase(p_call);
			return false;
		}
		::OS::get_singleton()->delay_usec(1000);
	}
}

TypedArray<SteamLeaderboardEntry> Steam::_collect_leaderboard_entries(const Vector<uint8_t> &p_result) {
	TypedArray<SteamLeaderboardEntry> entries;
	SteamLeaderboardScoresDownloaded downloaded;
	memcpy(&downloaded, p_result.ptr(), sizeof(SteamLeaderboardScoresDownloaded));

	for (int i = 0; i < downloaded.m_cEntryCount; i++) {
		SteamLeaderboardEntryData data;
		Vector<int32_t> details;
		if (!loader.get_downloaded_leaderboard_entry(steam_user_stats, downloaded.m_hSteamLeaderboardEntries, i, data, details)) {
			continue;
		}

		uint64_t steam_id = 0;
		memcpy(&steam_id, data.m_steamIDUser, sizeof(steam_id));

		PackedInt32Array packed_details;
		packed_details.resize(details.size());
		for (int j = 0; j < details.size(); j++) {
			packed_details.set(j, details[j]);
		}

		Ref<SteamLeaderboardEntry> entry;
		entry.instantiate();
		entry->set_steam_id(steam_id);
		entry->set_persona_name(loader.get_friend_persona_name(steam_friends, steam_id));
		entry->set_global_rank(data.m_nGlobalRank);
		entry->set_score(data.m_nScore);
		entry->set_details(packed_details);
		entry->set_ugc_handle(data.m_hUGC);
		entries.push_back(entry);
	}
	return entries;
}

bool Steam::has_leaderboard_support() const {
	return loader.has_leaderboard_support();
}

int64_t Steam::find_leaderboard(const String &p_name, bool p_create, LeaderboardSortMethod p_sort, LeaderboardDisplayType p_display) {
	if (!_ensure_leaderboards_ready()) {
		return 0;
	}
	const CharString name = p_name.utf8();
	if (name.length() == 0 || name.length() > STEAM_LEADERBOARD_NAME_MAX) {
		ERR_PRINT(vformat("Steam leaderboard names must be 1 to %d bytes long.", STEAM_LEADERBOARD_NAME_MAX));
		return 0;
	}

	SteamAPICallHandle_t call = STEAM_API_CALL_INVALID;
	if (p_create) {
		if (p_sort == LEADERBOARD_SORT_NONE || p_display == LEADERBOARD_DISPLAY_NONE) {
			ERR_PRINT("Creating a Steam leaderboard needs a sort method and a display type other than NONE.");
			return 0;
		}
		call = loader.find_or_create_leaderboard(steam_user_stats, name.get_data(), (int)p_sort, (int)p_display);
	} else {
		call = loader.find_leaderboard(steam_user_stats, name.get_data());
	}

	Vector<uint8_t> data;
	if (!_wait_for_leaderboard_call(call, SteamLeaderboardFindResult::k_iCallback, (int)sizeof(SteamLeaderboardFindResult), data)) {
		return 0;
	}
	SteamLeaderboardFindResult result;
	memcpy(&result, data.ptr(), sizeof(SteamLeaderboardFindResult));
	if (!result.m_bLeaderboardFound) {
		_log_debug(vformat("Leaderboard '%s' not found", p_name));
		return 0;
	}
	_log_debug(vformat("Leaderboard '%s' -> handle %d", p_name, (int64_t)result.m_hSteamLeaderboard));
	return (int64_t)result.m_hSteamLeaderboard;
}

Dictionary Steam::upload_leaderboard_score(int64_t p_handle, int p_score, bool p_keep_best, const PackedInt32Array &p_details) {
	Dictionary out;
	out["success"] = false;
	out["score_changed"] = false;
	out["global_rank_new"] = 0;
	out["global_rank_previous"] = 0;
	if (!_ensure_leaderboards_ready() || p_handle == 0) {
		return out;
	}
	if (p_details.size() > STEAM_LEADERBOARD_DETAILS_MAX) {
		ERR_PRINT(vformat("Steam leaderboard entries can have at most %d details; extra values are dropped.", STEAM_LEADERBOARD_DETAILS_MAX));
	}

	Vector<int32_t> details;
	for (int i = 0; i < MIN(p_details.size(), STEAM_LEADERBOARD_DETAILS_MAX); i++) {
		details.push_back(p_details[i]);
	}
	// ELeaderboardUploadScoreMethod: KeepBest = 1, ForceUpdate = 2.
	const int method = p_keep_best ? 1 : 2;
	const SteamAPICallHandle_t call = loader.upload_leaderboard_score(steam_user_stats, (SteamLeaderboard_t)p_handle, method, (int32_t)p_score, details);

	Vector<uint8_t> data;
	if (!_wait_for_leaderboard_call(call, SteamLeaderboardScoreUploaded::k_iCallback, (int)sizeof(SteamLeaderboardScoreUploaded), data)) {
		return out;
	}
	SteamLeaderboardScoreUploaded result;
	memcpy(&result, data.ptr(), sizeof(SteamLeaderboardScoreUploaded));
	out["success"] = result.m_bSuccess != 0;
	out["score_changed"] = result.m_bScoreChanged != 0;
	out["global_rank_new"] = result.m_nGlobalRankNew;
	out["global_rank_previous"] = result.m_nGlobalRankPrevious;
	_log_debug(vformat("upload_leaderboard_score(%d, %d) -> success=%d rank=%d", p_handle, p_score, (int)result.m_bSuccess, result.m_nGlobalRankNew));
	return out;
}

TypedArray<SteamLeaderboardEntry> Steam::download_leaderboard_entries(int64_t p_handle, LeaderboardDataRequest p_request, int p_range_start, int p_range_end) {
	if (!_ensure_leaderboards_ready() || p_handle == 0) {
		return TypedArray<SteamLeaderboardEntry>();
	}
	const SteamAPICallHandle_t call = loader.download_leaderboard_entries(steam_user_stats, (SteamLeaderboard_t)p_handle, (int)p_request, p_range_start, p_range_end);
	Vector<uint8_t> data;
	if (!_wait_for_leaderboard_call(call, SteamLeaderboardScoresDownloaded::k_iCallback, (int)sizeof(SteamLeaderboardScoresDownloaded), data)) {
		return TypedArray<SteamLeaderboardEntry>();
	}
	return _collect_leaderboard_entries(data);
}

TypedArray<SteamLeaderboardEntry> Steam::download_leaderboard_entries_for_users(int64_t p_handle, const PackedInt64Array &p_steam_ids) {
	if (!_ensure_leaderboards_ready() || p_handle == 0 || p_steam_ids.is_empty()) {
		return TypedArray<SteamLeaderboardEntry>();
	}
	if (p_steam_ids.size() > LEADERBOARD_MAX_USERS_PER_REQUEST) {
		ERR_PRINT(vformat("Steam allows at most %d users per leaderboard request.", LEADERBOARD_MAX_USERS_PER_REQUEST));
		return TypedArray<SteamLeaderboardEntry>();
	}
	Vector<uint64_t> users;
	for (int i = 0; i < p_steam_ids.size(); i++) {
		users.push_back((uint64_t)p_steam_ids[i]);
	}
	const SteamAPICallHandle_t call = loader.download_leaderboard_entries_for_users(steam_user_stats, (SteamLeaderboard_t)p_handle, users);
	Vector<uint8_t> data;
	if (!_wait_for_leaderboard_call(call, SteamLeaderboardScoresDownloaded::k_iCallback, (int)sizeof(SteamLeaderboardScoresDownloaded), data)) {
		return TypedArray<SteamLeaderboardEntry>();
	}
	return _collect_leaderboard_entries(data);
}

int Steam::get_leaderboard_entry_count(int64_t p_handle) const {
	if (!_ensure_leaderboards_ready() || p_handle == 0) {
		return 0;
	}
	return loader.get_leaderboard_entry_count(steam_user_stats, (SteamLeaderboard_t)p_handle);
}

String Steam::get_leaderboard_name(int64_t p_handle) const {
	if (!_ensure_leaderboards_ready() || p_handle == 0) {
		return String();
	}
	return loader.get_leaderboard_name(steam_user_stats, (SteamLeaderboard_t)p_handle);
}

Steam::LeaderboardSortMethod Steam::get_leaderboard_sort_method(int64_t p_handle) const {
	if (!_ensure_leaderboards_ready() || p_handle == 0) {
		return LEADERBOARD_SORT_NONE;
	}
	return (LeaderboardSortMethod)loader.get_leaderboard_sort_method(steam_user_stats, (SteamLeaderboard_t)p_handle);
}

Steam::LeaderboardDisplayType Steam::get_leaderboard_display_type(int64_t p_handle) const {
	if (!_ensure_leaderboards_ready() || p_handle == 0) {
		return LEADERBOARD_DISPLAY_NONE;
	}
	return (LeaderboardDisplayType)loader.get_leaderboard_display_type(steam_user_stats, (SteamLeaderboard_t)p_handle);
}
