/**************************************************************************/
/*  steam_matchmaking.cpp                                                 */
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

// Steam Networking Sockets routing and Steam Matchmaking (lobbies) for the
// Steam singleton.

#include "steam.h"
#include "steam_multiplayer_peer.h"
#include "steam_types.h"

#include "core/object/class_db.h"
#include "core/os/os.h"

void Steam::_bind_matchmaking_methods() {
	ClassDB::bind_method(D_METHOD("has_networking_support"), &Steam::has_networking_support);
	ClassDB::bind_method(D_METHOD("init_relay_network_access"), &Steam::init_relay_network_access);

	ClassDB::bind_method(D_METHOD("has_matchmaking_support"), &Steam::has_matchmaking_support);
	ClassDB::bind_method(D_METHOD("create_lobby", "type", "max_members"), &Steam::create_lobby, DEFVAL(LOBBY_TYPE_FRIENDS_ONLY), DEFVAL(4));
	ClassDB::bind_method(D_METHOD("join_lobby", "lobby_id"), &Steam::join_lobby);
	ClassDB::bind_method(D_METHOD("leave_lobby", "lobby_id"), &Steam::leave_lobby);
	ClassDB::bind_method(D_METHOD("add_lobby_list_string_filter", "key", "value", "comparison"), &Steam::add_lobby_list_string_filter, DEFVAL(LOBBY_COMPARISON_EQUAL));
	ClassDB::bind_method(D_METHOD("add_lobby_list_numerical_filter", "key", "value", "comparison"), &Steam::add_lobby_list_numerical_filter, DEFVAL(LOBBY_COMPARISON_EQUAL));
	ClassDB::bind_method(D_METHOD("add_lobby_list_slots_available_filter", "slots_available"), &Steam::add_lobby_list_slots_available_filter);
	ClassDB::bind_method(D_METHOD("add_lobby_list_distance_filter", "distance"), &Steam::add_lobby_list_distance_filter);
	ClassDB::bind_method(D_METHOD("add_lobby_list_result_count_filter", "max_results"), &Steam::add_lobby_list_result_count_filter);
	ClassDB::bind_method(D_METHOD("request_lobby_list"), &Steam::request_lobby_list);
	ClassDB::bind_method(D_METHOD("invite_user_to_lobby", "lobby_id", "steam_id"), &Steam::invite_user_to_lobby);
	ClassDB::bind_method(D_METHOD("show_lobby_invite_dialog", "lobby_id"), &Steam::show_lobby_invite_dialog);
	ClassDB::bind_method(D_METHOD("get_num_lobby_members", "lobby_id"), &Steam::get_num_lobby_members);
	ClassDB::bind_method(D_METHOD("get_lobby_members", "lobby_id"), &Steam::get_lobby_members);
	ClassDB::bind_method(D_METHOD("get_lobby_owner", "lobby_id"), &Steam::get_lobby_owner);
	ClassDB::bind_method(D_METHOD("set_lobby_owner", "lobby_id", "steam_id"), &Steam::set_lobby_owner);
	ClassDB::bind_method(D_METHOD("get_lobby_member_limit", "lobby_id"), &Steam::get_lobby_member_limit);
	ClassDB::bind_method(D_METHOD("set_lobby_member_limit", "lobby_id", "max_members"), &Steam::set_lobby_member_limit);
	ClassDB::bind_method(D_METHOD("set_lobby_type", "lobby_id", "type"), &Steam::set_lobby_type);
	ClassDB::bind_method(D_METHOD("set_lobby_joinable", "lobby_id", "joinable"), &Steam::set_lobby_joinable);
	ClassDB::bind_method(D_METHOD("get_lobby_data", "lobby_id", "key"), &Steam::get_lobby_data);
	ClassDB::bind_method(D_METHOD("set_lobby_data", "lobby_id", "key", "value"), &Steam::set_lobby_data);
	ClassDB::bind_method(D_METHOD("delete_lobby_data", "lobby_id", "key"), &Steam::delete_lobby_data);
	ClassDB::bind_method(D_METHOD("get_all_lobby_data", "lobby_id"), &Steam::get_all_lobby_data);
	ClassDB::bind_method(D_METHOD("get_lobby_member_data", "lobby_id", "steam_id", "key"), &Steam::get_lobby_member_data);
	ClassDB::bind_method(D_METHOD("set_lobby_member_data", "lobby_id", "key", "value"), &Steam::set_lobby_member_data);
	ClassDB::bind_method(D_METHOD("get_friend_persona_name", "steam_id"), &Steam::get_friend_persona_name);
	ClassDB::bind_method(D_METHOD("get_launch_lobby_id"), &Steam::get_launch_lobby_id);

	ADD_SIGNAL(MethodInfo("lobby_created", PropertyInfo(Variant::INT, "result"), PropertyInfo(Variant::INT, "lobby_id")));
	ADD_SIGNAL(MethodInfo("lobby_joined", PropertyInfo(Variant::INT, "lobby_id"), PropertyInfo(Variant::INT, "response", PROPERTY_HINT_ENUM, "", PROPERTY_USAGE_CLASS_IS_ENUM, "Steam.LobbyEnterResponse")));
	ADD_SIGNAL(MethodInfo("lobby_list_received", PropertyInfo(Variant::PACKED_INT64_ARRAY, "lobby_ids")));
	ADD_SIGNAL(MethodInfo("lobby_member_state_changed", PropertyInfo(Variant::INT, "lobby_id"), PropertyInfo(Variant::INT, "steam_id"), PropertyInfo(Variant::INT, "state_change", PROPERTY_HINT_FLAGS, "", PROPERTY_USAGE_CLASS_IS_BITFIELD, "Steam.LobbyMemberStateChange"), PropertyInfo(Variant::INT, "changed_by_steam_id")));
	ADD_SIGNAL(MethodInfo("lobby_data_updated", PropertyInfo(Variant::INT, "lobby_id"), PropertyInfo(Variant::INT, "member_steam_id")));
	ADD_SIGNAL(MethodInfo("lobby_invite_received", PropertyInfo(Variant::INT, "inviter_steam_id"), PropertyInfo(Variant::INT, "lobby_id")));
	ADD_SIGNAL(MethodInfo("lobby_join_requested", PropertyInfo(Variant::INT, "lobby_id"), PropertyInfo(Variant::INT, "friend_steam_id")));
	ADD_SIGNAL(MethodInfo("lobby_kicked", PropertyInfo(Variant::INT, "lobby_id"), PropertyInfo(Variant::INT, "admin_steam_id"), PropertyInfo(Variant::BOOL, "due_to_disconnect")));

	BIND_ENUM_CONSTANT(LOBBY_TYPE_PRIVATE);
	BIND_ENUM_CONSTANT(LOBBY_TYPE_FRIENDS_ONLY);
	BIND_ENUM_CONSTANT(LOBBY_TYPE_PUBLIC);
	BIND_ENUM_CONSTANT(LOBBY_TYPE_INVISIBLE);
	BIND_ENUM_CONSTANT(LOBBY_TYPE_PRIVATE_UNIQUE);

	BIND_ENUM_CONSTANT(LOBBY_COMPARISON_EQUAL_TO_OR_LESS_THAN);
	BIND_ENUM_CONSTANT(LOBBY_COMPARISON_LESS_THAN);
	BIND_ENUM_CONSTANT(LOBBY_COMPARISON_EQUAL);
	BIND_ENUM_CONSTANT(LOBBY_COMPARISON_GREATER_THAN);
	BIND_ENUM_CONSTANT(LOBBY_COMPARISON_EQUAL_TO_OR_GREATER_THAN);
	BIND_ENUM_CONSTANT(LOBBY_COMPARISON_NOT_EQUAL);

	BIND_ENUM_CONSTANT(LOBBY_DISTANCE_FILTER_CLOSE);
	BIND_ENUM_CONSTANT(LOBBY_DISTANCE_FILTER_DEFAULT);
	BIND_ENUM_CONSTANT(LOBBY_DISTANCE_FILTER_FAR);
	BIND_ENUM_CONSTANT(LOBBY_DISTANCE_FILTER_WORLDWIDE);

	BIND_BITFIELD_FLAG(LOBBY_MEMBER_ENTERED);
	BIND_BITFIELD_FLAG(LOBBY_MEMBER_LEFT);
	BIND_BITFIELD_FLAG(LOBBY_MEMBER_DISCONNECTED);
	BIND_BITFIELD_FLAG(LOBBY_MEMBER_KICKED);
	BIND_BITFIELD_FLAG(LOBBY_MEMBER_BANNED);

	BIND_ENUM_CONSTANT(LOBBY_ENTER_SUCCESS);
	BIND_ENUM_CONSTANT(LOBBY_ENTER_DOESNT_EXIST);
	BIND_ENUM_CONSTANT(LOBBY_ENTER_NOT_ALLOWED);
	BIND_ENUM_CONSTANT(LOBBY_ENTER_FULL);
	BIND_ENUM_CONSTANT(LOBBY_ENTER_ERROR);
	BIND_ENUM_CONSTANT(LOBBY_ENTER_BANNED);
	BIND_ENUM_CONSTANT(LOBBY_ENTER_LIMITED);
	BIND_ENUM_CONSTANT(LOBBY_ENTER_CLAN_DISABLED);
	BIND_ENUM_CONSTANT(LOBBY_ENTER_COMMUNITY_BAN);
	BIND_ENUM_CONSTANT(LOBBY_ENTER_MEMBER_BLOCKED_YOU);
	BIND_ENUM_CONSTANT(LOBBY_ENTER_YOU_BLOCKED_MEMBER);
}

// -----------------------------------------------------------------------------
// Callback routing
// -----------------------------------------------------------------------------

bool Steam::_ensure_matchmaking_ready() const {
	return initialized && steam_matchmaking && loader.has_matchmaking_support();
}

bool Steam::_track_lobby_call(SteamAPICallHandle_t p_call, int p_callback_id, uint64_t p_lobby_id) {
	if (p_call == STEAM_API_CALL_INVALID) {
		return false;
	}
	LobbyPendingCall pending;
	pending.callback_id = p_callback_id;
	pending.lobby_id = p_lobby_id;
	lobby_pending_calls[p_call] = pending;
	return true;
}

void Steam::_clear_networking_state() {
	// Peers can't outlive the Steam session: close them so SceneMultiplayer
	// reports the disconnection.
	LocalVector<SteamSocketsTransport *> transports(networking_transports);
	for (SteamSocketsTransport *transport : transports) {
		transport->on_steam_shutdown();
	}
	lobby_pending_calls.clear();
	steam_networking_sockets = nullptr;
	steam_networking_utils = nullptr;
	steam_matchmaking = nullptr;
	relay_network_access_initialized = false;
}

bool Steam::_handle_lobby_call_completed(uint64_t p_call, bool p_success, const uint8_t *p_data, int p_size) {
	HashMap<uint64_t, LobbyPendingCall>::Iterator E = lobby_pending_calls.find(p_call);
	if (!E) {
		return false;
	}
	const LobbyPendingCall call = E->value;
	lobby_pending_calls.remove(E);

	switch (call.callback_id) {
		case SteamLobbyCreated::k_iCallback: {
			if (!p_success || p_size < (int)sizeof(SteamLobbyCreated)) {
				_log_debug("CreateLobby failed with an IO failure");
				emit_signal(SNAME("lobby_created"), (int)STEAM_RESULT_IO_FAILURE, (int64_t)0);
				break;
			}
			const SteamLobbyCreated *response = (const SteamLobbyCreated *)p_data;
			_log_debug(vformat("Lobby created id=%d result=%d", (int64_t)response->m_ulSteamIDLobby, response->m_eResult));
			emit_signal(SNAME("lobby_created"), response->m_eResult, (int64_t)response->m_ulSteamIDLobby);
			if (response->m_eResult == STEAM_RESULT_OK) {
				// The creator is already in the lobby.
				emit_signal(SNAME("lobby_joined"), (int64_t)response->m_ulSteamIDLobby, (int)LOBBY_ENTER_SUCCESS);
			}
		} break;
		case SteamLobbyEnter::k_iCallback: {
			if (!p_success || p_size < (int)sizeof(SteamLobbyEnter)) {
				_log_debug(vformat("JoinLobby %d failed with an IO failure", (int64_t)call.lobby_id));
				emit_signal(SNAME("lobby_joined"), (int64_t)call.lobby_id, (int)LOBBY_ENTER_ERROR);
				break;
			}
			const SteamLobbyEnter *response = (const SteamLobbyEnter *)p_data;
			_log_debug(vformat("Lobby entered id=%d response=%d", (int64_t)response->m_ulSteamIDLobby, (int)response->m_EChatRoomEnterResponse));
			emit_signal(SNAME("lobby_joined"), (int64_t)response->m_ulSteamIDLobby, (int)response->m_EChatRoomEnterResponse);
		} break;
		case SteamLobbyMatchList::k_iCallback: {
			PackedInt64Array lobbies;
			if (p_success && p_size >= (int)sizeof(SteamLobbyMatchList) && _ensure_matchmaking_ready()) {
				const SteamLobbyMatchList *response = (const SteamLobbyMatchList *)p_data;
				for (uint32_t i = 0; i < response->m_nLobbiesMatching; i++) {
					const uint64_t lobby_id = loader.mm_get_lobby_by_index(steam_matchmaking, (int)i);
					if (lobby_id != 0) {
						lobbies.push_back((int64_t)lobby_id);
					}
				}
			}
			_log_debug(vformat("Lobby list received (%d lobbies)", lobbies.size()));
			emit_signal(SNAME("lobby_list_received"), lobbies);
		} break;
		default:
			break;
	}
	return true;
}

bool Steam::_handle_networking_callback(int p_callback_id, const void *p_data, int p_size) {
	switch (p_callback_id) {
		case SteamNetConnectionStatusChanged::k_iCallback: {
			if (p_size < (int)sizeof(SteamNetConnectionStatusChanged)) {
				return true;
			}
			const SteamNetConnectionStatusChanged *status = (const SteamNetConnectionStatusChanged *)p_data;
			SteamNetConnectionEvent event;
			event.connection = status->m_hConn;
			event.listen_socket = status->m_info.m_hListenSocket;
			if (status->m_info.m_identityRemote.m_eType == STEAM_NETWORKING_IDENTITY_TYPE_STEAM_ID) {
				event.remote_steam_id = status->m_info.m_identityRemote.m_steamID64;
			}
			event.old_state = status->m_eOldState;
			event.state = status->m_info.m_eState;
			event.end_reason = status->m_info.m_eEndReason;
			char end_debug[STEAM_NETWORKING_MAX_CONNECTION_CLOSE_REASON + 1] = {};
			memcpy(end_debug, status->m_info.m_szEndDebug, STEAM_NETWORKING_MAX_CONNECTION_CLOSE_REASON);
			event.end_debug = String::utf8(end_debug);
			_log_debug(vformat("Connection %d state %d -> %d (end reason %d)", (int64_t)event.connection, event.old_state, event.state, event.end_reason));

			LocalVector<SteamSocketsTransport *> transports(networking_transports);
			for (SteamSocketsTransport *transport : transports) {
				transport->on_connection_status_changed(event);
			}
			return true;
		}
		case SteamLobbyInvite::k_iCallback: {
			if (p_size < (int)sizeof(SteamLobbyInvite)) {
				return true;
			}
			const SteamLobbyInvite *invite = (const SteamLobbyInvite *)p_data;
			emit_signal(SNAME("lobby_invite_received"), (int64_t)invite->m_ulSteamIDUser, (int64_t)invite->m_ulSteamIDLobby);
			return true;
		}
		case SteamLobbyDataUpdate::k_iCallback: {
			if (p_size < (int)sizeof(SteamLobbyDataUpdate)) {
				return true;
			}
			const SteamLobbyDataUpdate *update = (const SteamLobbyDataUpdate *)p_data;
			if (update->m_bSuccess) {
				emit_signal(SNAME("lobby_data_updated"), (int64_t)update->m_ulSteamIDLobby, (int64_t)update->m_ulSteamIDMember);
			}
			return true;
		}
		case SteamLobbyChatUpdate::k_iCallback: {
			if (p_size < (int)sizeof(SteamLobbyChatUpdate)) {
				return true;
			}
			const SteamLobbyChatUpdate *update = (const SteamLobbyChatUpdate *)p_data;
			emit_signal(SNAME("lobby_member_state_changed"), (int64_t)update->m_ulSteamIDLobby, (int64_t)update->m_ulSteamIDUserChanged, (int64_t)update->m_rgfChatMemberStateChange, (int64_t)update->m_ulSteamIDMakingChange);
			return true;
		}
		case SteamLobbyKicked::k_iCallback: {
			if (p_size < (int)sizeof(SteamLobbyKicked)) {
				return true;
			}
			const SteamLobbyKicked *kicked = (const SteamLobbyKicked *)p_data;
			emit_signal(SNAME("lobby_kicked"), (int64_t)kicked->m_ulSteamIDLobby, (int64_t)kicked->m_ulSteamIDAdmin, kicked->m_bKickedDueToDisconnect != 0);
			return true;
		}
		case SteamGameLobbyJoinRequested::k_iCallback: {
			if (p_size < (int)sizeof(SteamGameLobbyJoinRequested)) {
				return true;
			}
			const SteamGameLobbyJoinRequested *request = (const SteamGameLobbyJoinRequested *)p_data;
			emit_signal(SNAME("lobby_join_requested"), (int64_t)request->m_steamIDLobby, (int64_t)request->m_steamIDFriend);
			return true;
		}
		case SteamLobbyCreated::k_iCallback:
		case SteamLobbyEnter::k_iCallback:
		case SteamLobbyMatchList::k_iCallback:
			// Reported through the call results of create_lobby(), join_lobby()
			// and request_lobby_list(); ignore the broadcast copies.
			return true;
		default:
			return false;
	}
}

// -----------------------------------------------------------------------------
// Networking
// -----------------------------------------------------------------------------

bool Steam::has_networking_support() const {
	return loader.has_networking_support();
}

void Steam::init_relay_network_access() {
	if (!initialized || relay_network_access_initialized || !steam_networking_utils) {
		return;
	}
	loader.net_init_relay_network_access(steam_networking_utils);
	relay_network_access_initialized = true;
}

void Steam::register_networking_transport(SteamSocketsTransport *p_transport) {
	ERR_FAIL_NULL(p_transport);
	if (networking_transports.find(p_transport) < 0) {
		networking_transports.push_back(p_transport);
	}
}

void Steam::unregister_networking_transport(SteamSocketsTransport *p_transport) {
	networking_transports.erase(p_transport);
}

// -----------------------------------------------------------------------------
// Lobbies
// -----------------------------------------------------------------------------

bool Steam::has_matchmaking_support() const {
	return loader.has_matchmaking_support();
}

bool Steam::create_lobby(LobbyType p_type, int p_max_members) {
	ERR_FAIL_COND_V_MSG(p_max_members < 1 || p_max_members > 250, false, "Lobbies can have between 1 and 250 members.");
	if (!_ensure_matchmaking_ready()) {
		return false;
	}
	const SteamAPICallHandle_t call = loader.mm_create_lobby(steam_matchmaking, (int)p_type, p_max_members);
	return _track_lobby_call(call, SteamLobbyCreated::k_iCallback);
}

bool Steam::join_lobby(uint64_t p_lobby_id) {
	if (!_ensure_matchmaking_ready() || p_lobby_id == 0) {
		return false;
	}
	const SteamAPICallHandle_t call = loader.mm_join_lobby(steam_matchmaking, p_lobby_id);
	return _track_lobby_call(call, SteamLobbyEnter::k_iCallback, p_lobby_id);
}

void Steam::leave_lobby(uint64_t p_lobby_id) {
	if (_ensure_matchmaking_ready() && p_lobby_id != 0) {
		loader.mm_leave_lobby(steam_matchmaking, p_lobby_id);
	}
}

void Steam::add_lobby_list_string_filter(const String &p_key, const String &p_value, LobbyComparison p_comparison) {
	if (_ensure_matchmaking_ready()) {
		loader.mm_add_request_lobby_list_string_filter(steam_matchmaking, p_key.utf8().get_data(), p_value.utf8().get_data(), (int)p_comparison);
	}
}

void Steam::add_lobby_list_numerical_filter(const String &p_key, int p_value, LobbyComparison p_comparison) {
	if (_ensure_matchmaking_ready()) {
		loader.mm_add_request_lobby_list_numerical_filter(steam_matchmaking, p_key.utf8().get_data(), p_value, (int)p_comparison);
	}
}

void Steam::add_lobby_list_slots_available_filter(int p_slots_available) {
	if (_ensure_matchmaking_ready()) {
		loader.mm_add_request_lobby_list_filter_slots_available(steam_matchmaking, p_slots_available);
	}
}

void Steam::add_lobby_list_distance_filter(LobbyDistanceFilter p_distance) {
	if (_ensure_matchmaking_ready()) {
		loader.mm_add_request_lobby_list_distance_filter(steam_matchmaking, (int)p_distance);
	}
}

void Steam::add_lobby_list_result_count_filter(int p_max_results) {
	if (_ensure_matchmaking_ready()) {
		loader.mm_add_request_lobby_list_result_count_filter(steam_matchmaking, p_max_results);
	}
}

bool Steam::request_lobby_list() {
	if (!_ensure_matchmaking_ready()) {
		return false;
	}
	const SteamAPICallHandle_t call = loader.mm_request_lobby_list(steam_matchmaking);
	return _track_lobby_call(call, SteamLobbyMatchList::k_iCallback);
}

bool Steam::invite_user_to_lobby(uint64_t p_lobby_id, uint64_t p_steam_id) {
	if (!_ensure_matchmaking_ready()) {
		return false;
	}
	return loader.mm_invite_user_to_lobby(steam_matchmaking, p_lobby_id, p_steam_id);
}

void Steam::show_lobby_invite_dialog(uint64_t p_lobby_id) {
	if (initialized && steam_friends) {
		loader.friends_activate_game_overlay_invite_dialog(steam_friends, p_lobby_id);
	}
}

int Steam::get_num_lobby_members(uint64_t p_lobby_id) const {
	if (!_ensure_matchmaking_ready()) {
		return 0;
	}
	return loader.mm_get_num_lobby_members(steam_matchmaking, p_lobby_id);
}

PackedInt64Array Steam::get_lobby_members(uint64_t p_lobby_id) const {
	PackedInt64Array members;
	if (!_ensure_matchmaking_ready()) {
		return members;
	}
	const int count = loader.mm_get_num_lobby_members(steam_matchmaking, p_lobby_id);
	for (int i = 0; i < count; i++) {
		const uint64_t member = loader.mm_get_lobby_member_by_index(steam_matchmaking, p_lobby_id, i);
		if (member != 0) {
			members.push_back((int64_t)member);
		}
	}
	return members;
}

uint64_t Steam::get_lobby_owner(uint64_t p_lobby_id) const {
	if (!_ensure_matchmaking_ready()) {
		return 0;
	}
	return loader.mm_get_lobby_owner(steam_matchmaking, p_lobby_id);
}

bool Steam::set_lobby_owner(uint64_t p_lobby_id, uint64_t p_steam_id) {
	if (!_ensure_matchmaking_ready()) {
		return false;
	}
	return loader.mm_set_lobby_owner(steam_matchmaking, p_lobby_id, p_steam_id);
}

int Steam::get_lobby_member_limit(uint64_t p_lobby_id) const {
	if (!_ensure_matchmaking_ready()) {
		return 0;
	}
	return loader.mm_get_lobby_member_limit(steam_matchmaking, p_lobby_id);
}

bool Steam::set_lobby_member_limit(uint64_t p_lobby_id, int p_max_members) {
	if (!_ensure_matchmaking_ready()) {
		return false;
	}
	return loader.mm_set_lobby_member_limit(steam_matchmaking, p_lobby_id, p_max_members);
}

bool Steam::set_lobby_type(uint64_t p_lobby_id, LobbyType p_type) {
	if (!_ensure_matchmaking_ready()) {
		return false;
	}
	return loader.mm_set_lobby_type(steam_matchmaking, p_lobby_id, (int)p_type);
}

bool Steam::set_lobby_joinable(uint64_t p_lobby_id, bool p_joinable) {
	if (!_ensure_matchmaking_ready()) {
		return false;
	}
	return loader.mm_set_lobby_joinable(steam_matchmaking, p_lobby_id, p_joinable);
}

String Steam::get_lobby_data(uint64_t p_lobby_id, const String &p_key) const {
	if (!_ensure_matchmaking_ready()) {
		return String();
	}
	return loader.mm_get_lobby_data(steam_matchmaking, p_lobby_id, p_key.utf8().get_data());
}

bool Steam::set_lobby_data(uint64_t p_lobby_id, const String &p_key, const String &p_value) {
	if (!_ensure_matchmaking_ready()) {
		return false;
	}
	return loader.mm_set_lobby_data(steam_matchmaking, p_lobby_id, p_key.utf8().get_data(), p_value.utf8().get_data());
}

bool Steam::delete_lobby_data(uint64_t p_lobby_id, const String &p_key) {
	if (!_ensure_matchmaking_ready()) {
		return false;
	}
	return loader.mm_delete_lobby_data(steam_matchmaking, p_lobby_id, p_key.utf8().get_data());
}

Dictionary Steam::get_all_lobby_data(uint64_t p_lobby_id) const {
	if (!_ensure_matchmaking_ready()) {
		return Dictionary();
	}
	return loader.mm_get_all_lobby_data(steam_matchmaking, p_lobby_id);
}

String Steam::get_lobby_member_data(uint64_t p_lobby_id, uint64_t p_steam_id, const String &p_key) const {
	if (!_ensure_matchmaking_ready()) {
		return String();
	}
	return loader.mm_get_lobby_member_data(steam_matchmaking, p_lobby_id, p_steam_id, p_key.utf8().get_data());
}

void Steam::set_lobby_member_data(uint64_t p_lobby_id, const String &p_key, const String &p_value) {
	if (_ensure_matchmaking_ready()) {
		loader.mm_set_lobby_member_data(steam_matchmaking, p_lobby_id, p_key.utf8().get_data(), p_value.utf8().get_data());
	}
}

String Steam::get_friend_persona_name(uint64_t p_steam_id) const {
	if (!initialized || !steam_friends) {
		return String();
	}
	return loader.friends_get_friend_persona_name(steam_friends, p_steam_id);
}

uint64_t Steam::get_launch_lobby_id() const {
	// Steam starts the game with "+connect_lobby <id>" when the player accepts
	// an invite or joins a friend while the game isn't running.
	List<String> args = OS::get_singleton()->get_cmdline_args();
	for (const String &arg : OS::get_singleton()->get_cmdline_user_args()) {
		args.push_back(arg);
	}
	for (List<String>::Element *E = args.front(); E; E = E->next()) {
		if (E->get() == "+connect_lobby" && E->next()) {
			const String value = E->next()->get();
			if (value.is_valid_int()) {
				return (uint64_t)value.to_int();
			}
		}
	}
	return 0;
}
