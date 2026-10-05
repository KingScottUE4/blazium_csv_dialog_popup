/**************************************************************************/
/*  steam_api_loader_leaderboard.cpp                                      */
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

// Steam leaderboard (ISteamUserStats) symbols and wrappers for SteamAPILoader.

#include "steam_api_loader.h"

#include <cstring>

bool SteamAPILoader::_load_leaderboard_symbols() {
	void *symbol = nullptr;

#define LOAD_LB_SYM(name, field, type) \
	if (!_load_symbol(name, symbol, true)) { \
		return false; \
	} \
	field = (type)symbol;

	LOAD_LB_SYM("SteamAPI_ISteamUserStats_FindOrCreateLeaderboard", fn_find_or_create_leaderboard, ISteamUserStats_FindOrCreateLeaderboardFn);
	LOAD_LB_SYM("SteamAPI_ISteamUserStats_FindLeaderboard", fn_find_leaderboard, ISteamUserStats_FindLeaderboardFn);
	LOAD_LB_SYM("SteamAPI_ISteamUserStats_GetLeaderboardName", fn_get_leaderboard_name, ISteamUserStats_GetLeaderboardNameFn);
	LOAD_LB_SYM("SteamAPI_ISteamUserStats_GetLeaderboardEntryCount", fn_get_leaderboard_entry_count, ISteamUserStats_GetLeaderboardIntFn);
	LOAD_LB_SYM("SteamAPI_ISteamUserStats_GetLeaderboardSortMethod", fn_get_leaderboard_sort_method, ISteamUserStats_GetLeaderboardIntFn);
	LOAD_LB_SYM("SteamAPI_ISteamUserStats_GetLeaderboardDisplayType", fn_get_leaderboard_display_type, ISteamUserStats_GetLeaderboardIntFn);
	LOAD_LB_SYM("SteamAPI_ISteamUserStats_DownloadLeaderboardEntries", fn_download_leaderboard_entries, ISteamUserStats_DownloadLeaderboardEntriesFn);
	LOAD_LB_SYM("SteamAPI_ISteamUserStats_DownloadLeaderboardEntriesForUsers", fn_download_leaderboard_entries_for_users, ISteamUserStats_DownloadLeaderboardEntriesForUsersFn);
	LOAD_LB_SYM("SteamAPI_ISteamUserStats_GetDownloadedLeaderboardEntry", fn_get_downloaded_leaderboard_entry, ISteamUserStats_GetDownloadedLeaderboardEntryFn);
	LOAD_LB_SYM("SteamAPI_ISteamUserStats_UploadLeaderboardScore", fn_upload_leaderboard_score, ISteamUserStats_UploadLeaderboardScoreFn);
	LOAD_LB_SYM("SteamAPI_ISteamFriends_GetFriendPersonaName", fn_get_friend_persona_name, ISteamFriends_GetFriendPersonaNameFn);

#undef LOAD_LB_SYM
	return true;
}

SteamAPICallHandle_t SteamAPILoader::find_or_create_leaderboard(ISteamUserStatsPtr p_stats, const char *p_name, int p_sort_method, int p_display_type) const {
	return fn_find_or_create_leaderboard ? fn_find_or_create_leaderboard(p_stats, p_name, p_sort_method, p_display_type) : STEAM_API_CALL_INVALID;
}

SteamAPICallHandle_t SteamAPILoader::find_leaderboard(ISteamUserStatsPtr p_stats, const char *p_name) const {
	return fn_find_leaderboard ? fn_find_leaderboard(p_stats, p_name) : STEAM_API_CALL_INVALID;
}

String SteamAPILoader::get_leaderboard_name(ISteamUserStatsPtr p_stats, SteamLeaderboard_t p_leaderboard) const {
	if (!fn_get_leaderboard_name) {
		return String();
	}
	const char *name = fn_get_leaderboard_name(p_stats, p_leaderboard);
	return name ? String::utf8(name) : String();
}

int SteamAPILoader::get_leaderboard_entry_count(ISteamUserStatsPtr p_stats, SteamLeaderboard_t p_leaderboard) const {
	return fn_get_leaderboard_entry_count ? fn_get_leaderboard_entry_count(p_stats, p_leaderboard) : 0;
}

int SteamAPILoader::get_leaderboard_sort_method(ISteamUserStatsPtr p_stats, SteamLeaderboard_t p_leaderboard) const {
	return fn_get_leaderboard_sort_method ? fn_get_leaderboard_sort_method(p_stats, p_leaderboard) : 0;
}

int SteamAPILoader::get_leaderboard_display_type(ISteamUserStatsPtr p_stats, SteamLeaderboard_t p_leaderboard) const {
	return fn_get_leaderboard_display_type ? fn_get_leaderboard_display_type(p_stats, p_leaderboard) : 0;
}

SteamAPICallHandle_t SteamAPILoader::download_leaderboard_entries(ISteamUserStatsPtr p_stats, SteamLeaderboard_t p_leaderboard, int p_request, int p_range_start, int p_range_end) const {
	return fn_download_leaderboard_entries ? fn_download_leaderboard_entries(p_stats, p_leaderboard, p_request, p_range_start, p_range_end) : STEAM_API_CALL_INVALID;
}

SteamAPICallHandle_t SteamAPILoader::download_leaderboard_entries_for_users(ISteamUserStatsPtr p_stats, SteamLeaderboard_t p_leaderboard, const Vector<uint64_t> &p_users) const {
	if (!fn_download_leaderboard_entries_for_users || p_users.is_empty()) {
		return STEAM_API_CALL_INVALID;
	}
	// CSteamID is a byte-packed 64-bit integer, so an array of uint64_t has the
	// same layout as an array of CSteamID.
	Vector<uint64_t> users = p_users;
	return fn_download_leaderboard_entries_for_users(p_stats, p_leaderboard, users.ptrw(), users.size());
}

bool SteamAPILoader::get_downloaded_leaderboard_entry(ISteamUserStatsPtr p_stats, SteamLeaderboardEntries_t p_entries, int p_index, SteamLeaderboardEntryData &r_entry, Vector<int32_t> &r_details) const {
	memset(&r_entry, 0, sizeof(SteamLeaderboardEntryData));
	r_details.clear();
	if (!fn_get_downloaded_leaderboard_entry) {
		return false;
	}
	int32_t details[STEAM_LEADERBOARD_DETAILS_MAX] = {};
	if (!fn_get_downloaded_leaderboard_entry(p_stats, p_entries, p_index, &r_entry, details, STEAM_LEADERBOARD_DETAILS_MAX)) {
		return false;
	}
	const int count = CLAMP(r_entry.m_cDetails, 0, STEAM_LEADERBOARD_DETAILS_MAX);
	r_details.resize(count);
	for (int i = 0; i < count; i++) {
		r_details.write[i] = details[i];
	}
	return true;
}

SteamAPICallHandle_t SteamAPILoader::upload_leaderboard_score(ISteamUserStatsPtr p_stats, SteamLeaderboard_t p_leaderboard, int p_upload_method, int32_t p_score, const Vector<int32_t> &p_details) const {
	if (!fn_upload_leaderboard_score) {
		return STEAM_API_CALL_INVALID;
	}
	const int count = MIN(p_details.size(), STEAM_LEADERBOARD_DETAILS_MAX);
	return fn_upload_leaderboard_score(p_stats, p_leaderboard, p_upload_method, p_score, count > 0 ? p_details.ptr() : nullptr, count);
}

String SteamAPILoader::get_friend_persona_name(ISteamFriendsPtr p_friends, uint64_t p_steam_id) const {
	if (!fn_get_friend_persona_name || !p_friends) {
		return String();
	}
	const char *name = fn_get_friend_persona_name(p_friends, p_steam_id);
	return name ? String::utf8(name) : String();
}
