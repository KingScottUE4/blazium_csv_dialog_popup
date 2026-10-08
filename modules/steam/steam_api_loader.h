/**************************************************************************/
/*  steam_api_loader.h                                                    */
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

#include "steam_types.h"

#include "core/error/error_list.h"
#include "core/io/image.h"
#include "core/object/ref_counted.h"
#include "core/string/ustring.h"
#include "core/templates/vector.h"

class SteamAPILoader {
public:
	typedef int32_t HAuthTicket;
	typedef int32_t HSteamPipe;
	typedef void *ISteamUserPtr;
	typedef void *ISteamUserStatsPtr;
	typedef void *ISteamFriendsPtr;
	typedef void *ISteamUtilsPtr;
	typedef void *ISteamInventoryPtr;
	typedef void *ISteamUGCPtr;
	typedef void *ISteamNetworkingSocketsPtr;
	typedef void *ISteamNetworkingUtilsPtr;
	typedef void *ISteamMatchmakingPtr;

private:
	void *library_handle = nullptr;
	bool loaded = false;
	bool stats_loaded = false;
	bool inventory_loaded = false;
	bool workshop_loaded = false;
	bool networking_loaded = false;
	bool matchmaking_loaded = false;

	// Core API
	typedef int (*SteamAPI_InitFlatFn)(char *p_out_err_msg);
	typedef void (*SteamAPI_ShutdownFn)();
	typedef HSteamPipe (*SteamAPI_GetHSteamPipeFn)();
	typedef void (*SteamAPI_RunCallbacksFn)();
	typedef void (*SteamAPI_ManualDispatch_InitFn)();
	typedef void (*SteamAPI_ManualDispatch_RunFrameFn)(HSteamPipe p_pipe);
	typedef bool (*SteamAPI_ManualDispatch_GetNextCallbackFn)(HSteamPipe p_pipe, void *p_callback_msg);
	typedef void (*SteamAPI_ManualDispatch_FreeLastCallbackFn)(HSteamPipe p_pipe);
	typedef bool (*SteamAPI_ManualDispatch_GetAPICallResultFn)(HSteamPipe p_pipe, uint64_t p_async_call, void *p_callback, int p_cub_callback, int p_callback_expected, bool *p_failed);
	typedef bool (*SteamAPI_IsSteamRunningFn)();
	typedef ISteamUserPtr (*SteamAPI_SteamUserFn)();
	typedef ISteamUserStatsPtr (*SteamAPI_SteamUserStatsFn)();
	typedef ISteamFriendsPtr (*SteamAPI_SteamFriendsFn)();
	typedef ISteamUtilsPtr (*SteamAPI_SteamUtilsFn)();

	// ISteamUser flat API
	typedef HAuthTicket (*ISteamUser_GetAuthTicketForWebApiFn)(ISteamUserPtr p_self, const char *p_identity);
	typedef void (*ISteamUser_CancelAuthTicketFn)(ISteamUserPtr p_self, HAuthTicket p_ticket);
	typedef bool (*ISteamUser_BLoggedOnFn)(ISteamUserPtr p_self);
	typedef uint64_t (*ISteamUser_GetSteamIDFn)(ISteamUserPtr p_self);

	// ISteamUserStats flat API
	typedef bool (*ISteamUserStats_RequestCurrentStatsFn)(ISteamUserStatsPtr p_self);
	typedef bool (*ISteamUserStats_GetAchievementFn)(ISteamUserStatsPtr p_self, const char *p_name, bool *p_achieved);
	typedef bool (*ISteamUserStats_SetAchievementFn)(ISteamUserStatsPtr p_self, const char *p_name);
	typedef bool (*ISteamUserStats_ClearAchievementFn)(ISteamUserStatsPtr p_self, const char *p_name);
	typedef bool (*ISteamUserStats_StoreStatsFn)(ISteamUserStatsPtr p_self);
	typedef bool (*ISteamUserStats_GetAchievementAndUnlockTimeFn)(ISteamUserStatsPtr p_self, const char *p_name, bool *p_achieved, uint32_t *p_unlock_time);
	typedef int (*ISteamUserStats_GetAchievementIconFn)(ISteamUserStatsPtr p_self, const char *p_name);
	typedef const char *(*ISteamUserStats_GetAchievementDisplayAttributeFn)(ISteamUserStatsPtr p_self, const char *p_name, const char *p_key);
	typedef uint32_t (*ISteamUserStats_GetNumAchievementsFn)(ISteamUserStatsPtr p_self);
	typedef const char *(*ISteamUserStats_GetAchievementNameFn)(ISteamUserStatsPtr p_self, uint32_t p_index);
	typedef bool (*ISteamUserStats_GetStatInt32Fn)(ISteamUserStatsPtr p_self, const char *p_name, int32_t *p_data);
	typedef bool (*ISteamUserStats_GetStatFloatFn)(ISteamUserStatsPtr p_self, const char *p_name, float *p_data);
	typedef bool (*ISteamUserStats_SetStatInt32Fn)(ISteamUserStatsPtr p_self, const char *p_name, int32_t p_data);
	typedef bool (*ISteamUserStats_SetStatFloatFn)(ISteamUserStatsPtr p_self, const char *p_name, float p_data);
	typedef bool (*ISteamUserStats_UpdateAvgRateStatFn)(ISteamUserStatsPtr p_self, const char *p_name, float p_count_this_session, double p_session_length);

	// ISteamFriends flat API
	typedef const char *(*ISteamFriends_GetPersonaNameFn)(ISteamFriendsPtr p_self);
	typedef int (*ISteamFriends_GetSmallFriendAvatarFn)(ISteamFriendsPtr p_self, uint64_t p_steam_id);
	typedef int (*ISteamFriends_GetMediumFriendAvatarFn)(ISteamFriendsPtr p_self, uint64_t p_steam_id);
	typedef int (*ISteamFriends_GetLargeFriendAvatarFn)(ISteamFriendsPtr p_self, uint64_t p_steam_id);

	// ISteamUtils flat API
	typedef bool (*ISteamUtils_GetImageSizeFn)(ISteamUtilsPtr p_self, int p_image, uint32_t *p_width, uint32_t *p_height);
	typedef bool (*ISteamUtils_GetImageRGBAFn)(ISteamUtilsPtr p_self, int p_image, uint8_t *p_dest, int p_dest_size);

	// ISteamInventory flat API
	typedef ISteamInventoryPtr (*SteamAPI_SteamInventoryFn)();
	typedef int (*ISteamInventory_GetResultStatusFn)(ISteamInventoryPtr p_self, SteamInventoryResult_t p_result);
	typedef bool (*ISteamInventory_GetResultItemsFn)(ISteamInventoryPtr p_self, SteamInventoryResult_t p_result, SteamItemDetails *p_out_items, uint32_t *p_out_count);
	typedef bool (*ISteamInventory_GetResultItemPropertyFn)(ISteamInventoryPtr p_self, SteamInventoryResult_t p_result, uint32_t p_item_index, const char *p_property_name, char *p_value_buffer, uint32_t *p_value_buffer_size);
	typedef uint32_t (*ISteamInventory_GetResultTimestampFn)(ISteamInventoryPtr p_self, SteamInventoryResult_t p_result);
	typedef bool (*ISteamInventory_CheckResultSteamIDFn)(ISteamInventoryPtr p_self, SteamInventoryResult_t p_result, uint64_t p_steam_id);
	typedef void (*ISteamInventory_DestroyResultFn)(ISteamInventoryPtr p_self, SteamInventoryResult_t p_result);
	typedef bool (*ISteamInventory_GetAllItemsFn)(ISteamInventoryPtr p_self, SteamInventoryResult_t *p_result);
	typedef bool (*ISteamInventory_GetItemsByIDFn)(ISteamInventoryPtr p_self, SteamInventoryResult_t *p_result, const SteamItemInstanceID_t *p_instance_ids, uint32_t p_count);
	typedef bool (*ISteamInventory_SerializeResultFn)(ISteamInventoryPtr p_self, SteamInventoryResult_t p_result, void *p_out_buffer, uint32_t *p_out_buffer_size);
	typedef bool (*ISteamInventory_DeserializeResultFn)(ISteamInventoryPtr p_self, SteamInventoryResult_t *p_out_result, const void *p_buffer, uint32_t p_buffer_size, bool p_reserved);
	typedef bool (*ISteamInventory_GrantPromoItemsFn)(ISteamInventoryPtr p_self, SteamInventoryResult_t *p_result);
	typedef bool (*ISteamInventory_AddPromoItemFn)(ISteamInventoryPtr p_self, SteamInventoryResult_t *p_result, SteamItemDef_t p_item_def);
	typedef bool (*ISteamInventory_AddPromoItemsFn)(ISteamInventoryPtr p_self, SteamInventoryResult_t *p_result, const SteamItemDef_t *p_item_defs, uint32_t p_count);
	typedef bool (*ISteamInventory_ConsumeItemFn)(ISteamInventoryPtr p_self, SteamInventoryResult_t *p_result, SteamItemInstanceID_t p_item, uint32_t p_quantity);
	typedef bool (*ISteamInventory_ExchangeItemsFn)(ISteamInventoryPtr p_self, SteamInventoryResult_t *p_result, const SteamItemDef_t *p_generate_defs, const uint32_t *p_generate_quantities, uint32_t p_generate_count, const SteamItemInstanceID_t *p_destroy_ids, const uint32_t *p_destroy_quantities, uint32_t p_destroy_count);
	typedef bool (*ISteamInventory_TransferItemQuantityFn)(ISteamInventoryPtr p_self, SteamInventoryResult_t *p_result, SteamItemInstanceID_t p_source, uint32_t p_quantity, SteamItemInstanceID_t p_dest);
	typedef bool (*ISteamInventory_LoadItemDefinitionsFn)(ISteamInventoryPtr p_self);
	typedef bool (*ISteamInventory_GetItemDefinitionIDsFn)(ISteamInventoryPtr p_self, SteamItemDef_t *p_item_def_ids, uint32_t *p_count);
	typedef bool (*ISteamInventory_GetItemDefinitionPropertyFn)(ISteamInventoryPtr p_self, SteamItemDef_t p_definition, const char *p_property_name, char *p_value_buffer, uint32_t *p_value_buffer_size);
	typedef SteamInventoryUpdateHandle_t (*ISteamInventory_StartUpdatePropertiesFn)(ISteamInventoryPtr p_self);
	typedef bool (*ISteamInventory_RemovePropertyFn)(ISteamInventoryPtr p_self, SteamInventoryUpdateHandle_t p_handle, SteamItemInstanceID_t p_item_id, const char *p_property_name);
	typedef bool (*ISteamInventory_SetPropertyStringFn)(ISteamInventoryPtr p_self, SteamInventoryUpdateHandle_t p_handle, SteamItemInstanceID_t p_item_id, const char *p_property_name, const char *p_value);
	typedef bool (*ISteamInventory_SetPropertyBoolFn)(ISteamInventoryPtr p_self, SteamInventoryUpdateHandle_t p_handle, SteamItemInstanceID_t p_item_id, const char *p_property_name, bool p_value);
	typedef bool (*ISteamInventory_SetPropertyInt64Fn)(ISteamInventoryPtr p_self, SteamInventoryUpdateHandle_t p_handle, SteamItemInstanceID_t p_item_id, const char *p_property_name, int64_t p_value);
	typedef bool (*ISteamInventory_SetPropertyFloatFn)(ISteamInventoryPtr p_self, SteamInventoryUpdateHandle_t p_handle, SteamItemInstanceID_t p_item_id, const char *p_property_name, float p_value);
	typedef bool (*ISteamInventory_SubmitUpdatePropertiesFn)(ISteamInventoryPtr p_self, SteamInventoryUpdateHandle_t p_handle, SteamInventoryResult_t *p_result);
	typedef bool (*ISteamInventory_InspectItemFn)(ISteamInventoryPtr p_self, SteamInventoryResult_t *p_result, const char *p_item_token);
	typedef bool (*ISteamInventory_TriggerItemDropFn)(ISteamInventoryPtr p_self, SteamInventoryResult_t *p_result, SteamItemDef_t p_drop_list_definition);
	typedef bool (*ISteamInventory_GetEligiblePromoItemDefinitionIDsFn)(ISteamInventoryPtr p_self, uint64_t p_steam_id, SteamItemDef_t *p_item_def_ids, uint32_t *p_count);

	// ISteamUGC flat API (Steam Workshop). Enum parameters are passed as int.
	typedef ISteamUGCPtr (*SteamAPI_SteamUGCFn)();
	typedef SteamUGCQueryHandle_t (*ISteamUGC_CreateQueryUserUGCRequestFn)(ISteamUGCPtr p_self, uint32_t p_account_id, int p_list_type, int p_matching_type, int p_sort_order, uint32_t p_creator_app_id, uint32_t p_consumer_app_id, uint32_t p_page);
	typedef SteamUGCQueryHandle_t (*ISteamUGC_CreateQueryAllUGCRequestPageFn)(ISteamUGCPtr p_self, int p_query_type, int p_matching_type, uint32_t p_creator_app_id, uint32_t p_consumer_app_id, uint32_t p_page);
	typedef SteamUGCQueryHandle_t (*ISteamUGC_CreateQueryAllUGCRequestCursorFn)(ISteamUGCPtr p_self, int p_query_type, int p_matching_type, uint32_t p_creator_app_id, uint32_t p_consumer_app_id, const char *p_cursor);
	typedef SteamUGCQueryHandle_t (*ISteamUGC_CreateQueryUGCDetailsRequestFn)(ISteamUGCPtr p_self, SteamPublishedFileId_t *p_file_ids, uint32_t p_count);
	typedef SteamAPICallHandle_t (*ISteamUGC_SendQueryUGCRequestFn)(ISteamUGCPtr p_self, SteamUGCQueryHandle_t p_handle);
	typedef bool (*ISteamUGC_GetQueryUGCResultFn)(ISteamUGCPtr p_self, SteamUGCQueryHandle_t p_handle, uint32_t p_index, SteamUGCDetails *p_details);
	typedef bool (*ISteamUGC_GetQueryUGCStringFn)(ISteamUGCPtr p_self, SteamUGCQueryHandle_t p_handle, uint32_t p_index, char *p_buffer, uint32_t p_buffer_size);
	typedef bool (*ISteamUGC_GetQueryUGCChildrenFn)(ISteamUGCPtr p_self, SteamUGCQueryHandle_t p_handle, uint32_t p_index, SteamPublishedFileId_t *p_file_ids, uint32_t p_max_entries);
	typedef bool (*ISteamUGC_GetQueryUGCStatisticFn)(ISteamUGCPtr p_self, SteamUGCQueryHandle_t p_handle, uint32_t p_index, int p_stat_type, uint64_t *p_value);
	typedef uint32_t (*ISteamUGC_GetQueryUGCCountFn)(ISteamUGCPtr p_self, SteamUGCQueryHandle_t p_handle, uint32_t p_index);
	typedef bool (*ISteamUGC_GetQueryUGCAdditionalPreviewFn)(ISteamUGCPtr p_self, SteamUGCQueryHandle_t p_handle, uint32_t p_index, uint32_t p_preview_index, char *p_url_or_video_id, uint32_t p_url_size, char *p_original_file_name, uint32_t p_original_file_name_size, int *p_preview_type);
	typedef bool (*ISteamUGC_GetQueryUGCKeyValueTagFn)(ISteamUGCPtr p_self, SteamUGCQueryHandle_t p_handle, uint32_t p_index, uint32_t p_tag_index, char *p_key, uint32_t p_key_size, char *p_value, uint32_t p_value_size);
	typedef bool (*ISteamUGC_QueryHandleFn)(ISteamUGCPtr p_self, SteamUGCQueryHandle_t p_handle);
	typedef bool (*ISteamUGC_QueryHandleStringFn)(ISteamUGCPtr p_self, SteamUGCQueryHandle_t p_handle, const char *p_value);
	typedef bool (*ISteamUGC_QueryHandleKeyValueFn)(ISteamUGCPtr p_self, SteamUGCQueryHandle_t p_handle, const char *p_key, const char *p_value);
	typedef bool (*ISteamUGC_QueryHandleBoolFn)(ISteamUGCPtr p_self, SteamUGCQueryHandle_t p_handle, bool p_value);
	typedef bool (*ISteamUGC_QueryHandleUIntFn)(ISteamUGCPtr p_self, SteamUGCQueryHandle_t p_handle, uint32_t p_value);
	typedef SteamAPICallHandle_t (*ISteamUGC_CreateItemFn)(ISteamUGCPtr p_self, uint32_t p_consumer_app_id, int p_file_type);
	typedef SteamUGCUpdateHandle_t (*ISteamUGC_StartItemUpdateFn)(ISteamUGCPtr p_self, uint32_t p_consumer_app_id, SteamPublishedFileId_t p_file_id);
	typedef bool (*ISteamUGC_UpdateHandleStringFn)(ISteamUGCPtr p_self, SteamUGCUpdateHandle_t p_handle, const char *p_value);
	typedef bool (*ISteamUGC_UpdateHandleKeyValueFn)(ISteamUGCPtr p_self, SteamUGCUpdateHandle_t p_handle, const char *p_key, const char *p_value);
	typedef bool (*ISteamUGC_UpdateHandleIntFn)(ISteamUGCPtr p_self, SteamUGCUpdateHandle_t p_handle, int p_value);
	typedef bool (*ISteamUGC_UpdateHandleFn)(ISteamUGCPtr p_self, SteamUGCUpdateHandle_t p_handle);
	typedef bool (*ISteamUGC_SetItemTagsFn)(ISteamUGCPtr p_self, SteamUGCUpdateHandle_t p_handle, const SteamParamStringArray *p_tags, bool p_allow_admin_tags);
	typedef bool (*ISteamUGC_AddItemPreviewFileFn)(ISteamUGCPtr p_self, SteamUGCUpdateHandle_t p_handle, const char *p_preview_file, int p_type);
	typedef SteamAPICallHandle_t (*ISteamUGC_SubmitItemUpdateFn)(ISteamUGCPtr p_self, SteamUGCUpdateHandle_t p_handle, const char *p_change_note);
	typedef int (*ISteamUGC_GetItemUpdateProgressFn)(ISteamUGCPtr p_self, SteamUGCUpdateHandle_t p_handle, uint64_t *p_bytes_processed, uint64_t *p_bytes_total);
	typedef SteamAPICallHandle_t (*ISteamUGC_FileIdCallFn)(ISteamUGCPtr p_self, SteamPublishedFileId_t p_file_id);
	typedef SteamAPICallHandle_t (*ISteamUGC_SetUserItemVoteFn)(ISteamUGCPtr p_self, SteamPublishedFileId_t p_file_id, bool p_vote_up);
	typedef SteamAPICallHandle_t (*ISteamUGC_AppFileIdCallFn)(ISteamUGCPtr p_self, uint32_t p_app_id, SteamPublishedFileId_t p_file_id);
	typedef uint32_t (*ISteamUGC_GetNumSubscribedItemsFn)(ISteamUGCPtr p_self, bool p_include_locally_disabled);
	typedef uint32_t (*ISteamUGC_GetSubscribedItemsFn)(ISteamUGCPtr p_self, SteamPublishedFileId_t *p_file_ids, uint32_t p_max_entries, bool p_include_locally_disabled);
	typedef uint32_t (*ISteamUGC_GetItemStateFn)(ISteamUGCPtr p_self, SteamPublishedFileId_t p_file_id);
	typedef bool (*ISteamUGC_GetItemInstallInfoFn)(ISteamUGCPtr p_self, SteamPublishedFileId_t p_file_id, uint64_t *p_size_on_disk, char *p_folder, uint32_t p_folder_size, uint32_t *p_timestamp);
	typedef bool (*ISteamUGC_GetItemDownloadInfoFn)(ISteamUGCPtr p_self, SteamPublishedFileId_t p_file_id, uint64_t *p_bytes_downloaded, uint64_t *p_bytes_total);
	typedef bool (*ISteamUGC_DownloadItemFn)(ISteamUGCPtr p_self, SteamPublishedFileId_t p_file_id, bool p_high_priority);
	typedef void (*ISteamUGC_SuspendDownloadsFn)(ISteamUGCPtr p_self, bool p_suspend);
	typedef SteamAPICallHandle_t (*ISteamUGC_PlaytimeTrackingFn)(ISteamUGCPtr p_self, SteamPublishedFileId_t *p_file_ids, uint32_t p_count);
	typedef SteamAPICallHandle_t (*ISteamUGC_StopPlaytimeTrackingForAllItemsFn)(ISteamUGCPtr p_self);
	typedef bool (*ISteamUGC_ShowWorkshopEULAFn)(ISteamUGCPtr p_self);
	typedef uint32_t (*ISteamUtils_GetAppIDFn)(ISteamUtilsPtr p_self);

	// ISteamNetworkingSockets / ISteamNetworkingUtils flat API. Configuration
	// options are never passed, so they are typed as opaque pointers.
	typedef ISteamNetworkingSocketsPtr (*SteamAPI_SteamNetworkingSocketsFn)();
	typedef ISteamNetworkingUtilsPtr (*SteamAPI_SteamNetworkingUtilsFn)();
	typedef SteamHSteamListenSocket (*ISteamNetworkingSockets_CreateListenSocketP2PFn)(ISteamNetworkingSocketsPtr p_self, int p_virtual_port, int p_num_options, const void *p_options);
	typedef SteamHSteamNetConnection (*ISteamNetworkingSockets_ConnectP2PFn)(ISteamNetworkingSocketsPtr p_self, const SteamNetworkingIdentityData *p_identity, int p_virtual_port, int p_num_options, const void *p_options);
	typedef int (*ISteamNetworkingSockets_AcceptConnectionFn)(ISteamNetworkingSocketsPtr p_self, SteamHSteamNetConnection p_connection);
	typedef bool (*ISteamNetworkingSockets_CloseConnectionFn)(ISteamNetworkingSocketsPtr p_self, SteamHSteamNetConnection p_connection, int p_reason, const char *p_debug, bool p_enable_linger);
	typedef bool (*ISteamNetworkingSockets_CloseListenSocketFn)(ISteamNetworkingSocketsPtr p_self, SteamHSteamListenSocket p_socket);
	typedef int (*ISteamNetworkingSockets_SendMessageToConnectionFn)(ISteamNetworkingSocketsPtr p_self, SteamHSteamNetConnection p_connection, const void *p_data, uint32_t p_size, int p_send_flags, int64_t *r_message_number);
	typedef int (*ISteamNetworkingSockets_FlushMessagesOnConnectionFn)(ISteamNetworkingSocketsPtr p_self, SteamHSteamNetConnection p_connection);
	typedef bool (*ISteamNetworkingSockets_GetConnectionInfoFn)(ISteamNetworkingSocketsPtr p_self, SteamHSteamNetConnection p_connection, SteamNetConnectionInfo *r_info);
	typedef int (*ISteamNetworkingSockets_GetConnectionRealTimeStatusFn)(ISteamNetworkingSocketsPtr p_self, SteamHSteamNetConnection p_connection, SteamNetConnectionRealTimeStatus *r_status, int p_num_lanes, void *r_lanes);
	typedef SteamHSteamNetPollGroup (*ISteamNetworkingSockets_CreatePollGroupFn)(ISteamNetworkingSocketsPtr p_self);
	typedef bool (*ISteamNetworkingSockets_DestroyPollGroupFn)(ISteamNetworkingSocketsPtr p_self, SteamHSteamNetPollGroup p_poll_group);
	typedef bool (*ISteamNetworkingSockets_SetConnectionPollGroupFn)(ISteamNetworkingSocketsPtr p_self, SteamHSteamNetConnection p_connection, SteamHSteamNetPollGroup p_poll_group);
	typedef int (*ISteamNetworkingSockets_ReceiveMessagesOnPollGroupFn)(ISteamNetworkingSocketsPtr p_self, SteamHSteamNetPollGroup p_poll_group, SteamNetworkingMessage **r_messages, int p_max_messages);
	typedef void (*SteamNetworkingMessage_ReleaseFn)(SteamNetworkingMessage *p_message);
	typedef void (*ISteamNetworkingUtils_InitRelayNetworkAccessFn)(ISteamNetworkingUtilsPtr p_self);

	// ISteamMatchmaking flat API (lobbies). Enum parameters are passed as int.
	typedef ISteamMatchmakingPtr (*SteamAPI_SteamMatchmakingFn)();
	typedef SteamAPICallHandle_t (*ISteamMatchmaking_RequestLobbyListFn)(ISteamMatchmakingPtr p_self);
	typedef void (*ISteamMatchmaking_AddStringFilterFn)(ISteamMatchmakingPtr p_self, const char *p_key, const char *p_value, int p_comparison);
	typedef void (*ISteamMatchmaking_AddNumericalFilterFn)(ISteamMatchmakingPtr p_self, const char *p_key, int p_value, int p_comparison);
	typedef void (*ISteamMatchmaking_AddIntFilterFn)(ISteamMatchmakingPtr p_self, int p_value);
	typedef uint64_t (*ISteamMatchmaking_GetLobbyByIndexFn)(ISteamMatchmakingPtr p_self, int p_index);
	typedef SteamAPICallHandle_t (*ISteamMatchmaking_CreateLobbyFn)(ISteamMatchmakingPtr p_self, int p_lobby_type, int p_max_members);
	typedef SteamAPICallHandle_t (*ISteamMatchmaking_JoinLobbyFn)(ISteamMatchmakingPtr p_self, uint64_t p_lobby_id);
	typedef void (*ISteamMatchmaking_LeaveLobbyFn)(ISteamMatchmakingPtr p_self, uint64_t p_lobby_id);
	typedef bool (*ISteamMatchmaking_LobbyUserBoolFn)(ISteamMatchmakingPtr p_self, uint64_t p_lobby_id, uint64_t p_steam_id);
	typedef int (*ISteamMatchmaking_LobbyIntFn)(ISteamMatchmakingPtr p_self, uint64_t p_lobby_id);
	typedef uint64_t (*ISteamMatchmaking_GetLobbyMemberByIndexFn)(ISteamMatchmakingPtr p_self, uint64_t p_lobby_id, int p_index);
	typedef const char *(*ISteamMatchmaking_GetLobbyDataFn)(ISteamMatchmakingPtr p_self, uint64_t p_lobby_id, const char *p_key);
	typedef bool (*ISteamMatchmaking_SetLobbyDataFn)(ISteamMatchmakingPtr p_self, uint64_t p_lobby_id, const char *p_key, const char *p_value);
	typedef bool (*ISteamMatchmaking_GetLobbyDataByIndexFn)(ISteamMatchmakingPtr p_self, uint64_t p_lobby_id, int p_index, char *r_key, int p_key_size, char *r_value, int p_value_size);
	typedef bool (*ISteamMatchmaking_DeleteLobbyDataFn)(ISteamMatchmakingPtr p_self, uint64_t p_lobby_id, const char *p_key);
	typedef const char *(*ISteamMatchmaking_GetLobbyMemberDataFn)(ISteamMatchmakingPtr p_self, uint64_t p_lobby_id, uint64_t p_steam_id, const char *p_key);
	typedef void (*ISteamMatchmaking_SetLobbyMemberDataFn)(ISteamMatchmakingPtr p_self, uint64_t p_lobby_id, const char *p_key, const char *p_value);
	typedef bool (*ISteamMatchmaking_LobbyIntSetterFn)(ISteamMatchmakingPtr p_self, uint64_t p_lobby_id, int p_value);
	typedef bool (*ISteamMatchmaking_SetLobbyJoinableFn)(ISteamMatchmakingPtr p_self, uint64_t p_lobby_id, bool p_joinable);
	typedef uint64_t (*ISteamMatchmaking_GetLobbyOwnerFn)(ISteamMatchmakingPtr p_self, uint64_t p_lobby_id);
	typedef void (*ISteamFriends_ActivateGameOverlayInviteDialogFn)(ISteamFriendsPtr p_self, uint64_t p_lobby_id);
	typedef const char *(*ISteamFriends_GetFriendPersonaNameFn)(ISteamFriendsPtr p_self, uint64_t p_steam_id);

	SteamAPI_InitFlatFn fn_init_flat = nullptr;
	SteamAPI_ShutdownFn fn_shutdown = nullptr;
	SteamAPI_GetHSteamPipeFn fn_get_h_steam_pipe = nullptr;
	SteamAPI_RunCallbacksFn fn_run_callbacks = nullptr;
	SteamAPI_ManualDispatch_InitFn fn_manual_dispatch_init = nullptr;
	SteamAPI_ManualDispatch_RunFrameFn fn_manual_dispatch_run_frame = nullptr;
	SteamAPI_ManualDispatch_GetNextCallbackFn fn_manual_dispatch_get_next_callback = nullptr;
	SteamAPI_ManualDispatch_FreeLastCallbackFn fn_manual_dispatch_free_last_callback = nullptr;
	SteamAPI_ManualDispatch_GetAPICallResultFn fn_manual_dispatch_get_api_call_result = nullptr;
	SteamAPI_IsSteamRunningFn fn_is_steam_running = nullptr;
	SteamAPI_SteamUserFn fn_steam_user = nullptr;
	ISteamUser_GetAuthTicketForWebApiFn fn_get_auth_ticket_for_web_api = nullptr;
	ISteamUser_CancelAuthTicketFn fn_cancel_auth_ticket = nullptr;
	ISteamUser_BLoggedOnFn fn_b_logged_on = nullptr;
	ISteamUser_GetSteamIDFn fn_get_steam_id = nullptr;

	SteamAPI_SteamUserStatsFn fn_steam_user_stats = nullptr;
	ISteamUserStats_RequestCurrentStatsFn fn_request_current_stats = nullptr;
	ISteamUserStats_GetAchievementFn fn_get_achievement = nullptr;
	ISteamUserStats_SetAchievementFn fn_set_achievement = nullptr;
	ISteamUserStats_ClearAchievementFn fn_clear_achievement = nullptr;
	ISteamUserStats_StoreStatsFn fn_store_stats = nullptr;
	ISteamUserStats_GetAchievementAndUnlockTimeFn fn_get_achievement_and_unlock_time = nullptr;
	ISteamUserStats_GetAchievementIconFn fn_get_achievement_icon = nullptr;
	ISteamUserStats_GetAchievementDisplayAttributeFn fn_get_achievement_display_attribute = nullptr;
	ISteamUserStats_GetNumAchievementsFn fn_get_num_achievements = nullptr;
	ISteamUserStats_GetAchievementNameFn fn_get_achievement_name = nullptr;
	ISteamUserStats_GetStatInt32Fn fn_get_stat_int32 = nullptr;
	ISteamUserStats_GetStatFloatFn fn_get_stat_float = nullptr;
	ISteamUserStats_SetStatInt32Fn fn_set_stat_int32 = nullptr;
	ISteamUserStats_SetStatFloatFn fn_set_stat_float = nullptr;
	ISteamUserStats_UpdateAvgRateStatFn fn_update_avg_rate_stat = nullptr;

	SteamAPI_SteamFriendsFn fn_steam_friends = nullptr;
	ISteamFriends_GetPersonaNameFn fn_get_persona_name = nullptr;
	ISteamFriends_GetSmallFriendAvatarFn fn_get_small_friend_avatar = nullptr;
	ISteamFriends_GetMediumFriendAvatarFn fn_get_medium_friend_avatar = nullptr;
	ISteamFriends_GetLargeFriendAvatarFn fn_get_large_friend_avatar = nullptr;

	SteamAPI_SteamUtilsFn fn_steam_utils = nullptr;
	ISteamUtils_GetImageSizeFn fn_get_image_size = nullptr;
	ISteamUtils_GetImageRGBAFn fn_get_image_rgba = nullptr;

	SteamAPI_SteamInventoryFn fn_steam_inventory = nullptr;
	ISteamInventory_GetResultStatusFn fn_inventory_get_result_status = nullptr;
	ISteamInventory_GetResultItemsFn fn_inventory_get_result_items = nullptr;
	ISteamInventory_GetResultItemPropertyFn fn_inventory_get_result_item_property = nullptr;
	ISteamInventory_GetResultTimestampFn fn_inventory_get_result_timestamp = nullptr;
	ISteamInventory_CheckResultSteamIDFn fn_inventory_check_result_steam_id = nullptr;
	ISteamInventory_DestroyResultFn fn_inventory_destroy_result = nullptr;
	ISteamInventory_GetAllItemsFn fn_inventory_get_all_items = nullptr;
	ISteamInventory_GetItemsByIDFn fn_inventory_get_items_by_id = nullptr;
	ISteamInventory_SerializeResultFn fn_inventory_serialize_result = nullptr;
	ISteamInventory_DeserializeResultFn fn_inventory_deserialize_result = nullptr;
	ISteamInventory_GrantPromoItemsFn fn_inventory_grant_promo_items = nullptr;
	ISteamInventory_AddPromoItemFn fn_inventory_add_promo_item = nullptr;
	ISteamInventory_AddPromoItemsFn fn_inventory_add_promo_items = nullptr;
	ISteamInventory_ConsumeItemFn fn_inventory_consume_item = nullptr;
	ISteamInventory_ExchangeItemsFn fn_inventory_exchange_items = nullptr;
	ISteamInventory_TransferItemQuantityFn fn_inventory_transfer_item_quantity = nullptr;
	ISteamInventory_LoadItemDefinitionsFn fn_inventory_load_item_definitions = nullptr;
	ISteamInventory_GetItemDefinitionIDsFn fn_inventory_get_item_definition_ids = nullptr;
	ISteamInventory_GetItemDefinitionPropertyFn fn_inventory_get_item_definition_property = nullptr;
	ISteamInventory_StartUpdatePropertiesFn fn_inventory_start_update_properties = nullptr;
	ISteamInventory_RemovePropertyFn fn_inventory_remove_property = nullptr;
	ISteamInventory_SetPropertyStringFn fn_inventory_set_property_string = nullptr;
	ISteamInventory_SetPropertyBoolFn fn_inventory_set_property_bool = nullptr;
	ISteamInventory_SetPropertyInt64Fn fn_inventory_set_property_int64 = nullptr;
	ISteamInventory_SetPropertyFloatFn fn_inventory_set_property_float = nullptr;
	ISteamInventory_SubmitUpdatePropertiesFn fn_inventory_submit_update_properties = nullptr;
	ISteamInventory_InspectItemFn fn_inventory_inspect_item = nullptr;
	ISteamInventory_TriggerItemDropFn fn_inventory_trigger_item_drop = nullptr;
	ISteamInventory_GetEligiblePromoItemDefinitionIDsFn fn_inventory_get_eligible_promo_item_definition_ids = nullptr;

	SteamAPI_SteamUGCFn fn_steam_ugc = nullptr;
	ISteamUGC_CreateQueryUserUGCRequestFn fn_ugc_create_query_user_request = nullptr;
	ISteamUGC_CreateQueryAllUGCRequestPageFn fn_ugc_create_query_all_request_page = nullptr;
	ISteamUGC_CreateQueryAllUGCRequestCursorFn fn_ugc_create_query_all_request_cursor = nullptr;
	ISteamUGC_CreateQueryUGCDetailsRequestFn fn_ugc_create_query_details_request = nullptr;
	ISteamUGC_SendQueryUGCRequestFn fn_ugc_send_query_request = nullptr;
	ISteamUGC_GetQueryUGCResultFn fn_ugc_get_query_result = nullptr;
	ISteamUGC_GetQueryUGCStringFn fn_ugc_get_query_preview_url = nullptr;
	ISteamUGC_GetQueryUGCStringFn fn_ugc_get_query_metadata = nullptr;
	ISteamUGC_GetQueryUGCChildrenFn fn_ugc_get_query_children = nullptr;
	ISteamUGC_GetQueryUGCStatisticFn fn_ugc_get_query_statistic = nullptr;
	ISteamUGC_GetQueryUGCCountFn fn_ugc_get_query_num_additional_previews = nullptr;
	ISteamUGC_GetQueryUGCAdditionalPreviewFn fn_ugc_get_query_additional_preview = nullptr;
	ISteamUGC_GetQueryUGCCountFn fn_ugc_get_query_num_key_value_tags = nullptr;
	ISteamUGC_GetQueryUGCKeyValueTagFn fn_ugc_get_query_key_value_tag = nullptr;
	ISteamUGC_QueryHandleFn fn_ugc_release_query_request = nullptr;
	ISteamUGC_QueryHandleStringFn fn_ugc_add_required_tag = nullptr;
	ISteamUGC_QueryHandleStringFn fn_ugc_add_excluded_tag = nullptr;
	ISteamUGC_QueryHandleKeyValueFn fn_ugc_add_required_key_value_tag = nullptr;
	ISteamUGC_QueryHandleBoolFn fn_ugc_set_return_only_ids = nullptr;
	ISteamUGC_QueryHandleBoolFn fn_ugc_set_return_key_value_tags = nullptr;
	ISteamUGC_QueryHandleBoolFn fn_ugc_set_return_long_description = nullptr;
	ISteamUGC_QueryHandleBoolFn fn_ugc_set_return_metadata = nullptr;
	ISteamUGC_QueryHandleBoolFn fn_ugc_set_return_children = nullptr;
	ISteamUGC_QueryHandleBoolFn fn_ugc_set_return_additional_previews = nullptr;
	ISteamUGC_QueryHandleBoolFn fn_ugc_set_return_total_only = nullptr;
	ISteamUGC_QueryHandleBoolFn fn_ugc_set_match_any_tag = nullptr;
	ISteamUGC_QueryHandleStringFn fn_ugc_set_language = nullptr;
	ISteamUGC_QueryHandleStringFn fn_ugc_set_search_text = nullptr;
	ISteamUGC_QueryHandleStringFn fn_ugc_set_cloud_file_name_filter = nullptr;
	ISteamUGC_QueryHandleUIntFn fn_ugc_set_allow_cached_response = nullptr;
	ISteamUGC_QueryHandleUIntFn fn_ugc_set_ranked_by_trend_days = nullptr;
	ISteamUGC_CreateItemFn fn_ugc_create_item = nullptr;
	ISteamUGC_StartItemUpdateFn fn_ugc_start_item_update = nullptr;
	ISteamUGC_UpdateHandleStringFn fn_ugc_set_item_title = nullptr;
	ISteamUGC_UpdateHandleStringFn fn_ugc_set_item_description = nullptr;
	ISteamUGC_UpdateHandleStringFn fn_ugc_set_item_update_language = nullptr;
	ISteamUGC_UpdateHandleStringFn fn_ugc_set_item_metadata = nullptr;
	ISteamUGC_UpdateHandleIntFn fn_ugc_set_item_visibility = nullptr;
	ISteamUGC_SetItemTagsFn fn_ugc_set_item_tags = nullptr;
	ISteamUGC_UpdateHandleStringFn fn_ugc_set_item_content = nullptr;
	ISteamUGC_UpdateHandleStringFn fn_ugc_set_item_preview = nullptr;
	ISteamUGC_UpdateHandleFn fn_ugc_remove_all_item_key_value_tags = nullptr;
	ISteamUGC_UpdateHandleStringFn fn_ugc_remove_item_key_value_tags = nullptr;
	ISteamUGC_UpdateHandleKeyValueFn fn_ugc_add_item_key_value_tag = nullptr;
	ISteamUGC_AddItemPreviewFileFn fn_ugc_add_item_preview_file = nullptr;
	ISteamUGC_UpdateHandleStringFn fn_ugc_add_item_preview_video = nullptr;
	ISteamUGC_UpdateHandleIntFn fn_ugc_remove_item_preview = nullptr;
	ISteamUGC_SubmitItemUpdateFn fn_ugc_submit_item_update = nullptr;
	ISteamUGC_GetItemUpdateProgressFn fn_ugc_get_item_update_progress = nullptr;
	ISteamUGC_SetUserItemVoteFn fn_ugc_set_user_item_vote = nullptr;
	ISteamUGC_AppFileIdCallFn fn_ugc_add_item_to_favorites = nullptr;
	ISteamUGC_AppFileIdCallFn fn_ugc_remove_item_from_favorites = nullptr;
	ISteamUGC_FileIdCallFn fn_ugc_subscribe_item = nullptr;
	ISteamUGC_FileIdCallFn fn_ugc_unsubscribe_item = nullptr;
	ISteamUGC_FileIdCallFn fn_ugc_delete_item = nullptr;
	ISteamUGC_GetNumSubscribedItemsFn fn_ugc_get_num_subscribed_items = nullptr;
	ISteamUGC_GetSubscribedItemsFn fn_ugc_get_subscribed_items = nullptr;
	ISteamUGC_GetItemStateFn fn_ugc_get_item_state = nullptr;
	ISteamUGC_GetItemInstallInfoFn fn_ugc_get_item_install_info = nullptr;
	ISteamUGC_GetItemDownloadInfoFn fn_ugc_get_item_download_info = nullptr;
	ISteamUGC_DownloadItemFn fn_ugc_download_item = nullptr;
	ISteamUGC_SuspendDownloadsFn fn_ugc_suspend_downloads = nullptr;
	ISteamUGC_PlaytimeTrackingFn fn_ugc_start_playtime_tracking = nullptr;
	ISteamUGC_PlaytimeTrackingFn fn_ugc_stop_playtime_tracking = nullptr;
	ISteamUGC_StopPlaytimeTrackingForAllItemsFn fn_ugc_stop_playtime_tracking_for_all_items = nullptr;
	ISteamUGC_ShowWorkshopEULAFn fn_ugc_show_workshop_eula = nullptr;
	// Optional: lets the Workshop resolve the running app ID on its own.
	SteamAPI_SteamUtilsFn fn_ugc_steam_utils = nullptr;
	ISteamUtils_GetAppIDFn fn_utils_get_app_id = nullptr;

	SteamAPI_SteamNetworkingSocketsFn fn_steam_networking_sockets = nullptr;
	SteamAPI_SteamNetworkingUtilsFn fn_steam_networking_utils = nullptr;
	ISteamNetworkingSockets_CreateListenSocketP2PFn fn_net_create_listen_socket_p2p = nullptr;
	ISteamNetworkingSockets_ConnectP2PFn fn_net_connect_p2p = nullptr;
	ISteamNetworkingSockets_AcceptConnectionFn fn_net_accept_connection = nullptr;
	ISteamNetworkingSockets_CloseConnectionFn fn_net_close_connection = nullptr;
	ISteamNetworkingSockets_CloseListenSocketFn fn_net_close_listen_socket = nullptr;
	ISteamNetworkingSockets_SendMessageToConnectionFn fn_net_send_message_to_connection = nullptr;
	ISteamNetworkingSockets_FlushMessagesOnConnectionFn fn_net_flush_messages_on_connection = nullptr;
	ISteamNetworkingSockets_GetConnectionInfoFn fn_net_get_connection_info = nullptr;
	ISteamNetworkingSockets_GetConnectionRealTimeStatusFn fn_net_get_connection_real_time_status = nullptr;
	ISteamNetworkingSockets_CreatePollGroupFn fn_net_create_poll_group = nullptr;
	ISteamNetworkingSockets_DestroyPollGroupFn fn_net_destroy_poll_group = nullptr;
	ISteamNetworkingSockets_SetConnectionPollGroupFn fn_net_set_connection_poll_group = nullptr;
	ISteamNetworkingSockets_ReceiveMessagesOnPollGroupFn fn_net_receive_messages_on_poll_group = nullptr;
	SteamNetworkingMessage_ReleaseFn fn_net_message_release = nullptr;
	ISteamNetworkingUtils_InitRelayNetworkAccessFn fn_net_init_relay_network_access = nullptr;

	SteamAPI_SteamMatchmakingFn fn_steam_matchmaking = nullptr;
	ISteamMatchmaking_RequestLobbyListFn fn_mm_request_lobby_list = nullptr;
	ISteamMatchmaking_AddStringFilterFn fn_mm_add_string_filter = nullptr;
	ISteamMatchmaking_AddNumericalFilterFn fn_mm_add_numerical_filter = nullptr;
	ISteamMatchmaking_AddIntFilterFn fn_mm_add_slots_available_filter = nullptr;
	ISteamMatchmaking_AddIntFilterFn fn_mm_add_distance_filter = nullptr;
	ISteamMatchmaking_AddIntFilterFn fn_mm_add_result_count_filter = nullptr;
	ISteamMatchmaking_GetLobbyByIndexFn fn_mm_get_lobby_by_index = nullptr;
	ISteamMatchmaking_CreateLobbyFn fn_mm_create_lobby = nullptr;
	ISteamMatchmaking_JoinLobbyFn fn_mm_join_lobby = nullptr;
	ISteamMatchmaking_LeaveLobbyFn fn_mm_leave_lobby = nullptr;
	ISteamMatchmaking_LobbyUserBoolFn fn_mm_invite_user_to_lobby = nullptr;
	ISteamMatchmaking_LobbyIntFn fn_mm_get_num_lobby_members = nullptr;
	ISteamMatchmaking_GetLobbyMemberByIndexFn fn_mm_get_lobby_member_by_index = nullptr;
	ISteamMatchmaking_GetLobbyDataFn fn_mm_get_lobby_data = nullptr;
	ISteamMatchmaking_SetLobbyDataFn fn_mm_set_lobby_data = nullptr;
	ISteamMatchmaking_LobbyIntFn fn_mm_get_lobby_data_count = nullptr;
	ISteamMatchmaking_GetLobbyDataByIndexFn fn_mm_get_lobby_data_by_index = nullptr;
	ISteamMatchmaking_DeleteLobbyDataFn fn_mm_delete_lobby_data = nullptr;
	ISteamMatchmaking_GetLobbyMemberDataFn fn_mm_get_lobby_member_data = nullptr;
	ISteamMatchmaking_SetLobbyMemberDataFn fn_mm_set_lobby_member_data = nullptr;
	ISteamMatchmaking_LobbyIntSetterFn fn_mm_set_lobby_member_limit = nullptr;
	ISteamMatchmaking_LobbyIntFn fn_mm_get_lobby_member_limit = nullptr;
	ISteamMatchmaking_LobbyIntSetterFn fn_mm_set_lobby_type = nullptr;
	ISteamMatchmaking_SetLobbyJoinableFn fn_mm_set_lobby_joinable = nullptr;
	ISteamMatchmaking_GetLobbyOwnerFn fn_mm_get_lobby_owner = nullptr;
	ISteamMatchmaking_LobbyUserBoolFn fn_mm_set_lobby_owner = nullptr;
	ISteamFriends_ActivateGameOverlayInviteDialogFn fn_friends_activate_game_overlay_invite_dialog = nullptr;
	ISteamFriends_GetFriendPersonaNameFn fn_friends_get_friend_persona_name = nullptr;

	bool _load_symbol(const char *p_name, void *&r_symbol, bool p_optional = false);
	bool _load_stats_symbols();
	bool _load_inventory_symbols();
	bool _load_workshop_symbols();
	void _clear_workshop_symbols();
	bool _load_networking_symbols();
	void _clear_networking_symbols();
	bool _load_matchmaking_symbols();
	void _clear_matchmaking_symbols();

public:
	static constexpr int kGetTicketForWebApiResponseCallback = 168; // k_iSteamUserCallbacks(100) + 68

	bool try_load();
	void unload();
	bool is_loaded() const { return loaded; }
	bool has_stats_support() const { return stats_loaded; }
	bool has_inventory_support() const { return inventory_loaded; }
	bool has_workshop_support() const { return workshop_loaded; }
	bool has_networking_support() const { return networking_loaded; }
	bool has_matchmaking_support() const { return matchmaking_loaded; }

	int init_flat(String &r_err_msg);
	void shutdown();
	HSteamPipe get_h_steam_pipe() const;
	void run_callbacks();
	bool has_run_callbacks() const { return fn_run_callbacks != nullptr; }
	void manual_dispatch_init();
	void manual_dispatch_run_frame(HSteamPipe p_pipe);
	bool manual_dispatch_get_next_callback(HSteamPipe p_pipe, void *p_callback_msg);
	void manual_dispatch_free_last_callback(HSteamPipe p_pipe);
	bool manual_dispatch_get_api_call_result(HSteamPipe p_pipe, uint64_t p_async_call, void *p_callback, int p_cub_callback, int p_callback_expected, bool *p_failed);
	bool is_steam_running() const;

	ISteamUserPtr get_steam_user() const;
	HAuthTicket get_auth_ticket_for_web_api(ISteamUserPtr p_user, const char *p_identity) const;
	void cancel_auth_ticket(ISteamUserPtr p_user, HAuthTicket p_ticket) const;
	bool is_logged_on(ISteamUserPtr p_user) const;
	uint64_t get_steam_id(ISteamUserPtr p_user) const;

	ISteamUserStatsPtr get_steam_user_stats() const;
	bool request_current_stats(ISteamUserStatsPtr p_stats) const;
	bool get_achievement(ISteamUserStatsPtr p_stats, const char *p_name, bool &r_achieved) const;
	bool set_achievement(ISteamUserStatsPtr p_stats, const char *p_name) const;
	bool clear_achievement(ISteamUserStatsPtr p_stats, const char *p_name) const;
	bool store_stats(ISteamUserStatsPtr p_stats) const;
	bool get_achievement_and_unlock_time(ISteamUserStatsPtr p_stats, const char *p_name, bool &r_achieved, uint32_t &r_unlock_time) const;
	int get_achievement_icon(ISteamUserStatsPtr p_stats, const char *p_name) const;
	String get_achievement_display_attribute(ISteamUserStatsPtr p_stats, const char *p_name, const char *p_key) const;
	uint32_t get_num_achievements(ISteamUserStatsPtr p_stats) const;
	String get_achievement_name(ISteamUserStatsPtr p_stats, uint32_t p_index) const;
	bool get_stat_int32(ISteamUserStatsPtr p_stats, const char *p_name, int32_t &r_data) const;
	bool get_stat_float(ISteamUserStatsPtr p_stats, const char *p_name, float &r_data) const;
	bool set_stat_int32(ISteamUserStatsPtr p_stats, const char *p_name, int32_t p_data) const;
	bool set_stat_float(ISteamUserStatsPtr p_stats, const char *p_name, float p_data) const;
	bool update_avg_rate_stat(ISteamUserStatsPtr p_stats, const char *p_name, float p_count_this_session, double p_session_length) const;

	ISteamFriendsPtr get_steam_friends() const;
	String get_persona_name(ISteamFriendsPtr p_friends) const;
	int get_friend_avatar(ISteamFriendsPtr p_friends, uint64_t p_steam_id, int p_size) const;

	ISteamUtilsPtr get_steam_utils() const;
	bool get_image_size(ISteamUtilsPtr p_utils, int p_image, uint32_t &r_width, uint32_t &r_height) const;
	bool get_image_rgba(ISteamUtilsPtr p_utils, int p_image, uint8_t *p_dest, int p_dest_size) const;

	static String bytes_to_hex(const uint8_t *p_data, int p_size);
	static Ref<Image> image_from_rgba(const uint8_t *p_data, int p_width, int p_height);
	Ref<Image> image_from_steam_handle(ISteamUtilsPtr p_utils, int p_image) const;

	ISteamInventoryPtr get_steam_inventory() const;
	int inventory_get_result_status(ISteamInventoryPtr p_inventory, SteamInventoryResult_t p_result) const;
	bool inventory_get_result_items(ISteamInventoryPtr p_inventory, SteamInventoryResult_t p_result, Vector<SteamItemDetails> &r_items) const;
	String inventory_get_result_item_property(ISteamInventoryPtr p_inventory, SteamInventoryResult_t p_result, uint32_t p_item_index, const char *p_property_name) const;
	uint32_t inventory_get_result_timestamp(ISteamInventoryPtr p_inventory, SteamInventoryResult_t p_result) const;
	bool inventory_check_result_steam_id(ISteamInventoryPtr p_inventory, SteamInventoryResult_t p_result, uint64_t p_steam_id) const;
	void inventory_destroy_result(ISteamInventoryPtr p_inventory, SteamInventoryResult_t p_result) const;
	bool inventory_get_all_items(ISteamInventoryPtr p_inventory, SteamInventoryResult_t &r_result) const;
	bool inventory_get_items_by_id(ISteamInventoryPtr p_inventory, SteamInventoryResult_t &r_result, const Vector<uint64_t> &p_instance_ids) const;
	bool inventory_serialize_result(ISteamInventoryPtr p_inventory, SteamInventoryResult_t p_result, PackedByteArray &r_bytes) const;
	bool inventory_deserialize_result(ISteamInventoryPtr p_inventory, SteamInventoryResult_t &r_result, const PackedByteArray &p_bytes, bool p_reserved = false) const;
	bool inventory_grant_promo_items(ISteamInventoryPtr p_inventory, SteamInventoryResult_t &r_result) const;
	bool inventory_add_promo_item(ISteamInventoryPtr p_inventory, SteamInventoryResult_t &r_result, SteamItemDef_t p_item_def) const;
	bool inventory_add_promo_items(ISteamInventoryPtr p_inventory, SteamInventoryResult_t &r_result, const Vector<int32_t> &p_item_defs) const;
	bool inventory_consume_item(ISteamInventoryPtr p_inventory, SteamInventoryResult_t &r_result, SteamItemInstanceID_t p_item, uint32_t p_quantity) const;
	bool inventory_exchange_items(ISteamInventoryPtr p_inventory, SteamInventoryResult_t &r_result, const Vector<int32_t> &p_generate_defs, const Vector<int32_t> &p_generate_quantities, const Vector<uint64_t> &p_destroy_ids, const Vector<int32_t> &p_destroy_quantities) const;
	bool inventory_transfer_item_quantity(ISteamInventoryPtr p_inventory, SteamInventoryResult_t &r_result, SteamItemInstanceID_t p_source, uint32_t p_quantity, SteamItemInstanceID_t p_dest) const;
	bool inventory_load_item_definitions(ISteamInventoryPtr p_inventory) const;
	Vector<int32_t> inventory_get_item_definition_ids(ISteamInventoryPtr p_inventory) const;
	String inventory_get_item_definition_property(ISteamInventoryPtr p_inventory, SteamItemDef_t p_definition, const char *p_property_name) const;
	Dictionary inventory_get_item_definition_properties(ISteamInventoryPtr p_inventory, SteamItemDef_t p_definition) const;
	SteamInventoryUpdateHandle_t inventory_start_update_properties(ISteamInventoryPtr p_inventory) const;
	bool inventory_remove_property(ISteamInventoryPtr p_inventory, SteamInventoryUpdateHandle_t p_handle, SteamItemInstanceID_t p_item_id, const char *p_property_name) const;
	bool inventory_set_property_string(ISteamInventoryPtr p_inventory, SteamInventoryUpdateHandle_t p_handle, SteamItemInstanceID_t p_item_id, const char *p_property_name, const char *p_value) const;
	bool inventory_set_property_bool(ISteamInventoryPtr p_inventory, SteamInventoryUpdateHandle_t p_handle, SteamItemInstanceID_t p_item_id, const char *p_property_name, bool p_value) const;
	bool inventory_set_property_int64(ISteamInventoryPtr p_inventory, SteamInventoryUpdateHandle_t p_handle, SteamItemInstanceID_t p_item_id, const char *p_property_name, int64_t p_value) const;
	bool inventory_set_property_float(ISteamInventoryPtr p_inventory, SteamInventoryUpdateHandle_t p_handle, SteamItemInstanceID_t p_item_id, const char *p_property_name, float p_value) const;
	bool inventory_submit_update_properties(ISteamInventoryPtr p_inventory, SteamInventoryUpdateHandle_t p_handle, SteamInventoryResult_t &r_result) const;
	bool inventory_inspect_item(ISteamInventoryPtr p_inventory, SteamInventoryResult_t &r_result, const char *p_item_token) const;
	bool inventory_trigger_item_drop(ISteamInventoryPtr p_inventory, SteamInventoryResult_t &r_result, SteamItemDef_t p_drop_list_definition) const;
	Vector<int32_t> inventory_get_eligible_promo_item_definition_ids(ISteamInventoryPtr p_inventory, uint64_t p_steam_id) const;

	// Steam Workshop (ISteamUGC). All wrappers return a safe failure value when
	// the symbols are unavailable.
	ISteamUGCPtr get_steam_ugc() const;
	uint32_t get_app_id() const;
	SteamUGCQueryHandle_t ugc_create_query_user_request(ISteamUGCPtr p_ugc, uint32_t p_account_id, int p_list_type, int p_matching_type, int p_sort_order, uint32_t p_creator_app_id, uint32_t p_consumer_app_id, uint32_t p_page) const;
	SteamUGCQueryHandle_t ugc_create_query_all_request_page(ISteamUGCPtr p_ugc, int p_query_type, int p_matching_type, uint32_t p_creator_app_id, uint32_t p_consumer_app_id, uint32_t p_page) const;
	SteamUGCQueryHandle_t ugc_create_query_all_request_cursor(ISteamUGCPtr p_ugc, int p_query_type, int p_matching_type, uint32_t p_creator_app_id, uint32_t p_consumer_app_id, const char *p_cursor) const;
	SteamUGCQueryHandle_t ugc_create_query_details_request(ISteamUGCPtr p_ugc, const Vector<SteamPublishedFileId_t> &p_file_ids) const;
	SteamAPICallHandle_t ugc_send_query_request(ISteamUGCPtr p_ugc, SteamUGCQueryHandle_t p_handle) const;
	bool ugc_get_query_result(ISteamUGCPtr p_ugc, SteamUGCQueryHandle_t p_handle, uint32_t p_index, SteamUGCDetails &r_details) const;
	String ugc_get_query_preview_url(ISteamUGCPtr p_ugc, SteamUGCQueryHandle_t p_handle, uint32_t p_index) const;
	String ugc_get_query_metadata(ISteamUGCPtr p_ugc, SteamUGCQueryHandle_t p_handle, uint32_t p_index) const;
	Vector<SteamPublishedFileId_t> ugc_get_query_children(ISteamUGCPtr p_ugc, SteamUGCQueryHandle_t p_handle, uint32_t p_index, uint32_t p_num_children) const;
	bool ugc_get_query_statistic(ISteamUGCPtr p_ugc, SteamUGCQueryHandle_t p_handle, uint32_t p_index, int p_stat_type, uint64_t &r_value) const;
	uint32_t ugc_get_query_num_additional_previews(ISteamUGCPtr p_ugc, SteamUGCQueryHandle_t p_handle, uint32_t p_index) const;
	bool ugc_get_query_additional_preview(ISteamUGCPtr p_ugc, SteamUGCQueryHandle_t p_handle, uint32_t p_index, uint32_t p_preview_index, String &r_url_or_video_id, String &r_original_file_name, int &r_preview_type) const;
	uint32_t ugc_get_query_num_key_value_tags(ISteamUGCPtr p_ugc, SteamUGCQueryHandle_t p_handle, uint32_t p_index) const;
	bool ugc_get_query_key_value_tag(ISteamUGCPtr p_ugc, SteamUGCQueryHandle_t p_handle, uint32_t p_index, uint32_t p_tag_index, String &r_key, String &r_value) const;
	bool ugc_release_query_request(ISteamUGCPtr p_ugc, SteamUGCQueryHandle_t p_handle) const;
	bool ugc_add_required_tag(ISteamUGCPtr p_ugc, SteamUGCQueryHandle_t p_handle, const char *p_tag) const;
	bool ugc_add_excluded_tag(ISteamUGCPtr p_ugc, SteamUGCQueryHandle_t p_handle, const char *p_tag) const;
	bool ugc_add_required_key_value_tag(ISteamUGCPtr p_ugc, SteamUGCQueryHandle_t p_handle, const char *p_key, const char *p_value) const;
	bool ugc_set_return_only_ids(ISteamUGCPtr p_ugc, SteamUGCQueryHandle_t p_handle, bool p_value) const;
	bool ugc_set_return_key_value_tags(ISteamUGCPtr p_ugc, SteamUGCQueryHandle_t p_handle, bool p_value) const;
	bool ugc_set_return_long_description(ISteamUGCPtr p_ugc, SteamUGCQueryHandle_t p_handle, bool p_value) const;
	bool ugc_set_return_metadata(ISteamUGCPtr p_ugc, SteamUGCQueryHandle_t p_handle, bool p_value) const;
	bool ugc_set_return_children(ISteamUGCPtr p_ugc, SteamUGCQueryHandle_t p_handle, bool p_value) const;
	bool ugc_set_return_additional_previews(ISteamUGCPtr p_ugc, SteamUGCQueryHandle_t p_handle, bool p_value) const;
	bool ugc_set_return_total_only(ISteamUGCPtr p_ugc, SteamUGCQueryHandle_t p_handle, bool p_value) const;
	bool ugc_set_match_any_tag(ISteamUGCPtr p_ugc, SteamUGCQueryHandle_t p_handle, bool p_value) const;
	bool ugc_set_language(ISteamUGCPtr p_ugc, SteamUGCQueryHandle_t p_handle, const char *p_language) const;
	bool ugc_set_search_text(ISteamUGCPtr p_ugc, SteamUGCQueryHandle_t p_handle, const char *p_text) const;
	bool ugc_set_cloud_file_name_filter(ISteamUGCPtr p_ugc, SteamUGCQueryHandle_t p_handle, const char *p_file_name) const;
	bool ugc_set_allow_cached_response(ISteamUGCPtr p_ugc, SteamUGCQueryHandle_t p_handle, uint32_t p_max_age_seconds) const;
	bool ugc_set_ranked_by_trend_days(ISteamUGCPtr p_ugc, SteamUGCQueryHandle_t p_handle, uint32_t p_days) const;

	SteamAPICallHandle_t ugc_create_item(ISteamUGCPtr p_ugc, uint32_t p_consumer_app_id, int p_file_type) const;
	SteamUGCUpdateHandle_t ugc_start_item_update(ISteamUGCPtr p_ugc, uint32_t p_consumer_app_id, SteamPublishedFileId_t p_file_id) const;
	bool ugc_set_item_title(ISteamUGCPtr p_ugc, SteamUGCUpdateHandle_t p_handle, const char *p_title) const;
	bool ugc_set_item_description(ISteamUGCPtr p_ugc, SteamUGCUpdateHandle_t p_handle, const char *p_description) const;
	bool ugc_set_item_update_language(ISteamUGCPtr p_ugc, SteamUGCUpdateHandle_t p_handle, const char *p_language) const;
	bool ugc_set_item_metadata(ISteamUGCPtr p_ugc, SteamUGCUpdateHandle_t p_handle, const char *p_metadata) const;
	bool ugc_set_item_visibility(ISteamUGCPtr p_ugc, SteamUGCUpdateHandle_t p_handle, int p_visibility) const;
	bool ugc_set_item_tags(ISteamUGCPtr p_ugc, SteamUGCUpdateHandle_t p_handle, const Vector<String> &p_tags, bool p_allow_admin_tags) const;
	bool ugc_set_item_content(ISteamUGCPtr p_ugc, SteamUGCUpdateHandle_t p_handle, const char *p_content_folder) const;
	bool ugc_set_item_preview(ISteamUGCPtr p_ugc, SteamUGCUpdateHandle_t p_handle, const char *p_preview_file) const;
	bool ugc_remove_all_item_key_value_tags(ISteamUGCPtr p_ugc, SteamUGCUpdateHandle_t p_handle) const;
	bool ugc_remove_item_key_value_tags(ISteamUGCPtr p_ugc, SteamUGCUpdateHandle_t p_handle, const char *p_key) const;
	bool ugc_add_item_key_value_tag(ISteamUGCPtr p_ugc, SteamUGCUpdateHandle_t p_handle, const char *p_key, const char *p_value) const;
	bool ugc_add_item_preview_file(ISteamUGCPtr p_ugc, SteamUGCUpdateHandle_t p_handle, const char *p_preview_file, int p_type) const;
	bool ugc_add_item_preview_video(ISteamUGCPtr p_ugc, SteamUGCUpdateHandle_t p_handle, const char *p_video_id) const;
	bool ugc_remove_item_preview(ISteamUGCPtr p_ugc, SteamUGCUpdateHandle_t p_handle, uint32_t p_index) const;
	SteamAPICallHandle_t ugc_submit_item_update(ISteamUGCPtr p_ugc, SteamUGCUpdateHandle_t p_handle, const char *p_change_note) const;
	int ugc_get_item_update_progress(ISteamUGCPtr p_ugc, SteamUGCUpdateHandle_t p_handle, uint64_t &r_bytes_processed, uint64_t &r_bytes_total) const;

	SteamAPICallHandle_t ugc_set_user_item_vote(ISteamUGCPtr p_ugc, SteamPublishedFileId_t p_file_id, bool p_vote_up) const;
	SteamAPICallHandle_t ugc_add_item_to_favorites(ISteamUGCPtr p_ugc, uint32_t p_app_id, SteamPublishedFileId_t p_file_id) const;
	SteamAPICallHandle_t ugc_remove_item_from_favorites(ISteamUGCPtr p_ugc, uint32_t p_app_id, SteamPublishedFileId_t p_file_id) const;
	SteamAPICallHandle_t ugc_subscribe_item(ISteamUGCPtr p_ugc, SteamPublishedFileId_t p_file_id) const;
	SteamAPICallHandle_t ugc_unsubscribe_item(ISteamUGCPtr p_ugc, SteamPublishedFileId_t p_file_id) const;
	SteamAPICallHandle_t ugc_delete_item(ISteamUGCPtr p_ugc, SteamPublishedFileId_t p_file_id) const;
	Vector<SteamPublishedFileId_t> ugc_get_subscribed_items(ISteamUGCPtr p_ugc, bool p_include_locally_disabled) const;
	uint32_t ugc_get_num_subscribed_items(ISteamUGCPtr p_ugc, bool p_include_locally_disabled) const;
	uint32_t ugc_get_item_state(ISteamUGCPtr p_ugc, SteamPublishedFileId_t p_file_id) const;
	bool ugc_get_item_install_info(ISteamUGCPtr p_ugc, SteamPublishedFileId_t p_file_id, uint64_t &r_size_on_disk, String &r_folder, uint32_t &r_timestamp) const;
	bool ugc_get_item_download_info(ISteamUGCPtr p_ugc, SteamPublishedFileId_t p_file_id, uint64_t &r_bytes_downloaded, uint64_t &r_bytes_total) const;
	bool ugc_download_item(ISteamUGCPtr p_ugc, SteamPublishedFileId_t p_file_id, bool p_high_priority) const;
	void ugc_suspend_downloads(ISteamUGCPtr p_ugc, bool p_suspend) const;
	SteamAPICallHandle_t ugc_start_playtime_tracking(ISteamUGCPtr p_ugc, const Vector<SteamPublishedFileId_t> &p_file_ids) const;
	SteamAPICallHandle_t ugc_stop_playtime_tracking(ISteamUGCPtr p_ugc, const Vector<SteamPublishedFileId_t> &p_file_ids) const;
	SteamAPICallHandle_t ugc_stop_playtime_tracking_for_all_items(ISteamUGCPtr p_ugc) const;
	bool ugc_show_workshop_eula(ISteamUGCPtr p_ugc) const;

	// Steam Networking Sockets. All wrappers return a safe failure value when
	// the symbols are unavailable.
	ISteamNetworkingSocketsPtr get_steam_networking_sockets() const;
	ISteamNetworkingUtilsPtr get_steam_networking_utils() const;
	void net_init_relay_network_access(ISteamNetworkingUtilsPtr p_utils) const;
	SteamHSteamListenSocket net_create_listen_socket_p2p(ISteamNetworkingSocketsPtr p_sockets, int p_virtual_port) const;
	SteamHSteamNetConnection net_connect_p2p(ISteamNetworkingSocketsPtr p_sockets, uint64_t p_steam_id, int p_virtual_port) const;
	int net_accept_connection(ISteamNetworkingSocketsPtr p_sockets, SteamHSteamNetConnection p_connection) const;
	bool net_close_connection(ISteamNetworkingSocketsPtr p_sockets, SteamHSteamNetConnection p_connection, int p_reason, const char *p_debug, bool p_enable_linger) const;
	bool net_close_listen_socket(ISteamNetworkingSocketsPtr p_sockets, SteamHSteamListenSocket p_socket) const;
	int net_send_message_to_connection(ISteamNetworkingSocketsPtr p_sockets, SteamHSteamNetConnection p_connection, const void *p_data, uint32_t p_size, int p_send_flags) const;
	int net_flush_messages_on_connection(ISteamNetworkingSocketsPtr p_sockets, SteamHSteamNetConnection p_connection) const;
	bool net_get_connection_info(ISteamNetworkingSocketsPtr p_sockets, SteamHSteamNetConnection p_connection, SteamNetConnectionInfo &r_info) const;
	bool net_get_connection_real_time_status(ISteamNetworkingSocketsPtr p_sockets, SteamHSteamNetConnection p_connection, SteamNetConnectionRealTimeStatus &r_status) const;
	SteamHSteamNetPollGroup net_create_poll_group(ISteamNetworkingSocketsPtr p_sockets) const;
	bool net_destroy_poll_group(ISteamNetworkingSocketsPtr p_sockets, SteamHSteamNetPollGroup p_poll_group) const;
	bool net_set_connection_poll_group(ISteamNetworkingSocketsPtr p_sockets, SteamHSteamNetConnection p_connection, SteamHSteamNetPollGroup p_poll_group) const;
	int net_receive_messages_on_poll_group(ISteamNetworkingSocketsPtr p_sockets, SteamHSteamNetPollGroup p_poll_group, SteamNetworkingMessage **r_messages, int p_max_messages) const;
	void net_release_message(SteamNetworkingMessage *p_message) const;

	// Steam Matchmaking (lobbies).
	ISteamMatchmakingPtr get_steam_matchmaking() const;
	SteamAPICallHandle_t mm_request_lobby_list(ISteamMatchmakingPtr p_matchmaking) const;
	void mm_add_request_lobby_list_string_filter(ISteamMatchmakingPtr p_matchmaking, const char *p_key, const char *p_value, int p_comparison) const;
	void mm_add_request_lobby_list_numerical_filter(ISteamMatchmakingPtr p_matchmaking, const char *p_key, int p_value, int p_comparison) const;
	void mm_add_request_lobby_list_filter_slots_available(ISteamMatchmakingPtr p_matchmaking, int p_slots) const;
	void mm_add_request_lobby_list_distance_filter(ISteamMatchmakingPtr p_matchmaking, int p_distance) const;
	void mm_add_request_lobby_list_result_count_filter(ISteamMatchmakingPtr p_matchmaking, int p_max_results) const;
	uint64_t mm_get_lobby_by_index(ISteamMatchmakingPtr p_matchmaking, int p_index) const;
	SteamAPICallHandle_t mm_create_lobby(ISteamMatchmakingPtr p_matchmaking, int p_lobby_type, int p_max_members) const;
	SteamAPICallHandle_t mm_join_lobby(ISteamMatchmakingPtr p_matchmaking, uint64_t p_lobby_id) const;
	void mm_leave_lobby(ISteamMatchmakingPtr p_matchmaking, uint64_t p_lobby_id) const;
	bool mm_invite_user_to_lobby(ISteamMatchmakingPtr p_matchmaking, uint64_t p_lobby_id, uint64_t p_steam_id) const;
	int mm_get_num_lobby_members(ISteamMatchmakingPtr p_matchmaking, uint64_t p_lobby_id) const;
	uint64_t mm_get_lobby_member_by_index(ISteamMatchmakingPtr p_matchmaking, uint64_t p_lobby_id, int p_index) const;
	String mm_get_lobby_data(ISteamMatchmakingPtr p_matchmaking, uint64_t p_lobby_id, const char *p_key) const;
	bool mm_set_lobby_data(ISteamMatchmakingPtr p_matchmaking, uint64_t p_lobby_id, const char *p_key, const char *p_value) const;
	Dictionary mm_get_all_lobby_data(ISteamMatchmakingPtr p_matchmaking, uint64_t p_lobby_id) const;
	bool mm_delete_lobby_data(ISteamMatchmakingPtr p_matchmaking, uint64_t p_lobby_id, const char *p_key) const;
	String mm_get_lobby_member_data(ISteamMatchmakingPtr p_matchmaking, uint64_t p_lobby_id, uint64_t p_steam_id, const char *p_key) const;
	void mm_set_lobby_member_data(ISteamMatchmakingPtr p_matchmaking, uint64_t p_lobby_id, const char *p_key, const char *p_value) const;
	bool mm_set_lobby_member_limit(ISteamMatchmakingPtr p_matchmaking, uint64_t p_lobby_id, int p_max_members) const;
	int mm_get_lobby_member_limit(ISteamMatchmakingPtr p_matchmaking, uint64_t p_lobby_id) const;
	bool mm_set_lobby_type(ISteamMatchmakingPtr p_matchmaking, uint64_t p_lobby_id, int p_lobby_type) const;
	bool mm_set_lobby_joinable(ISteamMatchmakingPtr p_matchmaking, uint64_t p_lobby_id, bool p_joinable) const;
	uint64_t mm_get_lobby_owner(ISteamMatchmakingPtr p_matchmaking, uint64_t p_lobby_id) const;
	bool mm_set_lobby_owner(ISteamMatchmakingPtr p_matchmaking, uint64_t p_lobby_id, uint64_t p_steam_id) const;
	void friends_activate_game_overlay_invite_dialog(ISteamFriendsPtr p_friends, uint64_t p_lobby_id) const;
	String friends_get_friend_persona_name(ISteamFriendsPtr p_friends, uint64_t p_steam_id) const;
};
