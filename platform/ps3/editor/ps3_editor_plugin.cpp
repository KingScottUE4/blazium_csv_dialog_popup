/**************************************************************************/
/*  ps3_editor_plugin.cpp                                                 */
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

#include "ps3_editor_plugin.h"

#include "core/config/project_settings.h"
#include "core/io/dir_access.h"
#include "core/io/file_access.h"
#include "core/version.h"
#include "editor/editor_node.h"
#include "editor/editor_paths.h"

static void _ps3_editor_init_callback() {
	EditorNode::get_singleton()->add_editor_plugin(memnew(PS3EditorPlugin));
}

void PS3EditorPlugin::initialize() {
	EditorNode::add_init_callback(_ps3_editor_init_callback);
}

PS3EditorPlugin::PS3EditorPlugin() {
	add_tool_menu_item("PS3: Validate Toolchain", callable_mp(this, &PS3EditorPlugin::_validate_toolchain));
	add_tool_menu_item("PS3: Deploy to RPCS3", callable_mp(this, &PS3EditorPlugin::_deploy_rpcs3));
	add_tool_menu_item("PS3: Open RPCS3 Data Folder", callable_mp(this, &PS3EditorPlugin::_open_rpcs3_data_folder));
}

void PS3EditorPlugin::_validate_toolchain() {
	String output;
	int exit_code = 0;
	List<String> args;
	args.push_back("-lc");
	args.push_back("source ~/.bashrc 2>/dev/null; ppu-gcc --version && test -f ${PS3DEV:-/usr/local/ps3dev}/ppu/lib/lv2.ld && echo PS3DEV_OK");

	Error err = OS::get_singleton()->execute("wsl", args, &output, &exit_code, true);
	if (err != OK || exit_code != 0) {
		EditorNode::get_singleton()->show_warning(vformat(TTR("PS3 toolchain validation failed.\n%s"), output));
		return;
	}

	const String templates_dir = EditorPaths::get_singleton()->get_export_templates_dir().path_join(EXTERNAL_VERSION_FULL_CONFIG);
	const bool release_ok = FileAccess::exists(templates_dir.path_join("ps3_release.ppc64"));
	const bool debug_ok = FileAccess::exists(templates_dir.path_join("ps3_debug.ppc64"));

	String message = TTR("PS3 toolchain OK (WSL ppu-gcc + lv2.ld).");
	if (!release_ok && !debug_ok) {
		message += "\n" + TTR("Export templates not installed. Copy bin/ps3_release.ppc64 and ps3_debug.ppc64 to export_templates.");
	} else {
		message += "\n" + TTR("Export templates found.");
	}

	EditorNode::get_singleton()->show_warning(message, TTR("PS3 Toolchain"));
}

void PS3EditorPlugin::_deploy_rpcs3() {
	const String title_id = "BLAZ00001";
	const String export_dir = ProjectSettings::get_singleton()->get_resource_path().path_join("export").path_join("ps3");
	const String emulator_root = ProjectSettings::get_singleton()->get_resource_path().path_join("..").path_join("..").path_join("ps3_emulator");

	String script_path = OS::get_singleton()->get_executable_path().get_base_dir().path_join("..").path_join("platform").path_join("ps3").path_join("scripts").path_join("deploy_rpcs3.py");
	if (!FileAccess::exists(script_path)) {
		script_path = ProjectSettings::get_singleton()->get_resource_path().path_join("..").path_join("..").path_join("blazium").path_join("platform").path_join("ps3").path_join("scripts").path_join("deploy_rpcs3.py");
	}

	List<String> args;
	args.push_back(script_path);
	args.push_back("--export-dir");
	args.push_back(export_dir);
	args.push_back("--title-id");
	args.push_back(title_id);
	args.push_back("--emulator-root");
	args.push_back(emulator_root);

	String output;
	int exit_code = 0;
	Error err = OS::get_singleton()->execute("py", args, &output, &exit_code, true);
	if (err != OK || exit_code != 0) {
		EditorNode::get_singleton()->show_warning(vformat(TTR("RPCS3 deploy failed.\n%s"), output));
		return;
	}

	EditorNode::get_singleton()->show_warning(output, TTR("PS3 Deploy"));
}

void PS3EditorPlugin::_open_rpcs3_data_folder() {
	const String emulator_root = ProjectSettings::get_singleton()->get_resource_path().path_join("..").path_join("..").path_join("ps3_emulator");
	const String game_dir = emulator_root.path_join("dev_hdd0").path_join("game");
	Ref<DirAccess> da = DirAccess::create(DirAccess::ACCESS_FILESYSTEM);
	if (da->make_dir_recursive(game_dir) != OK) {
		EditorNode::get_singleton()->show_warning(TTR("Could not create RPCS3 game directory."));
		return;
	}
	OS::get_singleton()->shell_open(ProjectSettings::get_singleton()->globalize_path(game_dir));
}
