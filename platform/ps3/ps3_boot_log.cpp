/**************************************************************************/
/*  ps3_boot_log.cpp                                                      */
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

#include "ps3_boot_log.h"

#include "core/string/print_string.h"

#include <cstdio>
#include <cstring>

static FILE *g_boot_log_file = nullptr;
static bool g_boot_log_tried_open = false;

static const char *BOOT_LOG_PATHS[] = {
	"/app_home/USRDIR/boot.log",
	"/app_home/boot.log",
	"/dev_hdd0/home/00000001/blazium/boot.log",
	nullptr,
};

static const char *EARLY_BOOT_LOG_PATHS[] = {
	"/app_home/USRDIR/boot.log",
	"/app_home/boot.log",
	nullptr,
};

static void _append_early_log(const char *p_event) {
	if (!p_event) {
		return;
	}
	for (int i = 0; EARLY_BOOT_LOG_PATHS[i] != nullptr; i++) {
		FILE *f = fopen(EARLY_BOOT_LOG_PATHS[i], "a");
		if (f) {
			fprintf(f, "%s\n", p_event);
			fflush(f);
			fclose(f);
			return;
		}
	}
}

void ps3_boot_log_early_init() {
	_append_early_log("early_init");
}

#if defined(__GNUC__)
__attribute__((constructor(101))) static void _ps3_boot_log_constructor() {
	ps3_boot_log_early_init();
}
#endif

static void _ensure_boot_log_open() {
	if (g_boot_log_tried_open) {
		return;
	}
	g_boot_log_tried_open = true;

	for (int i = 0; BOOT_LOG_PATHS[i] != nullptr; i++) {
		g_boot_log_file = fopen(BOOT_LOG_PATHS[i], "a");
		if (g_boot_log_file) {
			break;
		}
	}
}

void ps3_boot_log(const char *p_event) {
	if (!p_event) {
		return;
	}

	print_line(vformat("PS3: %s", p_event));
	_ensure_boot_log_open();
	if (!g_boot_log_file) {
		return;
	}

	fprintf(g_boot_log_file, "%s\n", p_event);
	fflush(g_boot_log_file);
}

void ps3_boot_logf(const char *p_event, const String &p_detail) {
	if (!p_event) {
		return;
	}

	const String line = vformat("%s %s", p_event, p_detail);
	print_line(vformat("PS3: %s", line));
	_ensure_boot_log_open();
	if (!g_boot_log_file) {
		return;
	}

	const CharString utf8 = line.utf8();
	fprintf(g_boot_log_file, "%s\n", utf8.get_data());
	fflush(g_boot_log_file);
}
