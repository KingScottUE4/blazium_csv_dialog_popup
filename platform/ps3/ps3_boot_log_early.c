/**************************************************************************/
/*  ps3_boot_log_early.c — CRT-safe boot tracing (no C++ / Godot headers) */
/**************************************************************************/

#include <stdio.h>
#include <string.h>

#ifndef PS3_TITLE_ID
#define PS3_TITLE_ID "BLAZ00001"
#endif

static char g_hdd0_boot_log_path[128];
static int g_hdd0_boot_log_path_ready = 0;

static void _ensure_hdd0_boot_log_path(void) {
	if (g_hdd0_boot_log_path_ready) {
		return;
	}
	snprintf(
			g_hdd0_boot_log_path,
			sizeof(g_hdd0_boot_log_path),
			"/dev_hdd0/game/%s/USRDIR/boot.log",
			PS3_TITLE_ID);
	g_hdd0_boot_log_path_ready = 1;
}

static const char *EARLY_BOOT_LOG_PATHS[] = {
	NULL,
	"/app_home/USRDIR/boot.log",
	"/app_home/boot.log",
	NULL,
};

static void _append_via_fopen(const char *p_event) {
	const char *const *paths = EARLY_BOOT_LOG_PATHS;
	int i;

	if (!p_event) {
		return;
	}

	_ensure_hdd0_boot_log_path();
	EARLY_BOOT_LOG_PATHS[0] = g_hdd0_boot_log_path;

	for (i = 0; paths[i] != NULL; i++) {
		FILE *f = fopen(paths[i], "a");
		if (f) {
			fprintf(f, "%s\n", p_event);
			fflush(f);
			fclose(f);
			return;
		}
	}
}

void ps3_boot_log_trace(const char *p_event) {
	if (!p_event) {
		return;
	}
	printf("PS3: %s\n", p_event);
	fflush(stdout);
	_append_via_fopen(p_event);
}

void ps3_boot_log_early_init(void) {
	ps3_boot_log_trace("early_init");
}

#if defined(__GNUC__)
__attribute__((constructor(101))) static void _ps3_boot_log_constructor(void) {
	ps3_boot_log_early_init();
}
#endif
