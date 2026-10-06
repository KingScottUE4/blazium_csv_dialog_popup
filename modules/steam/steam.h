/**************************************************************************/
/*  steam.h                                                               */
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

#include "steam_achievement_info.h"
#include "steam_api_loader.h"
#include "steam_auth_result.h"
#include "steam_inventory_item.h"
#include "steam_item_definition.h"
#include "steam_workshop_item.h"

#include "core/object/object.h"
#include "core/variant/type_info.h"

class SteamAuthClient;

class Steam : public Object {
	GDCLASS(Steam, Object);

public:
	enum TicketState {
		TICKET_STATE_IDLE,
		TICKET_STATE_PENDING,
		TICKET_STATE_READY,
		TICKET_STATE_FAILED,
	};

	enum AvatarSize {
		AVATAR_SIZE_SMALL,
		AVATAR_SIZE_MEDIUM,
		AVATAR_SIZE_LARGE,
	};

	// Values mirror the Steamworks SDK enums so they can be passed through.
	enum WorkshopFileType {
		WORKSHOP_FILE_TYPE_COMMUNITY = 0,
		WORKSHOP_FILE_TYPE_MICROTRANSACTION = 1,
		WORKSHOP_FILE_TYPE_COLLECTION = 2,
		WORKSHOP_FILE_TYPE_ART = 3,
		WORKSHOP_FILE_TYPE_VIDEO = 4,
		WORKSHOP_FILE_TYPE_SCREENSHOT = 5,
		WORKSHOP_FILE_TYPE_WEB_GUIDE = 9,
		WORKSHOP_FILE_TYPE_INTEGRATED_GUIDE = 10,
		WORKSHOP_FILE_TYPE_MERCH = 11,
		WORKSHOP_FILE_TYPE_CONTROLLER_BINDING = 12,
		WORKSHOP_FILE_TYPE_STEAM_VIDEO = 14,
		WORKSHOP_FILE_TYPE_GAME_MANAGED_ITEM = 15,
	};

	enum WorkshopVisibility {
		WORKSHOP_VISIBILITY_PUBLIC = 0,
		WORKSHOP_VISIBILITY_FRIENDS_ONLY = 1,
		WORKSHOP_VISIBILITY_PRIVATE = 2,
		WORKSHOP_VISIBILITY_UNLISTED = 3,
	};

	enum WorkshopItemState {
		WORKSHOP_ITEM_STATE_NONE = 0,
		WORKSHOP_ITEM_STATE_SUBSCRIBED = 1,
		WORKSHOP_ITEM_STATE_LEGACY_ITEM = 2,
		WORKSHOP_ITEM_STATE_INSTALLED = 4,
		WORKSHOP_ITEM_STATE_NEEDS_UPDATE = 8,
		WORKSHOP_ITEM_STATE_DOWNLOADING = 16,
		WORKSHOP_ITEM_STATE_DOWNLOAD_PENDING = 32,
		WORKSHOP_ITEM_STATE_DISABLED_LOCALLY = 64,
	};

	enum WorkshopQueryType {
		WORKSHOP_QUERY_RANKED_BY_VOTE = 0,
		WORKSHOP_QUERY_RANKED_BY_PUBLICATION_DATE = 1,
		WORKSHOP_QUERY_ACCEPTED_FOR_GAME_RANKED_BY_ACCEPTANCE_DATE = 2,
		WORKSHOP_QUERY_RANKED_BY_TREND = 3,
		WORKSHOP_QUERY_FAVORITED_BY_FRIENDS_RANKED_BY_PUBLICATION_DATE = 4,
		WORKSHOP_QUERY_CREATED_BY_FRIENDS_RANKED_BY_PUBLICATION_DATE = 5,
		WORKSHOP_QUERY_RANKED_BY_NUM_TIMES_REPORTED = 6,
		WORKSHOP_QUERY_CREATED_BY_FOLLOWED_USERS_RANKED_BY_PUBLICATION_DATE = 7,
		WORKSHOP_QUERY_NOT_YET_RATED = 8,
		WORKSHOP_QUERY_RANKED_BY_TOTAL_VOTES_ASC = 9,
		WORKSHOP_QUERY_RANKED_BY_VOTES_UP = 10,
		WORKSHOP_QUERY_RANKED_BY_TEXT_SEARCH = 11,
		WORKSHOP_QUERY_RANKED_BY_TOTAL_UNIQUE_SUBSCRIPTIONS = 12,
		WORKSHOP_QUERY_RANKED_BY_PLAYTIME_TREND = 13,
		WORKSHOP_QUERY_RANKED_BY_TOTAL_PLAYTIME = 14,
		WORKSHOP_QUERY_RANKED_BY_AVERAGE_PLAYTIME_TREND = 15,
		WORKSHOP_QUERY_RANKED_BY_LIFETIME_AVERAGE_PLAYTIME = 16,
		WORKSHOP_QUERY_RANKED_BY_PLAYTIME_SESSIONS_TREND = 17,
		WORKSHOP_QUERY_RANKED_BY_LIFETIME_PLAYTIME_SESSIONS = 18,
		WORKSHOP_QUERY_RANKED_BY_LAST_UPDATED_DATE = 19,
	};

	enum WorkshopMatchingType {
		WORKSHOP_MATCHING_ITEMS = 0,
		WORKSHOP_MATCHING_ITEMS_MTX = 1,
		WORKSHOP_MATCHING_ITEMS_READY_TO_USE = 2,
		WORKSHOP_MATCHING_COLLECTIONS = 3,
		WORKSHOP_MATCHING_ARTWORK = 4,
		WORKSHOP_MATCHING_VIDEOS = 5,
		WORKSHOP_MATCHING_SCREENSHOTS = 6,
		WORKSHOP_MATCHING_ALL_GUIDES = 7,
		WORKSHOP_MATCHING_WEB_GUIDES = 8,
		WORKSHOP_MATCHING_INTEGRATED_GUIDES = 9,
		WORKSHOP_MATCHING_USABLE_IN_GAME = 10,
		WORKSHOP_MATCHING_CONTROLLER_BINDINGS = 11,
		WORKSHOP_MATCHING_GAME_MANAGED_ITEMS = 12,
		WORKSHOP_MATCHING_ALL = -1,
	};

	enum WorkshopUserList {
		WORKSHOP_USER_LIST_PUBLISHED = 0,
		WORKSHOP_USER_LIST_VOTED_ON = 1,
		WORKSHOP_USER_LIST_VOTED_UP = 2,
		WORKSHOP_USER_LIST_VOTED_DOWN = 3,
		WORKSHOP_USER_LIST_WILL_VOTE_LATER = 4,
		WORKSHOP_USER_LIST_FAVORITED = 5,
		WORKSHOP_USER_LIST_SUBSCRIBED = 6,
		WORKSHOP_USER_LIST_USED_OR_PLAYED = 7,
		WORKSHOP_USER_LIST_FOLLOWED = 8,
	};

	enum WorkshopUserListSortOrder {
		WORKSHOP_SORT_CREATION_ORDER_DESC = 0,
		WORKSHOP_SORT_CREATION_ORDER_ASC = 1,
		WORKSHOP_SORT_TITLE_ASC = 2,
		WORKSHOP_SORT_LAST_UPDATED_DESC = 3,
		WORKSHOP_SORT_SUBSCRIPTION_DATE_DESC = 4,
		WORKSHOP_SORT_VOTE_SCORE_DESC = 5,
		WORKSHOP_SORT_FOR_MODERATION = 6,
	};

	enum WorkshopUpdateStatus {
		WORKSHOP_UPDATE_STATUS_INVALID = 0,
		WORKSHOP_UPDATE_STATUS_PREPARING_CONFIG = 1,
		WORKSHOP_UPDATE_STATUS_PREPARING_CONTENT = 2,
		WORKSHOP_UPDATE_STATUS_UPLOADING_CONTENT = 3,
		WORKSHOP_UPDATE_STATUS_UPLOADING_PREVIEW_FILE = 4,
		WORKSHOP_UPDATE_STATUS_COMMITTING_CHANGES = 5,
	};

	enum WorkshopPreviewType {
		WORKSHOP_PREVIEW_TYPE_IMAGE = 0,
		WORKSHOP_PREVIEW_TYPE_YOUTUBE_VIDEO = 1,
		WORKSHOP_PREVIEW_TYPE_SKETCHFAB = 2,
		WORKSHOP_PREVIEW_TYPE_ENVIRONMENT_MAP_HORIZONTAL_CROSS = 3,
		WORKSHOP_PREVIEW_TYPE_ENVIRONMENT_MAP_LAT_LONG = 4,
		WORKSHOP_PREVIEW_TYPE_CLIP = 5,
	};

private:
	static Steam *singleton;

	SteamAPILoader loader;
	SteamAuthClient *auth_client = nullptr;

	bool dll_available = false;
	bool initialized = false;
	bool debug_logging = false;
	int app_id = 0;
	SteamAPILoader::HSteamPipe steam_pipe = 0;
	SteamAPILoader::ISteamUserPtr steam_user = nullptr;
	SteamAPILoader::ISteamUserStatsPtr steam_user_stats = nullptr;
	SteamAPILoader::ISteamFriendsPtr steam_friends = nullptr;
	SteamAPILoader::ISteamUtilsPtr steam_utils = nullptr;
	SteamAPILoader::ISteamInventoryPtr steam_inventory = nullptr;
	SteamAPILoader::ISteamUGCPtr steam_ugc = nullptr;

	TicketState ticket_state = TICKET_STATE_IDLE;
	SteamAPILoader::HAuthTicket pending_auth_ticket = 0;
	String pending_hex_ticket;
	String pending_ticket_error;

	bool stats_received = false;
	bool stats_store_pending = false;
	bool stats_store_succeeded = false;

	bool inventory_definitions_loaded = false;
	bool inventory_definitions_pending = false;
	bool dispatching_callbacks = false;
	bool manual_dispatch_enabled = false;
	HashMap<int, int> inventory_pending_results;

	SteamInventoryUpdateHandle_t inventory_property_update_handle = STEAM_INVENTORY_UPDATE_HANDLE_INVALID;

	// Steam Workshop async call tracking, keyed by SteamAPICall_t.
	struct WorkshopPendingCall {
		int callback_id = 0;
		uint64_t published_file_id = 0;
		uint64_t query_handle = 0;
		bool flag = false; // vote_up / favorite add / playtime start, depending on the call.
	};
	enum WorkshopQueryReturnFlags {
		WORKSHOP_QUERY_RETURN_METADATA = 1 << 0,
		WORKSHOP_QUERY_RETURN_KEY_VALUE_TAGS = 1 << 1,
		WORKSHOP_QUERY_RETURN_CHILDREN = 1 << 2,
		WORKSHOP_QUERY_RETURN_ADDITIONAL_PREVIEWS = 1 << 3,
	};
	HashMap<uint64_t, WorkshopPendingCall> workshop_pending_calls;
	HashMap<uint64_t, uint32_t> workshop_query_return_flags;

	Vector<String> debug_log;
	static const int kMaxDebugLogEntries = 256;

	void _log_debug(const String &p_message);
	void _dispatch_callbacks();
	void _handle_callback(int p_callback_id, const void *p_data, int p_size);
	void _reset_ticket_state();
	bool _ensure_stats_ready() const;
	bool _wait_for_user_stats(double p_timeout_sec, bool p_require_fresh);
	bool _probe_stats_loaded();
	bool _get_stat_int(const String &p_name, int &r_value) const;
	Ref<SteamAchievementInfo> _build_achievement_info(const String &p_name, bool p_include_icon) const;
	Ref<Image> _image_from_steam_handle(int p_image) const;
	bool _ensure_inventory_ready() const;
	bool _wait_for_inventory_result(SteamInventoryResult_t p_result, double p_timeout_sec);
	Array _parse_inventory_result(SteamInventoryResult_t p_result);
	Ref<SteamInventoryItem> _build_inventory_item(const SteamItemDetails &p_details, SteamInventoryResult_t p_result, uint32_t p_item_index) const;
	Ref<SteamItemDefinition> _build_item_definition(int p_def_id) const;
	Ref<Image> _fetch_image_from_url(const String &p_url) const;
	bool _wait_for_inventory_definitions(double p_timeout_sec);

	static void _bind_workshop_methods();
	bool _ensure_workshop_ready() const;
	uint32_t _get_workshop_app_id() const;
	bool _track_workshop_call(SteamAPICallHandle_t p_call, int p_callback_id, uint64_t p_published_file_id = 0, uint64_t p_query_handle = 0, bool p_flag = false);
	bool _handle_workshop_callback(int p_callback_id, const void *p_data, int p_size);
	void _handle_workshop_call_failure(const WorkshopPendingCall &p_call);
	int64_t _send_workshop_query(SteamUGCQueryHandle_t p_handle, const Dictionary &p_options);
	Array _parse_workshop_query_results(SteamUGCQueryHandle_t p_handle, uint32_t p_num_results);
	Ref<SteamWorkshopItem> _build_workshop_item(SteamUGCQueryHandle_t p_handle, uint32_t p_index, uint32_t p_return_flags) const;
	void _clear_workshop_state();

protected:
	static void _bind_methods();

public:
	static Steam *get_singleton();

	bool is_available();
	bool has_inventory_support() const;
	Error initialize(int p_app_id = 0);
	void shutdown();
	bool is_initialized() const { return initialized; }

	void set_debug_logging(bool p_enabled);
	bool is_debug_logging_enabled() const { return debug_logging; }
	Array get_debug_log() const;
	void clear_debug_log();

	void request_web_api_ticket(const String &p_identity = "blazium");
	void poll_callbacks();
	TicketState get_ticket_state() const { return ticket_state; }
	String get_pending_hex_ticket() const { return pending_hex_ticket; }
	int get_pending_auth_ticket_handle() const { return pending_auth_ticket; }
	String get_pending_ticket_error() const { return pending_ticket_error; }

	Ref<SteamAuthResult> authenticate_with_server(const String &p_url, const String &p_ticket, int p_app_id);
	void cancel_auth_ticket(int p_handle);

	uint64_t get_local_steam_id() const;
	bool is_logged_on() const;

	bool request_current_stats();
	bool refresh_current_stats();
	bool store_stats();
	bool set_achievement(const String &p_name);
	bool clear_achievement(const String &p_name);
	bool get_achievement(const String &p_name) const;
	Ref<SteamAchievementInfo> get_achievement_info(const String &p_name, bool p_include_icon = true);
	Array get_all_achievements(bool p_include_icons = false);

	int get_stat_int(const String &p_name) const;
	float get_stat_float(const String &p_name) const;
	bool set_stat_int(const String &p_name, int p_value);
	bool set_stat_float(const String &p_name, float p_value);
	bool increment_stat_int(const String &p_name, int p_delta);
	bool clear_stat(const String &p_name);

	String get_persona_name() const;
	Ref<Image> get_avatar_image(uint64_t p_steam_id = 0, AvatarSize p_size = AVATAR_SIZE_LARGE);

	bool load_item_definitions();
	bool request_item_definitions(double p_timeout_sec = 15.0);
	PackedInt32Array get_item_definition_ids();
	Ref<SteamItemDefinition> get_item_definition(int p_def_id);
	Array get_all_item_definitions();
	Array get_all_items();
	Array get_items_by_id(const PackedInt64Array &p_instance_ids);
	Array find_items_by_def_id(int p_def_id);
	Ref<Image> get_item_definition_icon(int p_def_id);
	bool consume_item(uint64_t p_instance_id, int p_quantity);
	bool exchange_items(const PackedInt32Array &p_generate_def_ids, const PackedInt32Array &p_generate_quantities, const PackedInt64Array &p_destroy_instance_ids, const PackedInt32Array &p_destroy_quantities);
	bool trigger_item_drop(int p_drop_list_definition);
	bool transfer_item_quantity(uint64_t p_source_instance_id, int p_quantity, uint64_t p_dest_instance_id);
	bool grant_promo_items();
	PackedInt32Array get_eligible_promo_item_definition_ids(uint64_t p_steam_id = 0);
	int get_inventory_result_status(int p_result_handle);
	Array get_inventory_result_items(int p_result_handle);
	bool add_promo_item(int p_def_id);
	bool add_promo_items(const PackedInt32Array &p_def_ids);
	bool start_property_update();
	bool set_property_string(uint64_t p_instance_id, const String &p_property_name, const String &p_value);
	bool set_property_bool(uint64_t p_instance_id, const String &p_property_name, bool p_value);
	bool set_property_int64(uint64_t p_instance_id, const String &p_property_name, int p_value);
	bool set_property_float(uint64_t p_instance_id, const String &p_property_name, float p_value);
	bool remove_property(uint64_t p_instance_id, const String &p_property_name);
	bool submit_property_update();
	PackedByteArray serialize_inventory_result(int p_result_handle);
	int deserialize_inventory(const PackedByteArray &p_bytes, uint64_t p_expected_steam_id = 0);

	// Steam Workshop (ISteamUGC).
	bool has_workshop_support() const;
	int get_num_subscribed_items(bool p_include_locally_disabled = false) const;
	PackedInt64Array get_subscribed_items(bool p_include_locally_disabled = false) const;
	BitField<WorkshopItemState> get_item_state(uint64_t p_published_file_id) const;
	Dictionary get_item_install_info(uint64_t p_published_file_id) const;
	Dictionary get_item_download_info(uint64_t p_published_file_id) const;
	bool download_item(uint64_t p_published_file_id, bool p_high_priority = false);
	void suspend_downloads(bool p_suspend);
	bool subscribe_item(uint64_t p_published_file_id);
	bool unsubscribe_item(uint64_t p_published_file_id);

	int64_t query_workshop_items(WorkshopQueryType p_query_type = WORKSHOP_QUERY_RANKED_BY_VOTE, WorkshopMatchingType p_matching_type = WORKSHOP_MATCHING_ITEMS_READY_TO_USE, int p_page = 1, const Dictionary &p_options = Dictionary());
	int64_t query_user_workshop_items(WorkshopUserList p_list_type = WORKSHOP_USER_LIST_PUBLISHED, WorkshopMatchingType p_matching_type = WORKSHOP_MATCHING_ITEMS_READY_TO_USE, WorkshopUserListSortOrder p_sort_order = WORKSHOP_SORT_CREATION_ORDER_DESC, int p_page = 1, uint64_t p_steam_id = 0, const Dictionary &p_options = Dictionary());
	int64_t query_workshop_item_details(const PackedInt64Array &p_published_file_ids, const Dictionary &p_options = Dictionary());

	bool create_workshop_item(WorkshopFileType p_file_type = WORKSHOP_FILE_TYPE_COMMUNITY);
	int64_t start_item_update(uint64_t p_published_file_id);
	bool set_item_title(uint64_t p_update_handle, const String &p_title);
	bool set_item_description(uint64_t p_update_handle, const String &p_description);
	bool set_item_update_language(uint64_t p_update_handle, const String &p_language);
	bool set_item_metadata(uint64_t p_update_handle, const String &p_metadata);
	bool set_item_visibility(uint64_t p_update_handle, WorkshopVisibility p_visibility);
	bool set_item_tags(uint64_t p_update_handle, const PackedStringArray &p_tags, bool p_allow_admin_tags = false);
	bool set_item_content(uint64_t p_update_handle, const String &p_content_folder);
	bool set_item_preview(uint64_t p_update_handle, const String &p_preview_file);
	bool add_item_key_value_tag(uint64_t p_update_handle, const String &p_key, const String &p_value);
	bool remove_item_key_value_tags(uint64_t p_update_handle, const String &p_key);
	bool remove_all_item_key_value_tags(uint64_t p_update_handle);
	bool add_item_preview_file(uint64_t p_update_handle, const String &p_preview_file, WorkshopPreviewType p_type = WORKSHOP_PREVIEW_TYPE_IMAGE);
	bool add_item_preview_video(uint64_t p_update_handle, const String &p_video_id);
	bool remove_item_preview(uint64_t p_update_handle, int p_index);
	bool submit_item_update(uint64_t p_update_handle, const String &p_change_note = String());
	Dictionary get_item_update_progress(uint64_t p_update_handle) const;
	bool delete_workshop_item(uint64_t p_published_file_id);

	bool set_user_item_vote(uint64_t p_published_file_id, bool p_vote_up);
	bool add_item_to_favorites(uint64_t p_published_file_id);
	bool remove_item_from_favorites(uint64_t p_published_file_id);
	bool start_playtime_tracking(const PackedInt64Array &p_published_file_ids);
	bool stop_playtime_tracking(const PackedInt64Array &p_published_file_ids);
	bool stop_playtime_tracking_for_all_items();
	bool show_workshop_eula();

	Steam();
	~Steam();
};

VARIANT_ENUM_CAST(Steam::TicketState);
VARIANT_ENUM_CAST(Steam::AvatarSize);
VARIANT_ENUM_CAST(Steam::WorkshopFileType);
VARIANT_ENUM_CAST(Steam::WorkshopVisibility);
VARIANT_BITFIELD_CAST(Steam::WorkshopItemState);
VARIANT_ENUM_CAST(Steam::WorkshopQueryType);
VARIANT_ENUM_CAST(Steam::WorkshopMatchingType);
VARIANT_ENUM_CAST(Steam::WorkshopUserList);
VARIANT_ENUM_CAST(Steam::WorkshopUserListSortOrder);
VARIANT_ENUM_CAST(Steam::WorkshopUpdateStatus);
VARIANT_ENUM_CAST(Steam::WorkshopPreviewType);
