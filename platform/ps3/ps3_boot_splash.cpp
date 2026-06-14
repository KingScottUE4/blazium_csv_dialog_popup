/**************************************************************************/
/*  ps3_boot_splash.cpp                                                   */
/**************************************************************************/

#include "ps3_boot_splash.h"

#include "core/os/os.h"

#ifdef NORSX_ENABLED
#include "drivers/norsx/norsx_framebuffer.h"

static NoRSXFramebuffer g_boot_framebuffer;
static bool g_boot_splash_ready = false;
static uint64_t g_last_splash_refresh_usec = 0;

static const uint64_t SPLASH_REFRESH_INTERVAL_USEC = 2000000;
#endif

void ps3_boot_splash_present() {
#ifdef NORSX_ENABLED
	if (g_boot_framebuffer.initialize() != OK) {
		print_line("PS3: boot splash init failed");
		return;
	}
	g_boot_framebuffer.clear(0xFF202040);
	g_boot_framebuffer.present();
	g_boot_splash_ready = true;
	g_last_splash_refresh_usec = OS::get_singleton()->get_ticks_usec();
	print_line("PS3: boot splash presented");
#endif
}

void ps3_boot_splash_refresh_if_due() {
#ifdef NORSX_ENABLED
	if (!g_boot_splash_ready) {
		return;
	}
	const uint64_t now = OS::get_singleton()->get_ticks_usec();
	if (g_last_splash_refresh_usec != 0 && now - g_last_splash_refresh_usec < SPLASH_REFRESH_INTERVAL_USEC) {
		return;
	}
	g_boot_framebuffer.present();
	g_last_splash_refresh_usec = now;
#endif
}
