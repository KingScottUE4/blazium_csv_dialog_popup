/**************************************************************************/
/*  steam_workshop_item.cpp                                               */
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

#include "steam_workshop_item.h"

#include "steam.h"

#include "core/object/class_db.h"

void SteamWorkshopItem::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_published_file_id"), &SteamWorkshopItem::get_published_file_id);
	ClassDB::bind_method(D_METHOD("get_result"), &SteamWorkshopItem::get_result);
	ClassDB::bind_method(D_METHOD("get_file_type"), &SteamWorkshopItem::get_file_type);
	ClassDB::bind_method(D_METHOD("get_creator_app_id"), &SteamWorkshopItem::get_creator_app_id);
	ClassDB::bind_method(D_METHOD("get_consumer_app_id"), &SteamWorkshopItem::get_consumer_app_id);
	ClassDB::bind_method(D_METHOD("get_title"), &SteamWorkshopItem::get_title);
	ClassDB::bind_method(D_METHOD("get_description"), &SteamWorkshopItem::get_description);
	ClassDB::bind_method(D_METHOD("get_owner_steam_id"), &SteamWorkshopItem::get_owner_steam_id);
	ClassDB::bind_method(D_METHOD("get_time_created"), &SteamWorkshopItem::get_time_created);
	ClassDB::bind_method(D_METHOD("get_time_updated"), &SteamWorkshopItem::get_time_updated);
	ClassDB::bind_method(D_METHOD("get_time_added_to_user_list"), &SteamWorkshopItem::get_time_added_to_user_list);
	ClassDB::bind_method(D_METHOD("get_visibility"), &SteamWorkshopItem::get_visibility);
	ClassDB::bind_method(D_METHOD("is_banned"), &SteamWorkshopItem::is_banned);
	ClassDB::bind_method(D_METHOD("is_accepted_for_use"), &SteamWorkshopItem::is_accepted_for_use);
	ClassDB::bind_method(D_METHOD("are_tags_truncated"), &SteamWorkshopItem::are_tags_truncated);
	ClassDB::bind_method(D_METHOD("get_tags"), &SteamWorkshopItem::get_tags);
	ClassDB::bind_method(D_METHOD("has_tag", "tag"), &SteamWorkshopItem::has_tag);
	ClassDB::bind_method(D_METHOD("get_file_name"), &SteamWorkshopItem::get_file_name);
	ClassDB::bind_method(D_METHOD("get_file_size"), &SteamWorkshopItem::get_file_size);
	ClassDB::bind_method(D_METHOD("get_preview_file_size"), &SteamWorkshopItem::get_preview_file_size);
	ClassDB::bind_method(D_METHOD("get_total_files_size"), &SteamWorkshopItem::get_total_files_size);
	ClassDB::bind_method(D_METHOD("get_url"), &SteamWorkshopItem::get_url);
	ClassDB::bind_method(D_METHOD("get_preview_url"), &SteamWorkshopItem::get_preview_url);
	ClassDB::bind_method(D_METHOD("get_votes_up"), &SteamWorkshopItem::get_votes_up);
	ClassDB::bind_method(D_METHOD("get_votes_down"), &SteamWorkshopItem::get_votes_down);
	ClassDB::bind_method(D_METHOD("get_score"), &SteamWorkshopItem::get_score);
	ClassDB::bind_method(D_METHOD("get_num_children"), &SteamWorkshopItem::get_num_children);
	ClassDB::bind_method(D_METHOD("get_children"), &SteamWorkshopItem::get_children);
	ClassDB::bind_method(D_METHOD("get_metadata"), &SteamWorkshopItem::get_metadata);
	ClassDB::bind_method(D_METHOD("get_key_value_tags"), &SteamWorkshopItem::get_key_value_tags);
	ClassDB::bind_method(D_METHOD("get_additional_previews"), &SteamWorkshopItem::get_additional_previews);
	ClassDB::bind_method(D_METHOD("get_statistics"), &SteamWorkshopItem::get_statistics);
	ClassDB::bind_method(D_METHOD("get_state"), &SteamWorkshopItem::get_state);
	ClassDB::bind_method(D_METHOD("is_subscribed"), &SteamWorkshopItem::is_subscribed);
	ClassDB::bind_method(D_METHOD("is_installed"), &SteamWorkshopItem::is_installed);
	ClassDB::bind_method(D_METHOD("needs_update"), &SteamWorkshopItem::needs_update);
	ClassDB::bind_method(D_METHOD("get_install_folder"), &SteamWorkshopItem::get_install_folder);
}

bool SteamWorkshopItem::has_tag(const String &p_tag) const {
	for (int i = 0; i < tags.size(); i++) {
		if (tags[i].nocasecmp_to(p_tag) == 0) {
			return true;
		}
	}
	return false;
}

int SteamWorkshopItem::get_state() const {
	Steam *steam = Steam::get_singleton();
	if (!steam || published_file_id == 0) {
		return 0;
	}
	return (int)steam->get_item_state(published_file_id);
}

bool SteamWorkshopItem::is_subscribed() const {
	return (get_state() & Steam::WORKSHOP_ITEM_STATE_SUBSCRIBED) != 0;
}

bool SteamWorkshopItem::is_installed() const {
	return (get_state() & Steam::WORKSHOP_ITEM_STATE_INSTALLED) != 0;
}

bool SteamWorkshopItem::needs_update() const {
	return (get_state() & Steam::WORKSHOP_ITEM_STATE_NEEDS_UPDATE) != 0;
}

String SteamWorkshopItem::get_install_folder() const {
	Steam *steam = Steam::get_singleton();
	if (!steam || published_file_id == 0) {
		return String();
	}
	const Dictionary info = steam->get_item_install_info(published_file_id);
	return info.has("folder") ? String(info["folder"]) : String();
}
