/**************************************************************************/
/*  steam_types.h                                                         */
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

#include "core/typedefs.h"

// Minimal Steamworks types used by the module at runtime.
// Kept local so CI/builds succeed without the official Steam SDK headers.

enum SteamEResult {
	STEAM_RESULT_OK = 1,
	STEAM_RESULT_PENDING = 22,
	STEAM_RESULT_EXPIRED = 16,
	STEAM_RESULT_FAIL = 2,
	STEAM_RESULT_IO_FAILURE = 15,
};

enum {
	STEAM_USER_CALLBACKS_BASE = 100,
	STEAM_GET_TICKET_FOR_WEB_API_RESPONSE_CALLBACK = STEAM_USER_CALLBACKS_BASE + 68,
	STEAM_USER_STATS_CALLBACKS_BASE = 1100,
	STEAM_USER_STATS_RECEIVED_CALLBACK = STEAM_USER_STATS_CALLBACKS_BASE + 1,
	STEAM_USER_STATS_STORED_CALLBACK = STEAM_USER_STATS_CALLBACKS_BASE + 2,
	STEAM_UTILS_CALLBACKS_BASE = 700,
	STEAM_API_CALL_COMPLETED_CALLBACK = STEAM_UTILS_CALLBACKS_BASE + 3,
	STEAM_INVENTORY_CALLBACKS_BASE = 4700,
	STEAM_INVENTORY_RESULT_READY_CALLBACK = STEAM_INVENTORY_CALLBACKS_BASE + 0,
	STEAM_INVENTORY_FULL_UPDATE_CALLBACK = STEAM_INVENTORY_CALLBACKS_BASE + 1,
	STEAM_INVENTORY_DEFINITION_UPDATE_CALLBACK = STEAM_INVENTORY_CALLBACKS_BASE + 2,
};

typedef uint64_t SteamItemInstanceID_t;
typedef int32_t SteamItemDef_t;
typedef int32_t SteamInventoryResult_t;
typedef uint64_t SteamInventoryUpdateHandle_t;

static constexpr SteamItemInstanceID_t STEAM_ITEM_INSTANCE_ID_INVALID = ~(SteamItemInstanceID_t)0;
static constexpr SteamInventoryResult_t STEAM_INVENTORY_RESULT_INVALID = -1;
static constexpr SteamInventoryUpdateHandle_t STEAM_INVENTORY_UPDATE_HANDLE_INVALID = 0xffffffffffffffffULL;

enum SteamItemFlags {
	STEAM_ITEM_NO_TRADE = 1 << 0,
	STEAM_ITEM_REMOVED = 1 << 8,
	STEAM_ITEM_CONSUMED = 1 << 9,
};

struct SteamItemDetails {
	SteamItemInstanceID_t item_id = STEAM_ITEM_INSTANCE_ID_INVALID;
	SteamItemDef_t definition = 0;
	uint16_t quantity = 0;
	uint16_t flags = 0;
};

typedef uint32_t SteamHAuthTicket;
typedef int32_t SteamHSteamUser;
typedef int32_t SteamHSteamPipe;
typedef uint64_t SteamCSteamID;

#pragma pack(push, 8)

struct SteamCallbackMsg {
	SteamHSteamUser m_hSteamUser = 0;
	int m_iCallback = 0;
	uint8_t *m_pubParam = nullptr;
	int m_cubParam = 0;
};

struct SteamAPICallCompleted {
	enum { k_iCallback = STEAM_API_CALL_COMPLETED_CALLBACK };

	uint64_t m_hAsyncCall = 0;
	int m_iCallback = 0;
	uint32_t m_cubParam = 0;
};

struct SteamGetTicketForWebApiResponse {
	enum {
		k_iCallback = STEAM_GET_TICKET_FOR_WEB_API_RESPONSE_CALLBACK,
		k_nCubTicketMaxLength = 2560,
	};

	SteamHAuthTicket m_hAuthTicket = 0;
	int m_eResult = 0;
	int m_cubTicket = 0;
	uint8_t m_rgubTicket[k_nCubTicketMaxLength];
};

struct SteamUserStatsReceived {
	enum { k_iCallback = STEAM_USER_STATS_RECEIVED_CALLBACK };

	uint64_t m_nGameID = 0;
	int m_eResult = 0;
	SteamCSteamID m_steamIDUser = 0;
};

struct SteamUserStatsStored {
	enum { k_iCallback = STEAM_USER_STATS_STORED_CALLBACK };

	uint64_t m_nGameID = 0;
	int m_eResult = 0;
};

struct SteamInventoryResultReady {
	enum { k_iCallback = STEAM_INVENTORY_RESULT_READY_CALLBACK };

	SteamInventoryResult_t m_handle = STEAM_INVENTORY_RESULT_INVALID;
	int m_result = 0;
};

struct SteamInventoryFullUpdate {
	enum { k_iCallback = STEAM_INVENTORY_FULL_UPDATE_CALLBACK };

	SteamInventoryResult_t m_handle = STEAM_INVENTORY_RESULT_INVALID;
};

struct SteamInventoryDefinitionUpdate {
	enum { k_iCallback = STEAM_INVENTORY_DEFINITION_UPDATE_CALLBACK };
};

#pragma pack(pop)

// -----------------------------------------------------------------------------
// Steam Workshop (ISteamUGC) types.
// -----------------------------------------------------------------------------

enum {
	STEAM_REMOTE_STORAGE_CALLBACKS_BASE = 1300,
	STEAM_REMOTE_STORAGE_SUBSCRIBE_PUBLISHED_FILE_RESULT_CALLBACK = STEAM_REMOTE_STORAGE_CALLBACKS_BASE + 13,
	STEAM_REMOTE_STORAGE_UNSUBSCRIBE_PUBLISHED_FILE_RESULT_CALLBACK = STEAM_REMOTE_STORAGE_CALLBACKS_BASE + 15,
	STEAM_UGC_CALLBACKS_BASE = 3400,
	STEAM_UGC_QUERY_COMPLETED_CALLBACK = STEAM_UGC_CALLBACKS_BASE + 1,
	STEAM_UGC_CREATE_ITEM_RESULT_CALLBACK = STEAM_UGC_CALLBACKS_BASE + 3,
	STEAM_UGC_SUBMIT_ITEM_UPDATE_RESULT_CALLBACK = STEAM_UGC_CALLBACKS_BASE + 4,
	STEAM_UGC_ITEM_INSTALLED_CALLBACK = STEAM_UGC_CALLBACKS_BASE + 5,
	STEAM_UGC_DOWNLOAD_ITEM_RESULT_CALLBACK = STEAM_UGC_CALLBACKS_BASE + 6,
	STEAM_UGC_USER_FAVORITE_ITEMS_LIST_CHANGED_CALLBACK = STEAM_UGC_CALLBACKS_BASE + 7,
	STEAM_UGC_SET_USER_ITEM_VOTE_RESULT_CALLBACK = STEAM_UGC_CALLBACKS_BASE + 8,
	STEAM_UGC_START_PLAYTIME_TRACKING_RESULT_CALLBACK = STEAM_UGC_CALLBACKS_BASE + 10,
	STEAM_UGC_STOP_PLAYTIME_TRACKING_RESULT_CALLBACK = STEAM_UGC_CALLBACKS_BASE + 11,
	STEAM_UGC_DELETE_ITEM_RESULT_CALLBACK = STEAM_UGC_CALLBACKS_BASE + 17,
	STEAM_UGC_USER_SUBSCRIBED_ITEMS_LIST_CHANGED_CALLBACK = STEAM_UGC_CALLBACKS_BASE + 18,
};

typedef uint64_t SteamPublishedFileId_t;
typedef uint64_t SteamUGCQueryHandle_t;
typedef uint64_t SteamUGCUpdateHandle_t;
typedef uint64_t SteamUGCHandle_t;
typedef uint64_t SteamAPICallHandle_t;

static constexpr SteamUGCQueryHandle_t STEAM_UGC_QUERY_HANDLE_INVALID = 0xffffffffffffffffULL;
static constexpr SteamUGCUpdateHandle_t STEAM_UGC_UPDATE_HANDLE_INVALID = 0xffffffffffffffffULL;
static constexpr SteamAPICallHandle_t STEAM_API_CALL_INVALID = 0;

static constexpr uint32_t STEAM_UGC_TITLE_MAX = 128 + 1; // k_cchPublishedDocumentTitleMax
static constexpr uint32_t STEAM_UGC_DESCRIPTION_MAX = 8000; // k_cchPublishedDocumentDescriptionMax
static constexpr uint32_t STEAM_UGC_TAG_LIST_MAX = 1024 + 1; // k_cchTagListMax
static constexpr uint32_t STEAM_UGC_FILENAME_MAX = 260; // k_cchFilenameMax
static constexpr uint32_t STEAM_UGC_URL_MAX = 256; // k_cchPublishedFileURLMax
static constexpr uint32_t STEAM_UGC_DEVELOPER_METADATA_MAX = 10000; // k_cchDeveloperMetadataMax (5000 before SDK 1.62)
static constexpr uint32_t STEAM_UGC_RESULTS_PER_PAGE = 50; // kNumUGCResultsPerPage

// Steamworks packs callback structures to 4 bytes on POSIX platforms and to
// 8 bytes on Windows (VALVE_CALLBACK_PACK_SMALL / VALVE_CALLBACK_PACK_LARGE).
// The Workshop structures contain 64-bit fields after 32-bit ones, so the
// packing must match exactly or the field offsets are wrong.
#if defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__)
#define STEAM_UGC_CALLBACK_PACK_SMALL
#pragma pack(push, 4)
#else
#pragma pack(push, 8)
#endif

struct SteamParamStringArray {
	const char **m_ppStrings = nullptr;
	int32_t m_nNumStrings = 0;
};

struct SteamUGCDetails {
	SteamPublishedFileId_t m_nPublishedFileId;
	int m_eResult;
	int m_eFileType;
	uint32_t m_nCreatorAppID;
	uint32_t m_nConsumerAppID;
	char m_rgchTitle[STEAM_UGC_TITLE_MAX];
	char m_rgchDescription[STEAM_UGC_DESCRIPTION_MAX];
	uint64_t m_ulSteamIDOwner;
	uint32_t m_rtimeCreated;
	uint32_t m_rtimeUpdated;
	uint32_t m_rtimeAddedToUserList;
	int m_eVisibility;
	bool m_bBanned;
	bool m_bAcceptedForUse;
	bool m_bTagsTruncated;
	char m_rgchTags[STEAM_UGC_TAG_LIST_MAX];
	SteamUGCHandle_t m_hFile;
	SteamUGCHandle_t m_hPreviewFile;
	char m_pchFileName[STEAM_UGC_FILENAME_MAX];
	int32_t m_nFileSize;
	int32_t m_nPreviewFileSize;
	char m_rgchURL[STEAM_UGC_URL_MAX];
	uint32_t m_unVotesUp;
	uint32_t m_unVotesDown;
	float m_flScore;
	uint32_t m_unNumChildren;
	uint64_t m_ulTotalFilesSize;
};

struct SteamUGCQueryCompleted {
	enum { k_iCallback = STEAM_UGC_QUERY_COMPLETED_CALLBACK };

	SteamUGCQueryHandle_t m_handle;
	int m_eResult;
	uint32_t m_unNumResultsReturned;
	uint32_t m_unTotalMatchingResults;
	bool m_bCachedData;
	char m_rgchNextCursor[STEAM_UGC_URL_MAX];
};

struct SteamUGCCreateItemResult {
	enum { k_iCallback = STEAM_UGC_CREATE_ITEM_RESULT_CALLBACK };

	int m_eResult;
	SteamPublishedFileId_t m_nPublishedFileId;
	bool m_bUserNeedsToAcceptWorkshopLegalAgreement;
};

struct SteamUGCSubmitItemUpdateResult {
	enum { k_iCallback = STEAM_UGC_SUBMIT_ITEM_UPDATE_RESULT_CALLBACK };

	int m_eResult;
	bool m_bUserNeedsToAcceptWorkshopLegalAgreement;
	SteamPublishedFileId_t m_nPublishedFileId;
};

struct SteamUGCItemInstalled {
	enum { k_iCallback = STEAM_UGC_ITEM_INSTALLED_CALLBACK };

	uint32_t m_unAppID;
	SteamPublishedFileId_t m_nPublishedFileId;
	SteamUGCHandle_t m_hLegacyContent;
	uint64_t m_unManifestID;
};

struct SteamUGCDownloadItemResult {
	enum { k_iCallback = STEAM_UGC_DOWNLOAD_ITEM_RESULT_CALLBACK };

	uint32_t m_unAppID;
	SteamPublishedFileId_t m_nPublishedFileId;
	int m_eResult;
};

struct SteamUGCUserFavoriteItemsListChanged {
	enum { k_iCallback = STEAM_UGC_USER_FAVORITE_ITEMS_LIST_CHANGED_CALLBACK };

	SteamPublishedFileId_t m_nPublishedFileId;
	int m_eResult;
	bool m_bWasAddRequest;
};

struct SteamUGCSetUserItemVoteResult {
	enum { k_iCallback = STEAM_UGC_SET_USER_ITEM_VOTE_RESULT_CALLBACK };

	SteamPublishedFileId_t m_nPublishedFileId;
	int m_eResult;
	bool m_bVoteUp;
};

struct SteamUGCPlaytimeTrackingResult {
	int m_eResult;
};

struct SteamUGCDeleteItemResult {
	enum { k_iCallback = STEAM_UGC_DELETE_ITEM_RESULT_CALLBACK };

	int m_eResult;
	SteamPublishedFileId_t m_nPublishedFileId;
};

struct SteamUGCUserSubscribedItemsListChanged {
	enum { k_iCallback = STEAM_UGC_USER_SUBSCRIBED_ITEMS_LIST_CHANGED_CALLBACK };

	uint32_t m_nAppID;
};

struct SteamRemoteStorageSubscribePublishedFileResult {
	enum { k_iCallback = STEAM_REMOTE_STORAGE_SUBSCRIBE_PUBLISHED_FILE_RESULT_CALLBACK };

	int m_eResult;
	SteamPublishedFileId_t m_nPublishedFileId;
};

struct SteamRemoteStorageUnsubscribePublishedFileResult {
	enum { k_iCallback = STEAM_REMOTE_STORAGE_UNSUBSCRIBE_PUBLISHED_FILE_RESULT_CALLBACK };

	int m_eResult;
	SteamPublishedFileId_t m_nPublishedFileId;
};

#pragma pack(pop)

// Layout checks against the Steamworks SDK (1.62 - 1.65) on 64-bit targets.
#if defined(__x86_64__) || defined(_M_X64) || defined(__aarch64__) || defined(_M_ARM64)
#ifdef STEAM_UGC_CALLBACK_PACK_SMALL
static_assert(sizeof(SteamParamStringArray) == 12, "SteamParamStringArray layout mismatch");
static_assert(sizeof(SteamUGCDetails) == 9772, "SteamUGCDetails layout mismatch");
static_assert(sizeof(SteamUGCCreateItemResult) == 16, "CreateItemResult layout mismatch");
static_assert(sizeof(SteamUGCItemInstalled) == 28, "ItemInstalled layout mismatch");
static_assert(sizeof(SteamUGCDownloadItemResult) == 16, "DownloadItemResult layout mismatch");
#else
static_assert(sizeof(SteamParamStringArray) == 16, "SteamParamStringArray layout mismatch");
static_assert(sizeof(SteamUGCDetails) == 9784, "SteamUGCDetails layout mismatch");
static_assert(sizeof(SteamUGCCreateItemResult) == 24, "CreateItemResult layout mismatch");
static_assert(sizeof(SteamUGCItemInstalled) == 32, "ItemInstalled layout mismatch");
static_assert(sizeof(SteamUGCDownloadItemResult) == 24, "DownloadItemResult layout mismatch");
#endif
static_assert(sizeof(SteamUGCQueryCompleted) == 280, "SteamUGCQueryCompleted layout mismatch");
#endif
