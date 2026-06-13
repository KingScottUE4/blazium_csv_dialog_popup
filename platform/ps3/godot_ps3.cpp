/**************************************************************************/
/*  godot_ps3.cpp                                                         */
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

#include "os_ps3.h"
#include "platform/ps3/ps3_wrapper.h"

#include "core/config/engine.h"
#include "core/io/resource_loader.h"
#include "main/main.h"
#include "servers/display_server.h"

#include <sysmodule.h>
#include <unistd.h>

static bool ps3_modules_loaded = false;

static void ps3_load_modules() {
	if (ps3_modules_loaded) {
		return;
	}
	sysModuleLoad(SYSMODULE_FS);
	sysModuleLoad(SYSMODULE_IO);
	sysModuleLoad(SYSMODULE_PAD);
	sysModuleLoad(SYSMODULE_SYSUTIL);
	sysModuleLoad(SYSMODULE_AUDIO);
	ps3_modules_loaded = true;
}

int main(int argc, char *argv[]) {
	ps3_load_modules();

	OS_PS3 os;

	char cwd[1024];
	if (getcwd(cwd, sizeof(cwd))) {
		print_line(vformat("PS3: cwd %s", cwd));
	}

	Error err = Main::setup(argv[0], argc - 1, &argv[1]);
	if (err != OK) {
		if (err == ERR_HELP) {
			return EXIT_SUCCESS;
		}
		return EXIT_FAILURE;
	}

	ResourceLoader::set_abort_on_missing_resources(false);

	int ret = Main::start();
	os.set_exit_code(ret);

	while (sysProcessExit() == 0) {
		if (!os.main_loop_iterate()) {
			break;
		}
		DisplayServer::get_singleton()->swap_buffers();
	}

	Main::cleanup();
	return os.get_exit_code();
}
