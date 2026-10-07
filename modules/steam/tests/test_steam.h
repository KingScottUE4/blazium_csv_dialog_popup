/**************************************************************************/
/*  test_steam.h                                                          */
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

#include "../steam.h"
#include "../steam_api_loader.h"
#include "../steam_inventory_item.h"
#include "../steam_workshop_item.h"

#include "core/io/image.h"
#include "tests/test_macros.h"

namespace TestSteam {

TEST_CASE("[Steam] bytes_to_hex encoding") {
	const uint8_t data[] = { 0xDE, 0xAD, 0xBE, 0xEF };
	String hex = SteamAPILoader::bytes_to_hex(data, 4);
	CHECK(hex == "deadbeef");
}

TEST_CASE("[Steam] image_from_rgba conversion") {
	const uint8_t rgba[] = {
		255,
		0,
		0,
		255,
		0,
		255,
		0,
		255,
		0,
		0,
		255,
		255,
		255,
		255,
		255,
		255,
	};
	Ref<Image> image = SteamAPILoader::image_from_rgba(rgba, 2, 2);
	REQUIRE(image.is_valid());
	CHECK(image->get_width() == 2);
	CHECK(image->get_height() == 2);
	CHECK(image->get_format() == Image::FORMAT_RGBA8);
	Color top_left = image->get_pixel(0, 0);
	CHECK(top_left.get_r8() == 255);
	CHECK(top_left.get_a8() == 255);
}

TEST_CASE("[Steam] loader unavailable without dll") {
	SteamAPILoader loader;
	const bool loaded = loader.try_load();
	CHECK(loaded == loader.is_loaded());
}

TEST_CASE("[Steam] achievement methods no-op without init") {
	Steam *steam = Steam::get_singleton();
	REQUIRE(steam != nullptr);
	CHECK_FALSE(steam->set_achievement("TEST_ACHIEVEMENT"));
	CHECK_FALSE(steam->clear_achievement("TEST_ACHIEVEMENT"));
	CHECK_FALSE(steam->get_achievement("TEST_ACHIEVEMENT"));
	CHECK_FALSE(steam->request_current_stats());
	CHECK_FALSE(steam->refresh_current_stats());
	CHECK_FALSE(steam->store_stats());
	Ref<SteamAchievementInfo> info = steam->get_achievement_info("TEST_ACHIEVEMENT");
	CHECK_FALSE(info.is_valid());
	CHECK(steam->get_all_achievements().is_empty());
	CHECK(steam->get_stat_int("TEST_STAT") == -1);
	CHECK(steam->get_stat_float("TEST_STAT") == 0.0f);
	CHECK_FALSE(steam->set_stat_int("TEST_STAT", 1));
	CHECK_FALSE(steam->set_stat_float("TEST_STAT", 1.0f));
	CHECK_FALSE(steam->increment_stat_int("TEST_STAT", 1));
	CHECK_FALSE(steam->increment_stat_int("TEST_STAT", 2));
	CHECK_FALSE(steam->clear_stat("TEST_STAT"));
	CHECK(steam->get_persona_name().is_empty());
	CHECK_FALSE(steam->get_avatar_image().is_valid());
	CHECK_FALSE(steam->load_item_definitions());
	CHECK(steam->get_item_definition_ids().is_empty());
	CHECK_FALSE(steam->get_item_definition(1).is_valid());
	CHECK(steam->get_all_item_definitions().is_empty());
	CHECK(steam->get_all_items().is_empty());
	CHECK(steam->get_items_by_id(PackedInt64Array()).is_empty());
	CHECK(steam->find_items_by_def_id(1).is_empty());
	CHECK_FALSE(steam->get_item_definition_icon(1).is_valid());
	CHECK_FALSE(steam->consume_item(1, 1));
	CHECK_FALSE(steam->trigger_item_drop(1));
	CHECK(steam->get_eligible_promo_item_definition_ids().is_empty());
	CHECK(steam->get_inventory_result_status(-1) == 0);
	CHECK(steam->get_inventory_result_items(-1).is_empty());
	CHECK_FALSE(steam->grant_promo_items());
	CHECK_FALSE(steam->add_promo_item(1));
	CHECK_FALSE(steam->start_property_update());
	CHECK_FALSE(steam->submit_property_update());
	CHECK(steam->deserialize_inventory(PackedByteArray()) == -1);
	CHECK_FALSE(steam->has_inventory_support());
}

TEST_CASE("[Steam] inventory item without init") {
	Ref<SteamInventoryItem> item;
	item.instantiate();
	item->set_def_id(100);
	CHECK(item->get_def_id() == 100);
	CHECK_FALSE(item->get_definition().is_valid());
}

TEST_CASE("[Steam] workshop methods no-op without init") {
	Steam *steam = Steam::get_singleton();
	REQUIRE(steam != nullptr);
	CHECK(steam->get_num_subscribed_items() == 0);
	CHECK(steam->get_subscribed_items().is_empty());
	CHECK(int64_t(steam->get_item_state(123)) == Steam::WORKSHOP_ITEM_STATE_NONE);
	CHECK(steam->get_item_install_info(123).is_empty());
	CHECK(steam->get_item_download_info(123).is_empty());
	CHECK_FALSE(steam->download_item(123));
	CHECK_FALSE(steam->subscribe_item(123));
	CHECK_FALSE(steam->unsubscribe_item(123));
	CHECK(steam->query_workshop_items() == -1);
	CHECK(steam->query_user_workshop_items() == -1);
	CHECK(steam->query_workshop_item_details(PackedInt64Array({ 123 })) == -1);
	CHECK_FALSE(steam->create_workshop_item());
	CHECK(steam->start_item_update(123) == -1);
	CHECK_FALSE(steam->set_item_title(1, "Title"));
	CHECK_FALSE(steam->set_item_description(1, "Description"));
	CHECK_FALSE(steam->set_item_metadata(1, "{}"));
	CHECK_FALSE(steam->set_item_visibility(1, Steam::WORKSHOP_VISIBILITY_PUBLIC));
	CHECK_FALSE(steam->set_item_tags(1, PackedStringArray({ "Maps" })));
	CHECK_FALSE(steam->set_item_content(1, "user://mod"));
	CHECK_FALSE(steam->set_item_preview(1, "user://preview.png"));
	CHECK_FALSE(steam->add_item_key_value_tag(1, "key", "value"));
	CHECK_FALSE(steam->submit_item_update(1, "note"));
	CHECK(int(steam->get_item_update_progress(1)["status"]) == Steam::WORKSHOP_UPDATE_STATUS_INVALID);
	CHECK_FALSE(steam->delete_workshop_item(123));
	CHECK_FALSE(steam->set_user_item_vote(123, true));
	CHECK_FALSE(steam->add_item_to_favorites(123));
	CHECK_FALSE(steam->remove_item_from_favorites(123));
	CHECK_FALSE(steam->start_playtime_tracking(PackedInt64Array({ 123 })));
	CHECK_FALSE(steam->stop_playtime_tracking_for_all_items());
	CHECK_FALSE(steam->show_workshop_eula());
	CHECK_FALSE(steam->has_workshop_support());
}

TEST_CASE("[Steam] workshop item without init") {
	Ref<SteamWorkshopItem> item;
	item.instantiate();
	item->set_published_file_id(42);
	item->set_title("Test Map");
	item->set_tags(PackedStringArray({ "Maps", "Co-op" }));
	CHECK(item->get_published_file_id() == 42);
	CHECK(item->get_title() == "Test Map");
	CHECK(item->has_tag("maps"));
	CHECK_FALSE(item->has_tag("Weapons"));
	CHECK(item->get_state() == 0);
	CHECK_FALSE(item->is_subscribed());
	CHECK_FALSE(item->is_installed());
	CHECK(item->get_install_folder().is_empty());
}

TEST_CASE("[Steam] workshop callback struct layouts") {
	// Offsets of fields that follow 32-bit members; these differ between
	// Steamworks' 4-byte (POSIX) and 8-byte (Windows) callback packing.
#if defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__)
	CHECK(offsetof(SteamUGCCreateItemResult, m_nPublishedFileId) == 4);
	CHECK(offsetof(SteamUGCDownloadItemResult, m_nPublishedFileId) == 4);
	CHECK(offsetof(SteamUGCDetails, m_ulSteamIDOwner) == 8156);
#else
	CHECK(offsetof(SteamUGCCreateItemResult, m_nPublishedFileId) == 8);
	CHECK(offsetof(SteamUGCDownloadItemResult, m_nPublishedFileId) == 8);
	CHECK(offsetof(SteamUGCDetails, m_ulSteamIDOwner) == 8160);
#endif
	CHECK(offsetof(SteamUGCQueryCompleted, m_rgchNextCursor) == 21);
}

TEST_CASE("[Steam] singleton registration") {
	CHECK(Steam::get_singleton() != nullptr);
}

} // namespace TestSteam
