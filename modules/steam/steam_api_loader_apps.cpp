/**************************************************************************/
/*  steam_api_loader_apps.cpp                                             */
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

// Steam apps (ISteamApps) symbols and wrappers for SteamAPILoader.

#include "steam_api_loader.h"

bool SteamAPILoader::_load_apps_symbols() {
	void *symbol = nullptr;

	// SteamApps_v009 ships with recent Steamworks SDKs (1.65 has it), v008
	// with older ones. The flat functions below have the same names and
	// signatures in both, so whichever accessor the library has will do.
	if (!_load_symbol("SteamAPI_SteamApps_v009", symbol, true) && !_load_symbol("SteamAPI_SteamApps_v008", symbol, true)) {
		return false;
	}
	fn_steam_apps = (SteamAPI_SteamAppsFn)symbol;

#define LOAD_APPS_SYM(name, field, type) \
	if (!_load_symbol(name, symbol, true)) { \
		return false; \
	} \
	field = (type)symbol;

	LOAD_APPS_SYM("SteamAPI_ISteamApps_GetCurrentGameLanguage", fn_apps_get_current_game_language, ISteamApps_GetLanguageFn);
	LOAD_APPS_SYM("SteamAPI_ISteamApps_GetAvailableGameLanguages", fn_apps_get_available_game_languages, ISteamApps_GetLanguageFn);

#undef LOAD_APPS_SYM
	return true;
}

void SteamAPILoader::_clear_apps_symbols() {
	apps_loaded = false;
	fn_steam_apps = nullptr;
	fn_apps_get_current_game_language = nullptr;
	fn_apps_get_available_game_languages = nullptr;
}

SteamAPILoader::ISteamAppsPtr SteamAPILoader::get_steam_apps() const {
	return fn_steam_apps ? fn_steam_apps() : nullptr;
}

String SteamAPILoader::get_current_game_language(ISteamAppsPtr p_apps) const {
	if (!fn_apps_get_current_game_language || !p_apps) {
		return String();
	}
	const char *language = fn_apps_get_current_game_language(p_apps);
	return language ? String::utf8(language) : String();
}

String SteamAPILoader::get_available_game_languages(ISteamAppsPtr p_apps) const {
	if (!fn_apps_get_available_game_languages || !p_apps) {
		return String();
	}
	const char *languages = fn_apps_get_available_game_languages(p_apps);
	return languages ? String::utf8(languages) : String();
}
