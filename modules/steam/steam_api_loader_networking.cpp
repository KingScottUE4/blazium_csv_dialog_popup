/**************************************************************************/
/*  steam_api_loader_networking.cpp                                       */
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

// Steam Networking Sockets and Matchmaking (lobby) symbols and wrappers for
// SteamAPILoader.

#include "steam_api_loader.h"

#include <cstring>

#define LOAD_STEAM_SYM(name, field, type) \
	if (!_load_symbol(name, symbol)) { \
		return false; \
	} \
	field = (type)symbol;

bool SteamAPILoader::_load_networking_symbols() {
	void *symbol = nullptr;

	// SteamNetworkingSockets_SteamAPI_v012 and SteamNetworkingUtils_SteamAPI_v004
	// ship with every Steamworks SDK from 1.53 to 1.64.
	if (!_load_symbol("SteamAPI_SteamNetworkingSockets_SteamAPI_v012", symbol, true)) {
		return false;
	}
	fn_steam_networking_sockets = (SteamAPI_SteamNetworkingSocketsFn)symbol;

	LOAD_STEAM_SYM("SteamAPI_ISteamNetworkingSockets_CreateListenSocketP2P", fn_net_create_listen_socket_p2p, ISteamNetworkingSockets_CreateListenSocketP2PFn);
	LOAD_STEAM_SYM("SteamAPI_ISteamNetworkingSockets_ConnectP2P", fn_net_connect_p2p, ISteamNetworkingSockets_ConnectP2PFn);
	LOAD_STEAM_SYM("SteamAPI_ISteamNetworkingSockets_AcceptConnection", fn_net_accept_connection, ISteamNetworkingSockets_AcceptConnectionFn);
	LOAD_STEAM_SYM("SteamAPI_ISteamNetworkingSockets_CloseConnection", fn_net_close_connection, ISteamNetworkingSockets_CloseConnectionFn);
	LOAD_STEAM_SYM("SteamAPI_ISteamNetworkingSockets_CloseListenSocket", fn_net_close_listen_socket, ISteamNetworkingSockets_CloseListenSocketFn);
	LOAD_STEAM_SYM("SteamAPI_ISteamNetworkingSockets_SendMessageToConnection", fn_net_send_message_to_connection, ISteamNetworkingSockets_SendMessageToConnectionFn);
	LOAD_STEAM_SYM("SteamAPI_ISteamNetworkingSockets_FlushMessagesOnConnection", fn_net_flush_messages_on_connection, ISteamNetworkingSockets_FlushMessagesOnConnectionFn);
	LOAD_STEAM_SYM("SteamAPI_ISteamNetworkingSockets_GetConnectionInfo", fn_net_get_connection_info, ISteamNetworkingSockets_GetConnectionInfoFn);
	LOAD_STEAM_SYM("SteamAPI_ISteamNetworkingSockets_CreatePollGroup", fn_net_create_poll_group, ISteamNetworkingSockets_CreatePollGroupFn);
	LOAD_STEAM_SYM("SteamAPI_ISteamNetworkingSockets_DestroyPollGroup", fn_net_destroy_poll_group, ISteamNetworkingSockets_DestroyPollGroupFn);
	LOAD_STEAM_SYM("SteamAPI_ISteamNetworkingSockets_SetConnectionPollGroup", fn_net_set_connection_poll_group, ISteamNetworkingSockets_SetConnectionPollGroupFn);
	LOAD_STEAM_SYM("SteamAPI_ISteamNetworkingSockets_ReceiveMessagesOnPollGroup", fn_net_receive_messages_on_poll_group, ISteamNetworkingSockets_ReceiveMessagesOnPollGroupFn);
	LOAD_STEAM_SYM("SteamAPI_SteamNetworkingMessage_t_Release", fn_net_message_release, SteamNetworkingMessage_ReleaseFn);

	// Optional: ping statistics and early relay initialization.
	if (_load_symbol("SteamAPI_ISteamNetworkingSockets_GetConnectionRealTimeStatus", symbol, true)) {
		fn_net_get_connection_real_time_status = (ISteamNetworkingSockets_GetConnectionRealTimeStatusFn)symbol;
	}
	if (_load_symbol("SteamAPI_SteamNetworkingUtils_SteamAPI_v004", symbol, true)) {
		fn_steam_networking_utils = (SteamAPI_SteamNetworkingUtilsFn)symbol;
		if (_load_symbol("SteamAPI_ISteamNetworkingUtils_InitRelayNetworkAccess", symbol, true)) {
			fn_net_init_relay_network_access = (ISteamNetworkingUtils_InitRelayNetworkAccessFn)symbol;
		}
	}

	return true;
}

void SteamAPILoader::_clear_networking_symbols() {
	networking_loaded = false;
	fn_steam_networking_sockets = nullptr;
	fn_steam_networking_utils = nullptr;
	fn_net_create_listen_socket_p2p = nullptr;
	fn_net_connect_p2p = nullptr;
	fn_net_accept_connection = nullptr;
	fn_net_close_connection = nullptr;
	fn_net_close_listen_socket = nullptr;
	fn_net_send_message_to_connection = nullptr;
	fn_net_flush_messages_on_connection = nullptr;
	fn_net_get_connection_info = nullptr;
	fn_net_get_connection_real_time_status = nullptr;
	fn_net_create_poll_group = nullptr;
	fn_net_destroy_poll_group = nullptr;
	fn_net_set_connection_poll_group = nullptr;
	fn_net_receive_messages_on_poll_group = nullptr;
	fn_net_message_release = nullptr;
	fn_net_init_relay_network_access = nullptr;
}

bool SteamAPILoader::_load_matchmaking_symbols() {
	void *symbol = nullptr;

	if (!_load_symbol("SteamAPI_SteamMatchmaking_v009", symbol, true)) {
		return false;
	}
	fn_steam_matchmaking = (SteamAPI_SteamMatchmakingFn)symbol;

	LOAD_STEAM_SYM("SteamAPI_ISteamMatchmaking_RequestLobbyList", fn_mm_request_lobby_list, ISteamMatchmaking_RequestLobbyListFn);
	LOAD_STEAM_SYM("SteamAPI_ISteamMatchmaking_AddRequestLobbyListStringFilter", fn_mm_add_string_filter, ISteamMatchmaking_AddStringFilterFn);
	LOAD_STEAM_SYM("SteamAPI_ISteamMatchmaking_AddRequestLobbyListNumericalFilter", fn_mm_add_numerical_filter, ISteamMatchmaking_AddNumericalFilterFn);
	LOAD_STEAM_SYM("SteamAPI_ISteamMatchmaking_AddRequestLobbyListFilterSlotsAvailable", fn_mm_add_slots_available_filter, ISteamMatchmaking_AddIntFilterFn);
	LOAD_STEAM_SYM("SteamAPI_ISteamMatchmaking_AddRequestLobbyListDistanceFilter", fn_mm_add_distance_filter, ISteamMatchmaking_AddIntFilterFn);
	LOAD_STEAM_SYM("SteamAPI_ISteamMatchmaking_AddRequestLobbyListResultCountFilter", fn_mm_add_result_count_filter, ISteamMatchmaking_AddIntFilterFn);
	LOAD_STEAM_SYM("SteamAPI_ISteamMatchmaking_GetLobbyByIndex", fn_mm_get_lobby_by_index, ISteamMatchmaking_GetLobbyByIndexFn);
	LOAD_STEAM_SYM("SteamAPI_ISteamMatchmaking_CreateLobby", fn_mm_create_lobby, ISteamMatchmaking_CreateLobbyFn);
	LOAD_STEAM_SYM("SteamAPI_ISteamMatchmaking_JoinLobby", fn_mm_join_lobby, ISteamMatchmaking_JoinLobbyFn);
	LOAD_STEAM_SYM("SteamAPI_ISteamMatchmaking_LeaveLobby", fn_mm_leave_lobby, ISteamMatchmaking_LeaveLobbyFn);
	LOAD_STEAM_SYM("SteamAPI_ISteamMatchmaking_InviteUserToLobby", fn_mm_invite_user_to_lobby, ISteamMatchmaking_LobbyUserBoolFn);
	LOAD_STEAM_SYM("SteamAPI_ISteamMatchmaking_GetNumLobbyMembers", fn_mm_get_num_lobby_members, ISteamMatchmaking_LobbyIntFn);
	LOAD_STEAM_SYM("SteamAPI_ISteamMatchmaking_GetLobbyMemberByIndex", fn_mm_get_lobby_member_by_index, ISteamMatchmaking_GetLobbyMemberByIndexFn);
	LOAD_STEAM_SYM("SteamAPI_ISteamMatchmaking_GetLobbyData", fn_mm_get_lobby_data, ISteamMatchmaking_GetLobbyDataFn);
	LOAD_STEAM_SYM("SteamAPI_ISteamMatchmaking_SetLobbyData", fn_mm_set_lobby_data, ISteamMatchmaking_SetLobbyDataFn);
	LOAD_STEAM_SYM("SteamAPI_ISteamMatchmaking_GetLobbyDataCount", fn_mm_get_lobby_data_count, ISteamMatchmaking_LobbyIntFn);
	LOAD_STEAM_SYM("SteamAPI_ISteamMatchmaking_GetLobbyDataByIndex", fn_mm_get_lobby_data_by_index, ISteamMatchmaking_GetLobbyDataByIndexFn);
	LOAD_STEAM_SYM("SteamAPI_ISteamMatchmaking_DeleteLobbyData", fn_mm_delete_lobby_data, ISteamMatchmaking_DeleteLobbyDataFn);
	LOAD_STEAM_SYM("SteamAPI_ISteamMatchmaking_GetLobbyMemberData", fn_mm_get_lobby_member_data, ISteamMatchmaking_GetLobbyMemberDataFn);
	LOAD_STEAM_SYM("SteamAPI_ISteamMatchmaking_SetLobbyMemberData", fn_mm_set_lobby_member_data, ISteamMatchmaking_SetLobbyMemberDataFn);
	LOAD_STEAM_SYM("SteamAPI_ISteamMatchmaking_SetLobbyMemberLimit", fn_mm_set_lobby_member_limit, ISteamMatchmaking_LobbyIntSetterFn);
	LOAD_STEAM_SYM("SteamAPI_ISteamMatchmaking_GetLobbyMemberLimit", fn_mm_get_lobby_member_limit, ISteamMatchmaking_LobbyIntFn);
	LOAD_STEAM_SYM("SteamAPI_ISteamMatchmaking_SetLobbyType", fn_mm_set_lobby_type, ISteamMatchmaking_LobbyIntSetterFn);
	LOAD_STEAM_SYM("SteamAPI_ISteamMatchmaking_SetLobbyJoinable", fn_mm_set_lobby_joinable, ISteamMatchmaking_SetLobbyJoinableFn);
	LOAD_STEAM_SYM("SteamAPI_ISteamMatchmaking_GetLobbyOwner", fn_mm_get_lobby_owner, ISteamMatchmaking_GetLobbyOwnerFn);
	LOAD_STEAM_SYM("SteamAPI_ISteamMatchmaking_SetLobbyOwner", fn_mm_set_lobby_owner, ISteamMatchmaking_LobbyUserBoolFn);

	// Optional ISteamFriends helpers used by the lobby API.
	if (_load_symbol("SteamAPI_ISteamFriends_ActivateGameOverlayInviteDialog", symbol, true)) {
		fn_friends_activate_game_overlay_invite_dialog = (ISteamFriends_ActivateGameOverlayInviteDialogFn)symbol;
	}
	if (_load_symbol("SteamAPI_ISteamFriends_GetFriendPersonaName", symbol, true)) {
		fn_friends_get_friend_persona_name = (ISteamFriends_GetFriendPersonaNameFn)symbol;
	}

	return true;
}

#undef LOAD_STEAM_SYM

void SteamAPILoader::_clear_matchmaking_symbols() {
	matchmaking_loaded = false;
	fn_steam_matchmaking = nullptr;
	fn_mm_request_lobby_list = nullptr;
	fn_mm_add_string_filter = nullptr;
	fn_mm_add_numerical_filter = nullptr;
	fn_mm_add_slots_available_filter = nullptr;
	fn_mm_add_distance_filter = nullptr;
	fn_mm_add_result_count_filter = nullptr;
	fn_mm_get_lobby_by_index = nullptr;
	fn_mm_create_lobby = nullptr;
	fn_mm_join_lobby = nullptr;
	fn_mm_leave_lobby = nullptr;
	fn_mm_invite_user_to_lobby = nullptr;
	fn_mm_get_num_lobby_members = nullptr;
	fn_mm_get_lobby_member_by_index = nullptr;
	fn_mm_get_lobby_data = nullptr;
	fn_mm_set_lobby_data = nullptr;
	fn_mm_get_lobby_data_count = nullptr;
	fn_mm_get_lobby_data_by_index = nullptr;
	fn_mm_delete_lobby_data = nullptr;
	fn_mm_get_lobby_member_data = nullptr;
	fn_mm_set_lobby_member_data = nullptr;
	fn_mm_set_lobby_member_limit = nullptr;
	fn_mm_get_lobby_member_limit = nullptr;
	fn_mm_set_lobby_type = nullptr;
	fn_mm_set_lobby_joinable = nullptr;
	fn_mm_get_lobby_owner = nullptr;
	fn_mm_set_lobby_owner = nullptr;
	fn_friends_activate_game_overlay_invite_dialog = nullptr;
	fn_friends_get_friend_persona_name = nullptr;
}

// Steam Networking Sockets.

SteamAPILoader::ISteamNetworkingSocketsPtr SteamAPILoader::get_steam_networking_sockets() const {
	return fn_steam_networking_sockets ? fn_steam_networking_sockets() : nullptr;
}

SteamAPILoader::ISteamNetworkingUtilsPtr SteamAPILoader::get_steam_networking_utils() const {
	return fn_steam_networking_utils ? fn_steam_networking_utils() : nullptr;
}

void SteamAPILoader::net_init_relay_network_access(ISteamNetworkingUtilsPtr p_utils) const {
	if (fn_net_init_relay_network_access && p_utils) {
		fn_net_init_relay_network_access(p_utils);
	}
}

SteamHSteamListenSocket SteamAPILoader::net_create_listen_socket_p2p(ISteamNetworkingSocketsPtr p_sockets, int p_virtual_port) const {
	if (!fn_net_create_listen_socket_p2p || !p_sockets) {
		return STEAM_LISTEN_SOCKET_INVALID;
	}
	return fn_net_create_listen_socket_p2p(p_sockets, p_virtual_port, 0, nullptr);
}

SteamHSteamNetConnection SteamAPILoader::net_connect_p2p(ISteamNetworkingSocketsPtr p_sockets, uint64_t p_steam_id, int p_virtual_port) const {
	if (!fn_net_connect_p2p || !p_sockets) {
		return STEAM_NET_CONNECTION_INVALID;
	}
	// Equivalent to SteamNetworkingIdentity::SetSteamID64().
	SteamNetworkingIdentityData identity;
	memset(&identity, 0, sizeof(identity));
	identity.m_eType = STEAM_NETWORKING_IDENTITY_TYPE_STEAM_ID;
	identity.m_cbSize = (int)sizeof(uint64_t);
	identity.m_steamID64 = p_steam_id;
	return fn_net_connect_p2p(p_sockets, &identity, p_virtual_port, 0, nullptr);
}

int SteamAPILoader::net_accept_connection(ISteamNetworkingSocketsPtr p_sockets, SteamHSteamNetConnection p_connection) const {
	if (!fn_net_accept_connection || !p_sockets) {
		return STEAM_RESULT_FAIL;
	}
	return fn_net_accept_connection(p_sockets, p_connection);
}

bool SteamAPILoader::net_close_connection(ISteamNetworkingSocketsPtr p_sockets, SteamHSteamNetConnection p_connection, int p_reason, const char *p_debug, bool p_enable_linger) const {
	if (!fn_net_close_connection || !p_sockets) {
		return false;
	}
	return fn_net_close_connection(p_sockets, p_connection, p_reason, p_debug, p_enable_linger);
}

bool SteamAPILoader::net_close_listen_socket(ISteamNetworkingSocketsPtr p_sockets, SteamHSteamListenSocket p_socket) const {
	if (!fn_net_close_listen_socket || !p_sockets) {
		return false;
	}
	return fn_net_close_listen_socket(p_sockets, p_socket);
}

int SteamAPILoader::net_send_message_to_connection(ISteamNetworkingSocketsPtr p_sockets, SteamHSteamNetConnection p_connection, const void *p_data, uint32_t p_size, int p_send_flags) const {
	if (!fn_net_send_message_to_connection || !p_sockets) {
		return STEAM_RESULT_FAIL;
	}
	return fn_net_send_message_to_connection(p_sockets, p_connection, p_data, p_size, p_send_flags, nullptr);
}

int SteamAPILoader::net_flush_messages_on_connection(ISteamNetworkingSocketsPtr p_sockets, SteamHSteamNetConnection p_connection) const {
	if (!fn_net_flush_messages_on_connection || !p_sockets) {
		return STEAM_RESULT_FAIL;
	}
	return fn_net_flush_messages_on_connection(p_sockets, p_connection);
}

bool SteamAPILoader::net_get_connection_info(ISteamNetworkingSocketsPtr p_sockets, SteamHSteamNetConnection p_connection, SteamNetConnectionInfo &r_info) const {
	memset(&r_info, 0, sizeof(r_info));
	if (!fn_net_get_connection_info || !p_sockets) {
		return false;
	}
	return fn_net_get_connection_info(p_sockets, p_connection, &r_info);
}

bool SteamAPILoader::net_get_connection_real_time_status(ISteamNetworkingSocketsPtr p_sockets, SteamHSteamNetConnection p_connection, SteamNetConnectionRealTimeStatus &r_status) const {
	memset(&r_status, 0, sizeof(r_status));
	if (!fn_net_get_connection_real_time_status || !p_sockets) {
		return false;
	}
	return fn_net_get_connection_real_time_status(p_sockets, p_connection, &r_status, 0, nullptr) == STEAM_RESULT_OK;
}

SteamHSteamNetPollGroup SteamAPILoader::net_create_poll_group(ISteamNetworkingSocketsPtr p_sockets) const {
	if (!fn_net_create_poll_group || !p_sockets) {
		return STEAM_NET_POLL_GROUP_INVALID;
	}
	return fn_net_create_poll_group(p_sockets);
}

bool SteamAPILoader::net_destroy_poll_group(ISteamNetworkingSocketsPtr p_sockets, SteamHSteamNetPollGroup p_poll_group) const {
	if (!fn_net_destroy_poll_group || !p_sockets) {
		return false;
	}
	return fn_net_destroy_poll_group(p_sockets, p_poll_group);
}

bool SteamAPILoader::net_set_connection_poll_group(ISteamNetworkingSocketsPtr p_sockets, SteamHSteamNetConnection p_connection, SteamHSteamNetPollGroup p_poll_group) const {
	if (!fn_net_set_connection_poll_group || !p_sockets) {
		return false;
	}
	return fn_net_set_connection_poll_group(p_sockets, p_connection, p_poll_group);
}

int SteamAPILoader::net_receive_messages_on_poll_group(ISteamNetworkingSocketsPtr p_sockets, SteamHSteamNetPollGroup p_poll_group, SteamNetworkingMessage **r_messages, int p_max_messages) const {
	if (!fn_net_receive_messages_on_poll_group || !p_sockets) {
		return 0;
	}
	return fn_net_receive_messages_on_poll_group(p_sockets, p_poll_group, r_messages, p_max_messages);
}

void SteamAPILoader::net_release_message(SteamNetworkingMessage *p_message) const {
	if (fn_net_message_release && p_message) {
		fn_net_message_release(p_message);
	}
}

// Steam Matchmaking (lobbies).

namespace {

String _mm_c_string(const char *p_value) {
	return p_value ? String::utf8(p_value) : String();
}

} // namespace

SteamAPILoader::ISteamMatchmakingPtr SteamAPILoader::get_steam_matchmaking() const {
	return fn_steam_matchmaking ? fn_steam_matchmaking() : nullptr;
}

SteamAPICallHandle_t SteamAPILoader::mm_request_lobby_list(ISteamMatchmakingPtr p_matchmaking) const {
	if (!fn_mm_request_lobby_list || !p_matchmaking) {
		return STEAM_API_CALL_INVALID;
	}
	return fn_mm_request_lobby_list(p_matchmaking);
}

void SteamAPILoader::mm_add_request_lobby_list_string_filter(ISteamMatchmakingPtr p_matchmaking, const char *p_key, const char *p_value, int p_comparison) const {
	if (fn_mm_add_string_filter && p_matchmaking) {
		fn_mm_add_string_filter(p_matchmaking, p_key, p_value, p_comparison);
	}
}

void SteamAPILoader::mm_add_request_lobby_list_numerical_filter(ISteamMatchmakingPtr p_matchmaking, const char *p_key, int p_value, int p_comparison) const {
	if (fn_mm_add_numerical_filter && p_matchmaking) {
		fn_mm_add_numerical_filter(p_matchmaking, p_key, p_value, p_comparison);
	}
}

void SteamAPILoader::mm_add_request_lobby_list_filter_slots_available(ISteamMatchmakingPtr p_matchmaking, int p_slots) const {
	if (fn_mm_add_slots_available_filter && p_matchmaking) {
		fn_mm_add_slots_available_filter(p_matchmaking, p_slots);
	}
}

void SteamAPILoader::mm_add_request_lobby_list_distance_filter(ISteamMatchmakingPtr p_matchmaking, int p_distance) const {
	if (fn_mm_add_distance_filter && p_matchmaking) {
		fn_mm_add_distance_filter(p_matchmaking, p_distance);
	}
}

void SteamAPILoader::mm_add_request_lobby_list_result_count_filter(ISteamMatchmakingPtr p_matchmaking, int p_max_results) const {
	if (fn_mm_add_result_count_filter && p_matchmaking) {
		fn_mm_add_result_count_filter(p_matchmaking, p_max_results);
	}
}

uint64_t SteamAPILoader::mm_get_lobby_by_index(ISteamMatchmakingPtr p_matchmaking, int p_index) const {
	if (!fn_mm_get_lobby_by_index || !p_matchmaking) {
		return 0;
	}
	return fn_mm_get_lobby_by_index(p_matchmaking, p_index);
}

SteamAPICallHandle_t SteamAPILoader::mm_create_lobby(ISteamMatchmakingPtr p_matchmaking, int p_lobby_type, int p_max_members) const {
	if (!fn_mm_create_lobby || !p_matchmaking) {
		return STEAM_API_CALL_INVALID;
	}
	return fn_mm_create_lobby(p_matchmaking, p_lobby_type, p_max_members);
}

SteamAPICallHandle_t SteamAPILoader::mm_join_lobby(ISteamMatchmakingPtr p_matchmaking, uint64_t p_lobby_id) const {
	if (!fn_mm_join_lobby || !p_matchmaking) {
		return STEAM_API_CALL_INVALID;
	}
	return fn_mm_join_lobby(p_matchmaking, p_lobby_id);
}

void SteamAPILoader::mm_leave_lobby(ISteamMatchmakingPtr p_matchmaking, uint64_t p_lobby_id) const {
	if (fn_mm_leave_lobby && p_matchmaking) {
		fn_mm_leave_lobby(p_matchmaking, p_lobby_id);
	}
}

bool SteamAPILoader::mm_invite_user_to_lobby(ISteamMatchmakingPtr p_matchmaking, uint64_t p_lobby_id, uint64_t p_steam_id) const {
	if (!fn_mm_invite_user_to_lobby || !p_matchmaking) {
		return false;
	}
	return fn_mm_invite_user_to_lobby(p_matchmaking, p_lobby_id, p_steam_id);
}

int SteamAPILoader::mm_get_num_lobby_members(ISteamMatchmakingPtr p_matchmaking, uint64_t p_lobby_id) const {
	if (!fn_mm_get_num_lobby_members || !p_matchmaking) {
		return 0;
	}
	return fn_mm_get_num_lobby_members(p_matchmaking, p_lobby_id);
}

uint64_t SteamAPILoader::mm_get_lobby_member_by_index(ISteamMatchmakingPtr p_matchmaking, uint64_t p_lobby_id, int p_index) const {
	if (!fn_mm_get_lobby_member_by_index || !p_matchmaking) {
		return 0;
	}
	return fn_mm_get_lobby_member_by_index(p_matchmaking, p_lobby_id, p_index);
}

String SteamAPILoader::mm_get_lobby_data(ISteamMatchmakingPtr p_matchmaking, uint64_t p_lobby_id, const char *p_key) const {
	if (!fn_mm_get_lobby_data || !p_matchmaking) {
		return String();
	}
	return _mm_c_string(fn_mm_get_lobby_data(p_matchmaking, p_lobby_id, p_key));
}

bool SteamAPILoader::mm_set_lobby_data(ISteamMatchmakingPtr p_matchmaking, uint64_t p_lobby_id, const char *p_key, const char *p_value) const {
	if (!fn_mm_set_lobby_data || !p_matchmaking) {
		return false;
	}
	return fn_mm_set_lobby_data(p_matchmaking, p_lobby_id, p_key, p_value);
}

Dictionary SteamAPILoader::mm_get_all_lobby_data(ISteamMatchmakingPtr p_matchmaking, uint64_t p_lobby_id) const {
	Dictionary data;
	if (!fn_mm_get_lobby_data_count || !fn_mm_get_lobby_data_by_index || !p_matchmaking) {
		return data;
	}
	// Lobby values are limited to 8 KB (k_cubChatMetadataMax).
	const int kValueBufferSize = 8192 + 1;
	char key[STEAM_MAX_LOBBY_KEY_LENGTH + 1];
	Vector<char> value;
	value.resize(kValueBufferSize);
	const int count = fn_mm_get_lobby_data_count(p_matchmaking, p_lobby_id);
	for (int i = 0; i < count; i++) {
		memset(key, 0, sizeof(key));
		memset(value.ptrw(), 0, value.size());
		if (fn_mm_get_lobby_data_by_index(p_matchmaking, p_lobby_id, i, key, (int)sizeof(key), value.ptrw(), value.size())) {
			key[sizeof(key) - 1] = '\0';
			value.write[value.size() - 1] = '\0';
			data[String::utf8(key)] = String::utf8(value.ptr());
		}
	}
	return data;
}

bool SteamAPILoader::mm_delete_lobby_data(ISteamMatchmakingPtr p_matchmaking, uint64_t p_lobby_id, const char *p_key) const {
	if (!fn_mm_delete_lobby_data || !p_matchmaking) {
		return false;
	}
	return fn_mm_delete_lobby_data(p_matchmaking, p_lobby_id, p_key);
}

String SteamAPILoader::mm_get_lobby_member_data(ISteamMatchmakingPtr p_matchmaking, uint64_t p_lobby_id, uint64_t p_steam_id, const char *p_key) const {
	if (!fn_mm_get_lobby_member_data || !p_matchmaking) {
		return String();
	}
	return _mm_c_string(fn_mm_get_lobby_member_data(p_matchmaking, p_lobby_id, p_steam_id, p_key));
}

void SteamAPILoader::mm_set_lobby_member_data(ISteamMatchmakingPtr p_matchmaking, uint64_t p_lobby_id, const char *p_key, const char *p_value) const {
	if (fn_mm_set_lobby_member_data && p_matchmaking) {
		fn_mm_set_lobby_member_data(p_matchmaking, p_lobby_id, p_key, p_value);
	}
}

bool SteamAPILoader::mm_set_lobby_member_limit(ISteamMatchmakingPtr p_matchmaking, uint64_t p_lobby_id, int p_max_members) const {
	if (!fn_mm_set_lobby_member_limit || !p_matchmaking) {
		return false;
	}
	return fn_mm_set_lobby_member_limit(p_matchmaking, p_lobby_id, p_max_members);
}

int SteamAPILoader::mm_get_lobby_member_limit(ISteamMatchmakingPtr p_matchmaking, uint64_t p_lobby_id) const {
	if (!fn_mm_get_lobby_member_limit || !p_matchmaking) {
		return 0;
	}
	return fn_mm_get_lobby_member_limit(p_matchmaking, p_lobby_id);
}

bool SteamAPILoader::mm_set_lobby_type(ISteamMatchmakingPtr p_matchmaking, uint64_t p_lobby_id, int p_lobby_type) const {
	if (!fn_mm_set_lobby_type || !p_matchmaking) {
		return false;
	}
	return fn_mm_set_lobby_type(p_matchmaking, p_lobby_id, p_lobby_type);
}

bool SteamAPILoader::mm_set_lobby_joinable(ISteamMatchmakingPtr p_matchmaking, uint64_t p_lobby_id, bool p_joinable) const {
	if (!fn_mm_set_lobby_joinable || !p_matchmaking) {
		return false;
	}
	return fn_mm_set_lobby_joinable(p_matchmaking, p_lobby_id, p_joinable);
}

uint64_t SteamAPILoader::mm_get_lobby_owner(ISteamMatchmakingPtr p_matchmaking, uint64_t p_lobby_id) const {
	if (!fn_mm_get_lobby_owner || !p_matchmaking) {
		return 0;
	}
	return fn_mm_get_lobby_owner(p_matchmaking, p_lobby_id);
}

bool SteamAPILoader::mm_set_lobby_owner(ISteamMatchmakingPtr p_matchmaking, uint64_t p_lobby_id, uint64_t p_steam_id) const {
	if (!fn_mm_set_lobby_owner || !p_matchmaking) {
		return false;
	}
	return fn_mm_set_lobby_owner(p_matchmaking, p_lobby_id, p_steam_id);
}

void SteamAPILoader::friends_activate_game_overlay_invite_dialog(ISteamFriendsPtr p_friends, uint64_t p_lobby_id) const {
	if (fn_friends_activate_game_overlay_invite_dialog && p_friends) {
		fn_friends_activate_game_overlay_invite_dialog(p_friends, p_lobby_id);
	}
}

String SteamAPILoader::friends_get_friend_persona_name(ISteamFriendsPtr p_friends, uint64_t p_steam_id) const {
	if (!fn_friends_get_friend_persona_name || !p_friends) {
		return String();
	}
	return _mm_c_string(fn_friends_get_friend_persona_name(p_friends, p_steam_id));
}
