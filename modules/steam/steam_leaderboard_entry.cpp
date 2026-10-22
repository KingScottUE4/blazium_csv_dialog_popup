/**************************************************************************/
/*  steam_leaderboard_entry.cpp                                           */
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

#include "steam_leaderboard_entry.h"

#include "steam.h"

#include "core/object/class_db.h"

void SteamLeaderboardEntry::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_steam_id"), &SteamLeaderboardEntry::get_steam_id);
	ClassDB::bind_method(D_METHOD("get_persona_name"), &SteamLeaderboardEntry::get_persona_name);
	ClassDB::bind_method(D_METHOD("get_global_rank"), &SteamLeaderboardEntry::get_global_rank);
	ClassDB::bind_method(D_METHOD("get_score"), &SteamLeaderboardEntry::get_score);
	ClassDB::bind_method(D_METHOD("get_details"), &SteamLeaderboardEntry::get_details);
	ClassDB::bind_method(D_METHOD("get_ugc_handle"), &SteamLeaderboardEntry::get_ugc_handle);
	ClassDB::bind_method(D_METHOD("is_local_user"), &SteamLeaderboardEntry::is_local_user);
}

bool SteamLeaderboardEntry::is_local_user() const {
	Steam *steam = Steam::get_singleton();
	return steam && steam_id != 0 && steam->get_local_steam_id() == steam_id;
}
