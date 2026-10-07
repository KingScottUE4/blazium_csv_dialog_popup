/**************************************************************************/
/*  steam_api_loader_workshop.cpp                                         */
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

// Steam Workshop (ISteamUGC) symbols and wrappers for SteamAPILoader.

#include "steam_api_loader.h"

#include <cstring>

bool SteamAPILoader::_load_workshop_symbols() {
	void *symbol = nullptr;

	// SteamUGC_v021 ships with Steamworks SDK 1.62+, v020 with 1.60-1.61.
	// Every flat function used below has the same name in both versions; the
	// only signature difference (the trailing bIncludeLocallyDisabled flag on
	// Get[Num]SubscribedItems) is harmless to pass to v020 since extra
	// arguments are ignored by the callee.
	if (!_load_symbol("SteamAPI_SteamUGC_v021", symbol, true) && !_load_symbol("SteamAPI_SteamUGC_v020", symbol, true)) {
		return false;
	}
	fn_steam_ugc = (SteamAPI_SteamUGCFn)symbol;

#define LOAD_UGC_SYM(name, field, type) \
	if (!_load_symbol(name, symbol)) {  \
		return false;                   \
	}                                   \
	field = (type)symbol;

	LOAD_UGC_SYM("SteamAPI_ISteamUGC_CreateQueryUserUGCRequest", fn_ugc_create_query_user_request, ISteamUGC_CreateQueryUserUGCRequestFn);
	LOAD_UGC_SYM("SteamAPI_ISteamUGC_CreateQueryAllUGCRequestPage", fn_ugc_create_query_all_request_page, ISteamUGC_CreateQueryAllUGCRequestPageFn);
	LOAD_UGC_SYM("SteamAPI_ISteamUGC_CreateQueryAllUGCRequestCursor", fn_ugc_create_query_all_request_cursor, ISteamUGC_CreateQueryAllUGCRequestCursorFn);
	LOAD_UGC_SYM("SteamAPI_ISteamUGC_CreateQueryUGCDetailsRequest", fn_ugc_create_query_details_request, ISteamUGC_CreateQueryUGCDetailsRequestFn);
	LOAD_UGC_SYM("SteamAPI_ISteamUGC_SendQueryUGCRequest", fn_ugc_send_query_request, ISteamUGC_SendQueryUGCRequestFn);
	LOAD_UGC_SYM("SteamAPI_ISteamUGC_GetQueryUGCResult", fn_ugc_get_query_result, ISteamUGC_GetQueryUGCResultFn);
	LOAD_UGC_SYM("SteamAPI_ISteamUGC_GetQueryUGCPreviewURL", fn_ugc_get_query_preview_url, ISteamUGC_GetQueryUGCStringFn);
	LOAD_UGC_SYM("SteamAPI_ISteamUGC_GetQueryUGCMetadata", fn_ugc_get_query_metadata, ISteamUGC_GetQueryUGCStringFn);
	LOAD_UGC_SYM("SteamAPI_ISteamUGC_GetQueryUGCChildren", fn_ugc_get_query_children, ISteamUGC_GetQueryUGCChildrenFn);
	LOAD_UGC_SYM("SteamAPI_ISteamUGC_GetQueryUGCStatistic", fn_ugc_get_query_statistic, ISteamUGC_GetQueryUGCStatisticFn);
	LOAD_UGC_SYM("SteamAPI_ISteamUGC_GetQueryUGCNumAdditionalPreviews", fn_ugc_get_query_num_additional_previews, ISteamUGC_GetQueryUGCCountFn);
	LOAD_UGC_SYM("SteamAPI_ISteamUGC_GetQueryUGCAdditionalPreview", fn_ugc_get_query_additional_preview, ISteamUGC_GetQueryUGCAdditionalPreviewFn);
	LOAD_UGC_SYM("SteamAPI_ISteamUGC_GetQueryUGCNumKeyValueTags", fn_ugc_get_query_num_key_value_tags, ISteamUGC_GetQueryUGCCountFn);
	LOAD_UGC_SYM("SteamAPI_ISteamUGC_GetQueryUGCKeyValueTag", fn_ugc_get_query_key_value_tag, ISteamUGC_GetQueryUGCKeyValueTagFn);
	LOAD_UGC_SYM("SteamAPI_ISteamUGC_ReleaseQueryUGCRequest", fn_ugc_release_query_request, ISteamUGC_QueryHandleFn);
	LOAD_UGC_SYM("SteamAPI_ISteamUGC_AddRequiredTag", fn_ugc_add_required_tag, ISteamUGC_QueryHandleStringFn);
	LOAD_UGC_SYM("SteamAPI_ISteamUGC_AddExcludedTag", fn_ugc_add_excluded_tag, ISteamUGC_QueryHandleStringFn);
	LOAD_UGC_SYM("SteamAPI_ISteamUGC_AddRequiredKeyValueTag", fn_ugc_add_required_key_value_tag, ISteamUGC_QueryHandleKeyValueFn);
	LOAD_UGC_SYM("SteamAPI_ISteamUGC_SetReturnOnlyIDs", fn_ugc_set_return_only_ids, ISteamUGC_QueryHandleBoolFn);
	LOAD_UGC_SYM("SteamAPI_ISteamUGC_SetReturnKeyValueTags", fn_ugc_set_return_key_value_tags, ISteamUGC_QueryHandleBoolFn);
	LOAD_UGC_SYM("SteamAPI_ISteamUGC_SetReturnLongDescription", fn_ugc_set_return_long_description, ISteamUGC_QueryHandleBoolFn);
	LOAD_UGC_SYM("SteamAPI_ISteamUGC_SetReturnMetadata", fn_ugc_set_return_metadata, ISteamUGC_QueryHandleBoolFn);
	LOAD_UGC_SYM("SteamAPI_ISteamUGC_SetReturnChildren", fn_ugc_set_return_children, ISteamUGC_QueryHandleBoolFn);
	LOAD_UGC_SYM("SteamAPI_ISteamUGC_SetReturnAdditionalPreviews", fn_ugc_set_return_additional_previews, ISteamUGC_QueryHandleBoolFn);
	LOAD_UGC_SYM("SteamAPI_ISteamUGC_SetReturnTotalOnly", fn_ugc_set_return_total_only, ISteamUGC_QueryHandleBoolFn);
	LOAD_UGC_SYM("SteamAPI_ISteamUGC_SetMatchAnyTag", fn_ugc_set_match_any_tag, ISteamUGC_QueryHandleBoolFn);
	LOAD_UGC_SYM("SteamAPI_ISteamUGC_SetLanguage", fn_ugc_set_language, ISteamUGC_QueryHandleStringFn);
	LOAD_UGC_SYM("SteamAPI_ISteamUGC_SetSearchText", fn_ugc_set_search_text, ISteamUGC_QueryHandleStringFn);
	LOAD_UGC_SYM("SteamAPI_ISteamUGC_SetCloudFileNameFilter", fn_ugc_set_cloud_file_name_filter, ISteamUGC_QueryHandleStringFn);
	LOAD_UGC_SYM("SteamAPI_ISteamUGC_SetAllowCachedResponse", fn_ugc_set_allow_cached_response, ISteamUGC_QueryHandleUIntFn);
	LOAD_UGC_SYM("SteamAPI_ISteamUGC_SetRankedByTrendDays", fn_ugc_set_ranked_by_trend_days, ISteamUGC_QueryHandleUIntFn);
	LOAD_UGC_SYM("SteamAPI_ISteamUGC_CreateItem", fn_ugc_create_item, ISteamUGC_CreateItemFn);
	LOAD_UGC_SYM("SteamAPI_ISteamUGC_StartItemUpdate", fn_ugc_start_item_update, ISteamUGC_StartItemUpdateFn);
	LOAD_UGC_SYM("SteamAPI_ISteamUGC_SetItemTitle", fn_ugc_set_item_title, ISteamUGC_UpdateHandleStringFn);
	LOAD_UGC_SYM("SteamAPI_ISteamUGC_SetItemDescription", fn_ugc_set_item_description, ISteamUGC_UpdateHandleStringFn);
	LOAD_UGC_SYM("SteamAPI_ISteamUGC_SetItemUpdateLanguage", fn_ugc_set_item_update_language, ISteamUGC_UpdateHandleStringFn);
	LOAD_UGC_SYM("SteamAPI_ISteamUGC_SetItemMetadata", fn_ugc_set_item_metadata, ISteamUGC_UpdateHandleStringFn);
	LOAD_UGC_SYM("SteamAPI_ISteamUGC_SetItemVisibility", fn_ugc_set_item_visibility, ISteamUGC_UpdateHandleIntFn);
	LOAD_UGC_SYM("SteamAPI_ISteamUGC_SetItemTags", fn_ugc_set_item_tags, ISteamUGC_SetItemTagsFn);
	LOAD_UGC_SYM("SteamAPI_ISteamUGC_SetItemContent", fn_ugc_set_item_content, ISteamUGC_UpdateHandleStringFn);
	LOAD_UGC_SYM("SteamAPI_ISteamUGC_SetItemPreview", fn_ugc_set_item_preview, ISteamUGC_UpdateHandleStringFn);
	LOAD_UGC_SYM("SteamAPI_ISteamUGC_RemoveAllItemKeyValueTags", fn_ugc_remove_all_item_key_value_tags, ISteamUGC_UpdateHandleFn);
	LOAD_UGC_SYM("SteamAPI_ISteamUGC_RemoveItemKeyValueTags", fn_ugc_remove_item_key_value_tags, ISteamUGC_UpdateHandleStringFn);
	LOAD_UGC_SYM("SteamAPI_ISteamUGC_AddItemKeyValueTag", fn_ugc_add_item_key_value_tag, ISteamUGC_UpdateHandleKeyValueFn);
	LOAD_UGC_SYM("SteamAPI_ISteamUGC_AddItemPreviewFile", fn_ugc_add_item_preview_file, ISteamUGC_AddItemPreviewFileFn);
	LOAD_UGC_SYM("SteamAPI_ISteamUGC_AddItemPreviewVideo", fn_ugc_add_item_preview_video, ISteamUGC_UpdateHandleStringFn);
	LOAD_UGC_SYM("SteamAPI_ISteamUGC_RemoveItemPreview", fn_ugc_remove_item_preview, ISteamUGC_UpdateHandleIntFn);
	LOAD_UGC_SYM("SteamAPI_ISteamUGC_SubmitItemUpdate", fn_ugc_submit_item_update, ISteamUGC_SubmitItemUpdateFn);
	LOAD_UGC_SYM("SteamAPI_ISteamUGC_GetItemUpdateProgress", fn_ugc_get_item_update_progress, ISteamUGC_GetItemUpdateProgressFn);
	LOAD_UGC_SYM("SteamAPI_ISteamUGC_SetUserItemVote", fn_ugc_set_user_item_vote, ISteamUGC_SetUserItemVoteFn);
	LOAD_UGC_SYM("SteamAPI_ISteamUGC_AddItemToFavorites", fn_ugc_add_item_to_favorites, ISteamUGC_AppFileIdCallFn);
	LOAD_UGC_SYM("SteamAPI_ISteamUGC_RemoveItemFromFavorites", fn_ugc_remove_item_from_favorites, ISteamUGC_AppFileIdCallFn);
	LOAD_UGC_SYM("SteamAPI_ISteamUGC_SubscribeItem", fn_ugc_subscribe_item, ISteamUGC_FileIdCallFn);
	LOAD_UGC_SYM("SteamAPI_ISteamUGC_UnsubscribeItem", fn_ugc_unsubscribe_item, ISteamUGC_FileIdCallFn);
	LOAD_UGC_SYM("SteamAPI_ISteamUGC_DeleteItem", fn_ugc_delete_item, ISteamUGC_FileIdCallFn);
	LOAD_UGC_SYM("SteamAPI_ISteamUGC_GetNumSubscribedItems", fn_ugc_get_num_subscribed_items, ISteamUGC_GetNumSubscribedItemsFn);
	LOAD_UGC_SYM("SteamAPI_ISteamUGC_GetSubscribedItems", fn_ugc_get_subscribed_items, ISteamUGC_GetSubscribedItemsFn);
	LOAD_UGC_SYM("SteamAPI_ISteamUGC_GetItemState", fn_ugc_get_item_state, ISteamUGC_GetItemStateFn);
	LOAD_UGC_SYM("SteamAPI_ISteamUGC_GetItemInstallInfo", fn_ugc_get_item_install_info, ISteamUGC_GetItemInstallInfoFn);
	LOAD_UGC_SYM("SteamAPI_ISteamUGC_GetItemDownloadInfo", fn_ugc_get_item_download_info, ISteamUGC_GetItemDownloadInfoFn);
	LOAD_UGC_SYM("SteamAPI_ISteamUGC_DownloadItem", fn_ugc_download_item, ISteamUGC_DownloadItemFn);
	LOAD_UGC_SYM("SteamAPI_ISteamUGC_SuspendDownloads", fn_ugc_suspend_downloads, ISteamUGC_SuspendDownloadsFn);
	LOAD_UGC_SYM("SteamAPI_ISteamUGC_StartPlaytimeTracking", fn_ugc_start_playtime_tracking, ISteamUGC_PlaytimeTrackingFn);
	LOAD_UGC_SYM("SteamAPI_ISteamUGC_StopPlaytimeTracking", fn_ugc_stop_playtime_tracking, ISteamUGC_PlaytimeTrackingFn);
	LOAD_UGC_SYM("SteamAPI_ISteamUGC_StopPlaytimeTrackingForAllItems", fn_ugc_stop_playtime_tracking_for_all_items, ISteamUGC_StopPlaytimeTrackingForAllItemsFn);
	LOAD_UGC_SYM("SteamAPI_ISteamUGC_ShowWorkshopEULA", fn_ugc_show_workshop_eula, ISteamUGC_ShowWorkshopEULAFn);

#undef LOAD_UGC_SYM

	// Optional helpers; Workshop still works without them when an app ID is
	// passed to Steam.initialize().
	if (_load_symbol("SteamAPI_SteamUtils_v011", symbol, true) || _load_symbol("SteamAPI_SteamUtils_v010", symbol, true)) {
		fn_ugc_steam_utils = (SteamAPI_SteamUtilsFn)symbol;
		if (_load_symbol("SteamAPI_ISteamUtils_GetAppID", symbol, true)) {
			fn_utils_get_app_id = (ISteamUtils_GetAppIDFn)symbol;
		}
	}

	return true;
}

void SteamAPILoader::_clear_workshop_symbols() {
	workshop_loaded = false;
	fn_steam_ugc = nullptr;
	fn_ugc_create_query_user_request = nullptr;
	fn_ugc_create_query_all_request_page = nullptr;
	fn_ugc_create_query_all_request_cursor = nullptr;
	fn_ugc_create_query_details_request = nullptr;
	fn_ugc_send_query_request = nullptr;
	fn_ugc_get_query_result = nullptr;
	fn_ugc_get_query_preview_url = nullptr;
	fn_ugc_get_query_metadata = nullptr;
	fn_ugc_get_query_children = nullptr;
	fn_ugc_get_query_statistic = nullptr;
	fn_ugc_get_query_num_additional_previews = nullptr;
	fn_ugc_get_query_additional_preview = nullptr;
	fn_ugc_get_query_num_key_value_tags = nullptr;
	fn_ugc_get_query_key_value_tag = nullptr;
	fn_ugc_release_query_request = nullptr;
	fn_ugc_add_required_tag = nullptr;
	fn_ugc_add_excluded_tag = nullptr;
	fn_ugc_add_required_key_value_tag = nullptr;
	fn_ugc_set_return_only_ids = nullptr;
	fn_ugc_set_return_key_value_tags = nullptr;
	fn_ugc_set_return_long_description = nullptr;
	fn_ugc_set_return_metadata = nullptr;
	fn_ugc_set_return_children = nullptr;
	fn_ugc_set_return_additional_previews = nullptr;
	fn_ugc_set_return_total_only = nullptr;
	fn_ugc_set_match_any_tag = nullptr;
	fn_ugc_set_language = nullptr;
	fn_ugc_set_search_text = nullptr;
	fn_ugc_set_cloud_file_name_filter = nullptr;
	fn_ugc_set_allow_cached_response = nullptr;
	fn_ugc_set_ranked_by_trend_days = nullptr;
	fn_ugc_create_item = nullptr;
	fn_ugc_start_item_update = nullptr;
	fn_ugc_set_item_title = nullptr;
	fn_ugc_set_item_description = nullptr;
	fn_ugc_set_item_update_language = nullptr;
	fn_ugc_set_item_metadata = nullptr;
	fn_ugc_set_item_visibility = nullptr;
	fn_ugc_set_item_tags = nullptr;
	fn_ugc_set_item_content = nullptr;
	fn_ugc_set_item_preview = nullptr;
	fn_ugc_remove_all_item_key_value_tags = nullptr;
	fn_ugc_remove_item_key_value_tags = nullptr;
	fn_ugc_add_item_key_value_tag = nullptr;
	fn_ugc_add_item_preview_file = nullptr;
	fn_ugc_add_item_preview_video = nullptr;
	fn_ugc_remove_item_preview = nullptr;
	fn_ugc_submit_item_update = nullptr;
	fn_ugc_get_item_update_progress = nullptr;
	fn_ugc_set_user_item_vote = nullptr;
	fn_ugc_add_item_to_favorites = nullptr;
	fn_ugc_remove_item_from_favorites = nullptr;
	fn_ugc_subscribe_item = nullptr;
	fn_ugc_unsubscribe_item = nullptr;
	fn_ugc_delete_item = nullptr;
	fn_ugc_get_num_subscribed_items = nullptr;
	fn_ugc_get_subscribed_items = nullptr;
	fn_ugc_get_item_state = nullptr;
	fn_ugc_get_item_install_info = nullptr;
	fn_ugc_get_item_download_info = nullptr;
	fn_ugc_download_item = nullptr;
	fn_ugc_suspend_downloads = nullptr;
	fn_ugc_start_playtime_tracking = nullptr;
	fn_ugc_stop_playtime_tracking = nullptr;
	fn_ugc_stop_playtime_tracking_for_all_items = nullptr;
	fn_ugc_show_workshop_eula = nullptr;
	fn_ugc_steam_utils = nullptr;
	fn_utils_get_app_id = nullptr;
}

namespace {

String _ugc_buffer_to_string(const char *p_buffer, size_t p_capacity) {
	// Steam always null-terminates, but never trust a fixed-size buffer.
	size_t length = 0;
	while (length < p_capacity && p_buffer[length] != '\0') {
		length++;
	}
	return String::utf8(p_buffer, (int)length);
}

} // namespace

SteamAPILoader::ISteamUGCPtr SteamAPILoader::get_steam_ugc() const {
	return fn_steam_ugc ? fn_steam_ugc() : nullptr;
}

uint32_t SteamAPILoader::get_app_id() const {
	if (!fn_ugc_steam_utils || !fn_utils_get_app_id) {
		return 0;
	}
	ISteamUtilsPtr utils = fn_ugc_steam_utils();
	return utils ? fn_utils_get_app_id(utils) : 0;
}

SteamUGCQueryHandle_t SteamAPILoader::ugc_create_query_user_request(ISteamUGCPtr p_ugc, uint32_t p_account_id, int p_list_type, int p_matching_type, int p_sort_order, uint32_t p_creator_app_id, uint32_t p_consumer_app_id, uint32_t p_page) const {
	return fn_ugc_create_query_user_request ? fn_ugc_create_query_user_request(p_ugc, p_account_id, p_list_type, p_matching_type, p_sort_order, p_creator_app_id, p_consumer_app_id, p_page) : STEAM_UGC_QUERY_HANDLE_INVALID;
}

SteamUGCQueryHandle_t SteamAPILoader::ugc_create_query_all_request_page(ISteamUGCPtr p_ugc, int p_query_type, int p_matching_type, uint32_t p_creator_app_id, uint32_t p_consumer_app_id, uint32_t p_page) const {
	return fn_ugc_create_query_all_request_page ? fn_ugc_create_query_all_request_page(p_ugc, p_query_type, p_matching_type, p_creator_app_id, p_consumer_app_id, p_page) : STEAM_UGC_QUERY_HANDLE_INVALID;
}

SteamUGCQueryHandle_t SteamAPILoader::ugc_create_query_all_request_cursor(ISteamUGCPtr p_ugc, int p_query_type, int p_matching_type, uint32_t p_creator_app_id, uint32_t p_consumer_app_id, const char *p_cursor) const {
	return fn_ugc_create_query_all_request_cursor ? fn_ugc_create_query_all_request_cursor(p_ugc, p_query_type, p_matching_type, p_creator_app_id, p_consumer_app_id, p_cursor) : STEAM_UGC_QUERY_HANDLE_INVALID;
}

SteamUGCQueryHandle_t SteamAPILoader::ugc_create_query_details_request(ISteamUGCPtr p_ugc, const Vector<SteamPublishedFileId_t> &p_file_ids) const {
	if (!fn_ugc_create_query_details_request || p_file_ids.is_empty()) {
		return STEAM_UGC_QUERY_HANDLE_INVALID;
	}
	Vector<SteamPublishedFileId_t> ids = p_file_ids;
	return fn_ugc_create_query_details_request(p_ugc, ids.ptrw(), (uint32_t)ids.size());
}

SteamAPICallHandle_t SteamAPILoader::ugc_send_query_request(ISteamUGCPtr p_ugc, SteamUGCQueryHandle_t p_handle) const {
	return fn_ugc_send_query_request ? fn_ugc_send_query_request(p_ugc, p_handle) : STEAM_API_CALL_INVALID;
}

bool SteamAPILoader::ugc_get_query_result(ISteamUGCPtr p_ugc, SteamUGCQueryHandle_t p_handle, uint32_t p_index, SteamUGCDetails &r_details) const {
	memset(&r_details, 0, sizeof(SteamUGCDetails));
	return fn_ugc_get_query_result && fn_ugc_get_query_result(p_ugc, p_handle, p_index, &r_details);
}

String SteamAPILoader::ugc_get_query_preview_url(ISteamUGCPtr p_ugc, SteamUGCQueryHandle_t p_handle, uint32_t p_index) const {
	if (!fn_ugc_get_query_preview_url) {
		return String();
	}
	char buffer[STEAM_UGC_URL_MAX * 4] = {};
	if (!fn_ugc_get_query_preview_url(p_ugc, p_handle, p_index, buffer, sizeof(buffer))) {
		return String();
	}
	return _ugc_buffer_to_string(buffer, sizeof(buffer));
}

String SteamAPILoader::ugc_get_query_metadata(ISteamUGCPtr p_ugc, SteamUGCQueryHandle_t p_handle, uint32_t p_index) const {
	if (!fn_ugc_get_query_metadata) {
		return String();
	}
	Vector<char> buffer;
	buffer.resize(STEAM_UGC_DEVELOPER_METADATA_MAX + 1);
	memset(buffer.ptrw(), 0, buffer.size());
	if (!fn_ugc_get_query_metadata(p_ugc, p_handle, p_index, buffer.ptrw(), (uint32_t)buffer.size())) {
		return String();
	}
	return _ugc_buffer_to_string(buffer.ptr(), buffer.size());
}

Vector<SteamPublishedFileId_t> SteamAPILoader::ugc_get_query_children(ISteamUGCPtr p_ugc, SteamUGCQueryHandle_t p_handle, uint32_t p_index, uint32_t p_num_children) const {
	Vector<SteamPublishedFileId_t> children;
	if (!fn_ugc_get_query_children || p_num_children == 0) {
		return children;
	}
	children.resize(p_num_children);
	if (!fn_ugc_get_query_children(p_ugc, p_handle, p_index, children.ptrw(), p_num_children)) {
		children.clear();
	}
	return children;
}

bool SteamAPILoader::ugc_get_query_statistic(ISteamUGCPtr p_ugc, SteamUGCQueryHandle_t p_handle, uint32_t p_index, int p_stat_type, uint64_t &r_value) const {
	r_value = 0;
	return fn_ugc_get_query_statistic && fn_ugc_get_query_statistic(p_ugc, p_handle, p_index, p_stat_type, &r_value);
}

uint32_t SteamAPILoader::ugc_get_query_num_additional_previews(ISteamUGCPtr p_ugc, SteamUGCQueryHandle_t p_handle, uint32_t p_index) const {
	return fn_ugc_get_query_num_additional_previews ? fn_ugc_get_query_num_additional_previews(p_ugc, p_handle, p_index) : 0;
}

bool SteamAPILoader::ugc_get_query_additional_preview(ISteamUGCPtr p_ugc, SteamUGCQueryHandle_t p_handle, uint32_t p_index, uint32_t p_preview_index, String &r_url_or_video_id, String &r_original_file_name, int &r_preview_type) const {
	if (!fn_ugc_get_query_additional_preview) {
		return false;
	}
	char url[STEAM_UGC_URL_MAX * 4] = {};
	char file_name[STEAM_UGC_FILENAME_MAX] = {};
	int preview_type = 0;
	if (!fn_ugc_get_query_additional_preview(p_ugc, p_handle, p_index, p_preview_index, url, sizeof(url), file_name, sizeof(file_name), &preview_type)) {
		return false;
	}
	r_url_or_video_id = _ugc_buffer_to_string(url, sizeof(url));
	r_original_file_name = _ugc_buffer_to_string(file_name, sizeof(file_name));
	r_preview_type = preview_type;
	return true;
}

uint32_t SteamAPILoader::ugc_get_query_num_key_value_tags(ISteamUGCPtr p_ugc, SteamUGCQueryHandle_t p_handle, uint32_t p_index) const {
	return fn_ugc_get_query_num_key_value_tags ? fn_ugc_get_query_num_key_value_tags(p_ugc, p_handle, p_index) : 0;
}

bool SteamAPILoader::ugc_get_query_key_value_tag(ISteamUGCPtr p_ugc, SteamUGCQueryHandle_t p_handle, uint32_t p_index, uint32_t p_tag_index, String &r_key, String &r_value) const {
	if (!fn_ugc_get_query_key_value_tag) {
		return false;
	}
	// Key-value tags are limited to 255 characters each.
	char key[256] = {};
	char value[256] = {};
	if (!fn_ugc_get_query_key_value_tag(p_ugc, p_handle, p_index, p_tag_index, key, sizeof(key), value, sizeof(value))) {
		return false;
	}
	r_key = _ugc_buffer_to_string(key, sizeof(key));
	r_value = _ugc_buffer_to_string(value, sizeof(value));
	return true;
}

bool SteamAPILoader::ugc_release_query_request(ISteamUGCPtr p_ugc, SteamUGCQueryHandle_t p_handle) const {
	return fn_ugc_release_query_request && fn_ugc_release_query_request(p_ugc, p_handle);
}

bool SteamAPILoader::ugc_add_required_tag(ISteamUGCPtr p_ugc, SteamUGCQueryHandle_t p_handle, const char *p_tag) const {
	return fn_ugc_add_required_tag && fn_ugc_add_required_tag(p_ugc, p_handle, p_tag);
}

bool SteamAPILoader::ugc_add_excluded_tag(ISteamUGCPtr p_ugc, SteamUGCQueryHandle_t p_handle, const char *p_tag) const {
	return fn_ugc_add_excluded_tag && fn_ugc_add_excluded_tag(p_ugc, p_handle, p_tag);
}

bool SteamAPILoader::ugc_add_required_key_value_tag(ISteamUGCPtr p_ugc, SteamUGCQueryHandle_t p_handle, const char *p_key, const char *p_value) const {
	return fn_ugc_add_required_key_value_tag && fn_ugc_add_required_key_value_tag(p_ugc, p_handle, p_key, p_value);
}

bool SteamAPILoader::ugc_set_return_only_ids(ISteamUGCPtr p_ugc, SteamUGCQueryHandle_t p_handle, bool p_value) const {
	return fn_ugc_set_return_only_ids && fn_ugc_set_return_only_ids(p_ugc, p_handle, p_value);
}

bool SteamAPILoader::ugc_set_return_key_value_tags(ISteamUGCPtr p_ugc, SteamUGCQueryHandle_t p_handle, bool p_value) const {
	return fn_ugc_set_return_key_value_tags && fn_ugc_set_return_key_value_tags(p_ugc, p_handle, p_value);
}

bool SteamAPILoader::ugc_set_return_long_description(ISteamUGCPtr p_ugc, SteamUGCQueryHandle_t p_handle, bool p_value) const {
	return fn_ugc_set_return_long_description && fn_ugc_set_return_long_description(p_ugc, p_handle, p_value);
}

bool SteamAPILoader::ugc_set_return_metadata(ISteamUGCPtr p_ugc, SteamUGCQueryHandle_t p_handle, bool p_value) const {
	return fn_ugc_set_return_metadata && fn_ugc_set_return_metadata(p_ugc, p_handle, p_value);
}

bool SteamAPILoader::ugc_set_return_children(ISteamUGCPtr p_ugc, SteamUGCQueryHandle_t p_handle, bool p_value) const {
	return fn_ugc_set_return_children && fn_ugc_set_return_children(p_ugc, p_handle, p_value);
}

bool SteamAPILoader::ugc_set_return_additional_previews(ISteamUGCPtr p_ugc, SteamUGCQueryHandle_t p_handle, bool p_value) const {
	return fn_ugc_set_return_additional_previews && fn_ugc_set_return_additional_previews(p_ugc, p_handle, p_value);
}

bool SteamAPILoader::ugc_set_return_total_only(ISteamUGCPtr p_ugc, SteamUGCQueryHandle_t p_handle, bool p_value) const {
	return fn_ugc_set_return_total_only && fn_ugc_set_return_total_only(p_ugc, p_handle, p_value);
}

bool SteamAPILoader::ugc_set_match_any_tag(ISteamUGCPtr p_ugc, SteamUGCQueryHandle_t p_handle, bool p_value) const {
	return fn_ugc_set_match_any_tag && fn_ugc_set_match_any_tag(p_ugc, p_handle, p_value);
}

bool SteamAPILoader::ugc_set_language(ISteamUGCPtr p_ugc, SteamUGCQueryHandle_t p_handle, const char *p_language) const {
	return fn_ugc_set_language && fn_ugc_set_language(p_ugc, p_handle, p_language);
}

bool SteamAPILoader::ugc_set_search_text(ISteamUGCPtr p_ugc, SteamUGCQueryHandle_t p_handle, const char *p_text) const {
	return fn_ugc_set_search_text && fn_ugc_set_search_text(p_ugc, p_handle, p_text);
}

bool SteamAPILoader::ugc_set_cloud_file_name_filter(ISteamUGCPtr p_ugc, SteamUGCQueryHandle_t p_handle, const char *p_file_name) const {
	return fn_ugc_set_cloud_file_name_filter && fn_ugc_set_cloud_file_name_filter(p_ugc, p_handle, p_file_name);
}

bool SteamAPILoader::ugc_set_allow_cached_response(ISteamUGCPtr p_ugc, SteamUGCQueryHandle_t p_handle, uint32_t p_max_age_seconds) const {
	return fn_ugc_set_allow_cached_response && fn_ugc_set_allow_cached_response(p_ugc, p_handle, p_max_age_seconds);
}

bool SteamAPILoader::ugc_set_ranked_by_trend_days(ISteamUGCPtr p_ugc, SteamUGCQueryHandle_t p_handle, uint32_t p_days) const {
	return fn_ugc_set_ranked_by_trend_days && fn_ugc_set_ranked_by_trend_days(p_ugc, p_handle, p_days);
}

SteamAPICallHandle_t SteamAPILoader::ugc_create_item(ISteamUGCPtr p_ugc, uint32_t p_consumer_app_id, int p_file_type) const {
	return fn_ugc_create_item ? fn_ugc_create_item(p_ugc, p_consumer_app_id, p_file_type) : STEAM_API_CALL_INVALID;
}

SteamUGCUpdateHandle_t SteamAPILoader::ugc_start_item_update(ISteamUGCPtr p_ugc, uint32_t p_consumer_app_id, SteamPublishedFileId_t p_file_id) const {
	return fn_ugc_start_item_update ? fn_ugc_start_item_update(p_ugc, p_consumer_app_id, p_file_id) : STEAM_UGC_UPDATE_HANDLE_INVALID;
}

bool SteamAPILoader::ugc_set_item_title(ISteamUGCPtr p_ugc, SteamUGCUpdateHandle_t p_handle, const char *p_title) const {
	return fn_ugc_set_item_title && fn_ugc_set_item_title(p_ugc, p_handle, p_title);
}

bool SteamAPILoader::ugc_set_item_description(ISteamUGCPtr p_ugc, SteamUGCUpdateHandle_t p_handle, const char *p_description) const {
	return fn_ugc_set_item_description && fn_ugc_set_item_description(p_ugc, p_handle, p_description);
}

bool SteamAPILoader::ugc_set_item_update_language(ISteamUGCPtr p_ugc, SteamUGCUpdateHandle_t p_handle, const char *p_language) const {
	return fn_ugc_set_item_update_language && fn_ugc_set_item_update_language(p_ugc, p_handle, p_language);
}

bool SteamAPILoader::ugc_set_item_metadata(ISteamUGCPtr p_ugc, SteamUGCUpdateHandle_t p_handle, const char *p_metadata) const {
	return fn_ugc_set_item_metadata && fn_ugc_set_item_metadata(p_ugc, p_handle, p_metadata);
}

bool SteamAPILoader::ugc_set_item_visibility(ISteamUGCPtr p_ugc, SteamUGCUpdateHandle_t p_handle, int p_visibility) const {
	return fn_ugc_set_item_visibility && fn_ugc_set_item_visibility(p_ugc, p_handle, p_visibility);
}

bool SteamAPILoader::ugc_set_item_tags(ISteamUGCPtr p_ugc, SteamUGCUpdateHandle_t p_handle, const Vector<String> &p_tags, bool p_allow_admin_tags) const {
	if (!fn_ugc_set_item_tags) {
		return false;
	}
	// Keep the UTF-8 buffers alive for the duration of the call.
	Vector<CharString> utf8_tags;
	Vector<const char *> tag_ptrs;
	utf8_tags.resize(p_tags.size());
	tag_ptrs.resize(p_tags.size());
	for (int i = 0; i < p_tags.size(); i++) {
		utf8_tags.write[i] = p_tags[i].utf8();
		tag_ptrs.write[i] = utf8_tags[i].get_data();
	}
	SteamParamStringArray tags;
	tags.m_ppStrings = tag_ptrs.is_empty() ? nullptr : tag_ptrs.ptrw();
	tags.m_nNumStrings = (int32_t)tag_ptrs.size();
	return fn_ugc_set_item_tags(p_ugc, p_handle, &tags, p_allow_admin_tags);
}

bool SteamAPILoader::ugc_set_item_content(ISteamUGCPtr p_ugc, SteamUGCUpdateHandle_t p_handle, const char *p_content_folder) const {
	return fn_ugc_set_item_content && fn_ugc_set_item_content(p_ugc, p_handle, p_content_folder);
}

bool SteamAPILoader::ugc_set_item_preview(ISteamUGCPtr p_ugc, SteamUGCUpdateHandle_t p_handle, const char *p_preview_file) const {
	return fn_ugc_set_item_preview && fn_ugc_set_item_preview(p_ugc, p_handle, p_preview_file);
}

bool SteamAPILoader::ugc_remove_all_item_key_value_tags(ISteamUGCPtr p_ugc, SteamUGCUpdateHandle_t p_handle) const {
	return fn_ugc_remove_all_item_key_value_tags && fn_ugc_remove_all_item_key_value_tags(p_ugc, p_handle);
}

bool SteamAPILoader::ugc_remove_item_key_value_tags(ISteamUGCPtr p_ugc, SteamUGCUpdateHandle_t p_handle, const char *p_key) const {
	return fn_ugc_remove_item_key_value_tags && fn_ugc_remove_item_key_value_tags(p_ugc, p_handle, p_key);
}

bool SteamAPILoader::ugc_add_item_key_value_tag(ISteamUGCPtr p_ugc, SteamUGCUpdateHandle_t p_handle, const char *p_key, const char *p_value) const {
	return fn_ugc_add_item_key_value_tag && fn_ugc_add_item_key_value_tag(p_ugc, p_handle, p_key, p_value);
}

bool SteamAPILoader::ugc_add_item_preview_file(ISteamUGCPtr p_ugc, SteamUGCUpdateHandle_t p_handle, const char *p_preview_file, int p_type) const {
	return fn_ugc_add_item_preview_file && fn_ugc_add_item_preview_file(p_ugc, p_handle, p_preview_file, p_type);
}

bool SteamAPILoader::ugc_add_item_preview_video(ISteamUGCPtr p_ugc, SteamUGCUpdateHandle_t p_handle, const char *p_video_id) const {
	return fn_ugc_add_item_preview_video && fn_ugc_add_item_preview_video(p_ugc, p_handle, p_video_id);
}

bool SteamAPILoader::ugc_remove_item_preview(ISteamUGCPtr p_ugc, SteamUGCUpdateHandle_t p_handle, uint32_t p_index) const {
	return fn_ugc_remove_item_preview && fn_ugc_remove_item_preview(p_ugc, p_handle, (int)p_index);
}

SteamAPICallHandle_t SteamAPILoader::ugc_submit_item_update(ISteamUGCPtr p_ugc, SteamUGCUpdateHandle_t p_handle, const char *p_change_note) const {
	return fn_ugc_submit_item_update ? fn_ugc_submit_item_update(p_ugc, p_handle, p_change_note) : STEAM_API_CALL_INVALID;
}

int SteamAPILoader::ugc_get_item_update_progress(ISteamUGCPtr p_ugc, SteamUGCUpdateHandle_t p_handle, uint64_t &r_bytes_processed, uint64_t &r_bytes_total) const {
	r_bytes_processed = 0;
	r_bytes_total = 0;
	return fn_ugc_get_item_update_progress ? fn_ugc_get_item_update_progress(p_ugc, p_handle, &r_bytes_processed, &r_bytes_total) : 0;
}

SteamAPICallHandle_t SteamAPILoader::ugc_set_user_item_vote(ISteamUGCPtr p_ugc, SteamPublishedFileId_t p_file_id, bool p_vote_up) const {
	return fn_ugc_set_user_item_vote ? fn_ugc_set_user_item_vote(p_ugc, p_file_id, p_vote_up) : STEAM_API_CALL_INVALID;
}

SteamAPICallHandle_t SteamAPILoader::ugc_add_item_to_favorites(ISteamUGCPtr p_ugc, uint32_t p_app_id, SteamPublishedFileId_t p_file_id) const {
	return fn_ugc_add_item_to_favorites ? fn_ugc_add_item_to_favorites(p_ugc, p_app_id, p_file_id) : STEAM_API_CALL_INVALID;
}

SteamAPICallHandle_t SteamAPILoader::ugc_remove_item_from_favorites(ISteamUGCPtr p_ugc, uint32_t p_app_id, SteamPublishedFileId_t p_file_id) const {
	return fn_ugc_remove_item_from_favorites ? fn_ugc_remove_item_from_favorites(p_ugc, p_app_id, p_file_id) : STEAM_API_CALL_INVALID;
}

SteamAPICallHandle_t SteamAPILoader::ugc_subscribe_item(ISteamUGCPtr p_ugc, SteamPublishedFileId_t p_file_id) const {
	return fn_ugc_subscribe_item ? fn_ugc_subscribe_item(p_ugc, p_file_id) : STEAM_API_CALL_INVALID;
}

SteamAPICallHandle_t SteamAPILoader::ugc_unsubscribe_item(ISteamUGCPtr p_ugc, SteamPublishedFileId_t p_file_id) const {
	return fn_ugc_unsubscribe_item ? fn_ugc_unsubscribe_item(p_ugc, p_file_id) : STEAM_API_CALL_INVALID;
}

SteamAPICallHandle_t SteamAPILoader::ugc_delete_item(ISteamUGCPtr p_ugc, SteamPublishedFileId_t p_file_id) const {
	return fn_ugc_delete_item ? fn_ugc_delete_item(p_ugc, p_file_id) : STEAM_API_CALL_INVALID;
}

uint32_t SteamAPILoader::ugc_get_num_subscribed_items(ISteamUGCPtr p_ugc, bool p_include_locally_disabled) const {
	return fn_ugc_get_num_subscribed_items ? fn_ugc_get_num_subscribed_items(p_ugc, p_include_locally_disabled) : 0;
}

Vector<SteamPublishedFileId_t> SteamAPILoader::ugc_get_subscribed_items(ISteamUGCPtr p_ugc, bool p_include_locally_disabled) const {
	Vector<SteamPublishedFileId_t> items;
	if (!fn_ugc_get_subscribed_items) {
		return items;
	}
	const uint32_t count = ugc_get_num_subscribed_items(p_ugc, p_include_locally_disabled);
	if (count == 0) {
		return items;
	}
	items.resize(count);
	const uint32_t written = fn_ugc_get_subscribed_items(p_ugc, items.ptrw(), count, p_include_locally_disabled);
	items.resize(MIN(written, count));
	return items;
}

uint32_t SteamAPILoader::ugc_get_item_state(ISteamUGCPtr p_ugc, SteamPublishedFileId_t p_file_id) const {
	return fn_ugc_get_item_state ? fn_ugc_get_item_state(p_ugc, p_file_id) : 0;
}

bool SteamAPILoader::ugc_get_item_install_info(ISteamUGCPtr p_ugc, SteamPublishedFileId_t p_file_id, uint64_t &r_size_on_disk, String &r_folder, uint32_t &r_timestamp) const {
	r_size_on_disk = 0;
	r_timestamp = 0;
	r_folder = String();
	if (!fn_ugc_get_item_install_info) {
		return false;
	}
	char folder[4096] = {};
	if (!fn_ugc_get_item_install_info(p_ugc, p_file_id, &r_size_on_disk, folder, sizeof(folder), &r_timestamp)) {
		return false;
	}
	r_folder = _ugc_buffer_to_string(folder, sizeof(folder));
	return true;
}

bool SteamAPILoader::ugc_get_item_download_info(ISteamUGCPtr p_ugc, SteamPublishedFileId_t p_file_id, uint64_t &r_bytes_downloaded, uint64_t &r_bytes_total) const {
	r_bytes_downloaded = 0;
	r_bytes_total = 0;
	return fn_ugc_get_item_download_info && fn_ugc_get_item_download_info(p_ugc, p_file_id, &r_bytes_downloaded, &r_bytes_total);
}

bool SteamAPILoader::ugc_download_item(ISteamUGCPtr p_ugc, SteamPublishedFileId_t p_file_id, bool p_high_priority) const {
	return fn_ugc_download_item && fn_ugc_download_item(p_ugc, p_file_id, p_high_priority);
}

void SteamAPILoader::ugc_suspend_downloads(ISteamUGCPtr p_ugc, bool p_suspend) const {
	if (fn_ugc_suspend_downloads) {
		fn_ugc_suspend_downloads(p_ugc, p_suspend);
	}
}

SteamAPICallHandle_t SteamAPILoader::ugc_start_playtime_tracking(ISteamUGCPtr p_ugc, const Vector<SteamPublishedFileId_t> &p_file_ids) const {
	if (!fn_ugc_start_playtime_tracking || p_file_ids.is_empty()) {
		return STEAM_API_CALL_INVALID;
	}
	Vector<SteamPublishedFileId_t> ids = p_file_ids;
	return fn_ugc_start_playtime_tracking(p_ugc, ids.ptrw(), (uint32_t)ids.size());
}

SteamAPICallHandle_t SteamAPILoader::ugc_stop_playtime_tracking(ISteamUGCPtr p_ugc, const Vector<SteamPublishedFileId_t> &p_file_ids) const {
	if (!fn_ugc_stop_playtime_tracking || p_file_ids.is_empty()) {
		return STEAM_API_CALL_INVALID;
	}
	Vector<SteamPublishedFileId_t> ids = p_file_ids;
	return fn_ugc_stop_playtime_tracking(p_ugc, ids.ptrw(), (uint32_t)ids.size());
}

SteamAPICallHandle_t SteamAPILoader::ugc_stop_playtime_tracking_for_all_items(ISteamUGCPtr p_ugc) const {
	return fn_ugc_stop_playtime_tracking_for_all_items ? fn_ugc_stop_playtime_tracking_for_all_items(p_ugc) : STEAM_API_CALL_INVALID;
}

bool SteamAPILoader::ugc_show_workshop_eula(ISteamUGCPtr p_ugc) const {
	return fn_ugc_show_workshop_eula && fn_ugc_show_workshop_eula(p_ugc);
}
