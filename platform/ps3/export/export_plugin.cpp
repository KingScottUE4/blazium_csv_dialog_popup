/**************************************************************************/
/*  export_plugin.cpp                                                     */
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

#include "export_plugin.h"

#include "logo_svg.gen.h"

#include "core/config/project_settings.h"
#include "editor/export/editor_export.h"
#include "scene/resources/image_texture.h"

#include "modules/svg/image_loader_svg.h"
#include "editor/themes/editor_scale.h"

String EditorExportPlatformPS3::get_name() const {
	return "PS3";
}

String EditorExportPlatformPS3::get_os_name() const {
	return "PS3";
}

void EditorExportPlatformPS3::get_preset_features(const Ref<EditorExportPreset> &p_preset, List<String> *r_features) const {
	r_features->push_back("ppc64");
}

bool EditorExportPlatformPS3::has_valid_project_configuration(const Ref<EditorExportPreset> &p_preset, String &r_error) const {
	return true;
}

void EditorExportPlatformPS3::get_export_options(List<ExportOption> *r_options) const {
	r_options->push_back(ExportOption(PropertyInfo(Variant::STRING, "application/title_id", PROPERTY_HINT_PLACEHOLDER_TEXT, "BLAZ00001"), "BLAZ00001"));
	r_options->push_back(ExportOption(PropertyInfo(Variant::STRING, "application/content_id", PROPERTY_HINT_PLACEHOLDER_TEXT, "UP0001-BLAZ00001_00-BLAZIUM000"), "UP0001-BLAZ00001_00-BLAZIUM000"));
	r_options->push_back(ExportOption(PropertyInfo(Variant::STRING, "application/version", PROPERTY_HINT_PLACEHOLDER_TEXT, "01.00"), "01.00"));
	r_options->push_back(ExportOption(PropertyInfo(Variant::BOOL, "application/generate_pkg"), false));
	r_options->push_back(ExportOption(PropertyInfo(Variant::STRING, "application/emulator_root", PROPERTY_HINT_GLOBAL_DIR), ""));
}

bool EditorExportPlatformPS3::get_export_option_visibility(const EditorExportPreset *p_preset, const String &p_option) const {
	return true;
}

bool EditorExportPlatformPS3::has_valid_export_configuration(const Ref<EditorExportPreset> &p_preset, String &r_error, bool &r_missing_templates, bool p_debug) const {
	String err;
	bool dvalid = exists_export_template(_get_template_name(true), &err);
	bool rvalid = exists_export_template(_get_template_name(false), &err);

	if (p_preset->get("custom_template/debug") != "") {
		dvalid = FileAccess::exists(p_preset->get("custom_template/debug"));
	}
	if (p_preset->get("custom_template/release") != "") {
		rvalid = FileAccess::exists(p_preset->get("custom_template/release"));
	}

	bool valid = dvalid || rvalid;
	r_missing_templates = !valid;
	if (!err.is_empty()) {
		r_error = err;
	}
	return valid;
}

List<String> EditorExportPlatformPS3::get_binary_extensions(const Ref<EditorExportPreset> &p_preset) const {
	List<String> list;
	list.push_back("self");
	list.push_back("elf");
	return list;
}

Error EditorExportPlatformPS3::_copy_template(const String &p_template, const String &p_dest) const {
	Ref<FileAccess> src = FileAccess::open(p_template, FileAccess::READ);
	ERR_FAIL_COND_V_MSG(src.is_null(), ERR_FILE_CANT_OPEN, vformat("Cannot open template: %s", p_template));

	Ref<FileAccess> dst = FileAccess::open(p_dest, FileAccess::WRITE);
	ERR_FAIL_COND_V_MSG(dst.is_null(), ERR_CANT_CREATE, vformat("Cannot write: %s", p_dest));

	const uint64_t len = src->get_length();
	for (uint64_t i = 0; i < len; i++) {
		dst->store_8(src->get_8());
	}
	return OK;
}

Error EditorExportPlatformPS3::_write_param_sfo(const String &p_path, const String &p_title, const String &p_title_id, const String &p_version) const {
	Ref<FileAccess> f = FileAccess::open(p_path, FileAccess::WRITE);
	ERR_FAIL_COND_V_MSG(f.is_null(), ERR_CANT_CREATE, vformat("Cannot write PARAM.SFO: %s", p_path));

	f->store_string("PARAM.SFO placeholder - use deploy_rpcs3.py for full metadata\n");
	f->store_string("TITLE=" + p_title + "\n");
	f->store_string("TITLE_ID=" + p_title_id + "\n");
	f->store_string("VERSION=" + p_version + "\n");
	return OK;
}

Error EditorExportPlatformPS3::export_project(const Ref<EditorExportPreset> &p_preset, bool p_debug, const String &p_path, BitField<EditorExportPlatform::DebugFlags> p_flags) {
	String template_file = find_export_template(_get_template_name(p_debug));
	if (p_preset->get(p_debug ? "custom_template/debug" : "custom_template/release") != "") {
		template_file = p_preset->get(p_debug ? "custom_template/debug" : "custom_template/release");
	}
	ERR_FAIL_COND_V_MSG(template_file.is_empty(), ERR_FILE_NOT_FOUND, "PS3 export template not found.");

	String title_id = p_preset->get("application/title_id");
	if (title_id.is_empty()) {
		title_id = "BLAZ00001";
	}

	String base_path = p_path.get_basename();
	String game_dir = base_path.path_join(title_id);
	String usrdir = game_dir.path_join("USRDIR");
	Ref<DirAccess> da = DirAccess::create(DirAccess::ACCESS_FILESYSTEM);
	ERR_FAIL_COND_V(da->make_dir_recursive(usrdir) != OK, ERR_CANT_CREATE);

	String eboot_path = game_dir.path_join("EBOOT.BIN");
	Error err = _copy_template(template_file, eboot_path);
	ERR_FAIL_COND_V(err != OK, err);

	String pck_path = usrdir.path_join("game.pck");
	err = save_pack(p_preset, p_debug, pck_path);
	ERR_FAIL_COND_V(err != OK, err);

	String title = GLOBAL_GET("application/config/name");
	if (title.is_empty()) {
		title = "Blazium Game";
	}
	String version = p_preset->get("application/version");
	err = _write_param_sfo(game_dir.path_join("PARAM.SFO"), title, title_id, version);
	ERR_FAIL_COND_V(err != OK, err);

	String emulator_root = p_preset->get("application/emulator_root");
	if (!emulator_root.is_empty()) {
		String rpcs3_game = emulator_root.path_join("dev_hdd0").path_join("game").path_join(title_id);
		Ref<DirAccess> eda = DirAccess::create(DirAccess::ACCESS_FILESYSTEM);
		if (eda->make_dir_recursive(rpcs3_game.path_join("USRDIR")) == OK) {
			eda->copy(game_dir.path_join("EBOOT.BIN"), rpcs3_game.path_join("EBOOT.BIN"));
			eda->copy(game_dir.path_join("PARAM.SFO"), rpcs3_game.path_join("PARAM.SFO"));
			eda->copy(pck_path, rpcs3_game.path_join("USRDIR").path_join("game.pck"));
		}
	}

	add_message(EXPORT_MESSAGE_INFO, TTR("Export"), vformat(TTR("PS3 export written to %s"), game_dir));
	return OK;
}

Ref<Texture2D> EditorExportPlatformPS3::get_logo() const {
	return logo;
}

EditorExportPlatformPS3::EditorExportPlatformPS3() {
	Ref<Image> img;
	img.instantiate();
	bool upsample = false;
	ImageLoaderSVG::create_image_from_string(img, _ps3_logo_svg, EDSCALE, upsample, false);
	logo = ImageTexture::create_from_image(img);
}
