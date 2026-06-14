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
#include "core/io/file_access.h"
#include "core/io/resource_loader.h"
#include "main/main.h"
#include "servers/display_server.h"

#include <sysmodule/sysmodule.h>
#include <unistd.h>

static bool ps3_modules_loaded = false;

static void ps3_load_modules() {
	if (ps3_modules_loaded) {
		return;
	}
	sysModuleLoad(SYSMODULE_FS);
	sysModuleLoad(SYSMODULE_IO);
	sysModuleLoad(SYSMODULE_SYSUTIL);
	sysModuleLoad(SYSMODULE_AUDIO);
	ps3_modules_loaded = true;
	print_line("PS3: modules loaded");
}

static int64_t _pack_file_size(const String &p_path) {
	Ref<FileAccess> f = FileAccess::open(p_path, FileAccess::READ);
	if (f.is_null()) {
		return -1;
	}
	return (int64_t)f->get_length();
}

static String _find_main_pack(const String &p_exec_path) {
	const String game_dir = p_exec_path.get_base_dir();
	const String usrdir_pack = game_dir.path_join("USRDIR").path_join("game.pck");
	if (FileAccess::exists(usrdir_pack)) {
		return usrdir_pack;
	}

	const String eboot_pack = game_dir.path_join("EBOOT.pck");
	if (FileAccess::exists(eboot_pack)) {
		return eboot_pack;
	}

	return String();
}

int main(int argc, char *argv[]) {
	ps3_load_modules();

	OS_PS3 os;

	char cwd[1024];
	if (getcwd(cwd, sizeof(cwd))) {
		print_line(vformat("PS3: cwd %s", cwd));
	}

	const String main_pack = _find_main_pack(os.get_executable_path());
	if (!main_pack.is_empty()) {
		const int64_t pack_size = _pack_file_size(main_pack);
		print_line(vformat("PS3: loading pack %s (%d bytes)", main_pack, pack_size));
	} else {
		print_line("PS3: warning: no main pack found");
	}

	LocalVector<CharString> arg_storage;
	LocalVector<char *> new_argv;

	arg_storage.push_back(CharString(argv[0]));
	if (!main_pack.is_empty()) {
		arg_storage.push_back(CharString("--main-pack"));
		arg_storage.push_back(main_pack.utf8());
	}
	for (int i = 1; i < argc; i++) {
		arg_storage.push_back(CharString(argv[i]));
	}

	for (uint32_t i = 0; i < arg_storage.size(); i++) {
		new_argv.push_back(arg_storage[i].ptrw());
	}

	print_line("PS3: Main::setup begin");
	Error err = Main::setup(new_argv[0], new_argv.size() - 1, &new_argv[1]);
	if (err != OK) {
		print_line(vformat("PS3: Main::setup failed (error %d)", err));
		if (err == ERR_HELP) {
			return EXIT_SUCCESS;
		}
		return EXIT_FAILURE;
	}
	print_line("PS3: Main::setup end");

	if (DisplayServer::get_singleton()) {
		DisplayServer::get_singleton()->swap_buffers();
	}

	ResourceLoader::set_abort_on_missing_resources(false);

	print_line("PS3: Main::start");
	int ret = Main::start();
	os.set_exit_code(ret);

	uint64_t frame_count = 0;
	while (true) {
		if (!os.main_loop_iterate()) {
			break;
		}
		frame_count++;
		if (frame_count == 1) {
			print_line("PS3: first main_loop_iterate");
		} else if (frame_count % 60 == 0) {
			print_line(vformat("PS3: frame %d", frame_count));
		}
	}

	Main::cleanup();
	return os.get_exit_code();
}
