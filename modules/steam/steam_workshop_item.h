/**************************************************************************/
/*  steam_workshop_item.h                                                 */
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
#include "core/variant/dictionary.h"

// A Steam Workshop (UGC) item as returned by a Workshop query.
class SteamWorkshopItem : public RefCounted {
	GDCLASS(SteamWorkshopItem, RefCounted);

	uint64_t published_file_id = 0;
	int result = 0;
	int file_type = 0;
	uint32_t creator_app_id = 0;
	uint32_t consumer_app_id = 0;
	String title;
	String description;
	uint64_t owner_steam_id = 0;
	int64_t time_created = 0;
	int64_t time_updated = 0;
	int64_t time_added_to_user_list = 0;
	int visibility = 0;
	bool banned = false;
	bool accepted_for_use = false;
	bool tags_truncated = false;
	PackedStringArray tags;
	String file_name;
	int64_t file_size = 0;
	int64_t preview_file_size = 0;
	uint64_t total_files_size = 0;
	String url;
	String preview_url;
	int votes_up = 0;
	int votes_down = 0;
	float score = 0.0f;
	int num_children = 0;
	PackedInt64Array children;
	String metadata;
	Dictionary key_value_tags;
	Array additional_previews;
	Dictionary statistics;

protected:
	static void _bind_methods();

public:
	void set_published_file_id(uint64_t p_id) { published_file_id = p_id; }
	uint64_t get_published_file_id() const { return published_file_id; }

	void set_result(int p_result) { result = p_result; }
	int get_result() const { return result; }

	void set_file_type(int p_type) { file_type = p_type; }
	int get_file_type() const { return file_type; }

	void set_creator_app_id(uint32_t p_app_id) { creator_app_id = p_app_id; }
	uint32_t get_creator_app_id() const { return creator_app_id; }

	void set_consumer_app_id(uint32_t p_app_id) { consumer_app_id = p_app_id; }
	uint32_t get_consumer_app_id() const { return consumer_app_id; }

	void set_title(const String &p_title) { title = p_title; }
	String get_title() const { return title; }

	void set_description(const String &p_description) { description = p_description; }
	String get_description() const { return description; }

	void set_owner_steam_id(uint64_t p_steam_id) { owner_steam_id = p_steam_id; }
	uint64_t get_owner_steam_id() const { return owner_steam_id; }

	void set_time_created(int64_t p_time) { time_created = p_time; }
	int64_t get_time_created() const { return time_created; }

	void set_time_updated(int64_t p_time) { time_updated = p_time; }
	int64_t get_time_updated() const { return time_updated; }

	void set_time_added_to_user_list(int64_t p_time) { time_added_to_user_list = p_time; }
	int64_t get_time_added_to_user_list() const { return time_added_to_user_list; }

	void set_visibility(int p_visibility) { visibility = p_visibility; }
	int get_visibility() const { return visibility; }

	void set_banned(bool p_banned) { banned = p_banned; }
	bool is_banned() const { return banned; }

	void set_accepted_for_use(bool p_accepted) { accepted_for_use = p_accepted; }
	bool is_accepted_for_use() const { return accepted_for_use; }

	void set_tags_truncated(bool p_truncated) { tags_truncated = p_truncated; }
	bool are_tags_truncated() const { return tags_truncated; }

	void set_tags(const PackedStringArray &p_tags) { tags = p_tags; }
	PackedStringArray get_tags() const { return tags; }
	bool has_tag(const String &p_tag) const;

	void set_file_name(const String &p_file_name) { file_name = p_file_name; }
	String get_file_name() const { return file_name; }

	void set_file_size(int64_t p_size) { file_size = p_size; }
	int64_t get_file_size() const { return file_size; }

	void set_preview_file_size(int64_t p_size) { preview_file_size = p_size; }
	int64_t get_preview_file_size() const { return preview_file_size; }

	void set_total_files_size(uint64_t p_size) { total_files_size = p_size; }
	uint64_t get_total_files_size() const { return total_files_size; }

	void set_url(const String &p_url) { url = p_url; }
	String get_url() const { return url; }

	void set_preview_url(const String &p_url) { preview_url = p_url; }
	String get_preview_url() const { return preview_url; }

	void set_votes_up(int p_votes) { votes_up = p_votes; }
	int get_votes_up() const { return votes_up; }

	void set_votes_down(int p_votes) { votes_down = p_votes; }
	int get_votes_down() const { return votes_down; }

	void set_score(float p_score) { score = p_score; }
	float get_score() const { return score; }

	void set_num_children(int p_num_children) { num_children = p_num_children; }
	int get_num_children() const { return num_children; }

	void set_children(const PackedInt64Array &p_children) { children = p_children; }
	PackedInt64Array get_children() const { return children; }

	void set_metadata(const String &p_metadata) { metadata = p_metadata; }
	String get_metadata() const { return metadata; }

	void set_key_value_tags(const Dictionary &p_tags) { key_value_tags = p_tags; }
	Dictionary get_key_value_tags() const { return key_value_tags; }

	void set_additional_previews(const Array &p_previews) { additional_previews = p_previews; }
	Array get_additional_previews() const { return additional_previews; }

	void set_statistics(const Dictionary &p_statistics) { statistics = p_statistics; }
	Dictionary get_statistics() const { return statistics; }

	// Live state, read from the Steam client for the local user.
	int get_state() const;
	bool is_subscribed() const;
	bool is_installed() const;
	bool needs_update() const;
	String get_install_folder() const;
};
