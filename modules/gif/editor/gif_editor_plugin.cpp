/**************************************************************************/
/*  gif_editor_plugin.cpp                                                 */
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

#ifdef TOOLS_ENABLED

#include "gif_editor_plugin.h"

#include "modules/gif/gif_texture.h"

#include "core/config/engine.h"
#include "core/os/os.h"
#include "editor/editor_data.h"
#include "editor/editor_file_system.h"
#include "editor/editor_interface.h"
#include "editor/editor_main_screen.h"
#include "editor/editor_node.h"
#include "editor/gui/editor_file_dialog.h"
#include "editor/gui/editor_run_bar.h"
#include "editor/plugins/game_view_plugin.h"
#include "editor/themes/editor_scale.h"
#include "scene/animation/animation_player.h"
#include "scene/gui/box_container.h"
#include "scene/gui/button.h"
#include "scene/gui/check_button.h"
#include "scene/gui/control.h"
#include "scene/gui/label.h"
#include "scene/gui/line_edit.h"
#include "scene/gui/slider.h"
#include "scene/gui/texture_rect.h"
#include "scene/main/scene_tree.h"
#include "scene/main/viewport.h"
#include "scene/main/window.h"
#include "scene/resources/animation.h"
#include "scene/resources/image_texture.h"
#include "servers/display_server.h"

class GIFInspectorControls : public VBoxContainer {
	GDCLASS(GIFInspectorControls, VBoxContainer);

	Ref<GIFTexture> target;
	TextureRect *preview = nullptr;
	Label *info = nullptr;
	Button *play_btn = nullptr;
	CheckButton *loop_btn = nullptr;
	HSlider *frame_slider = nullptr;

	void _load_pressed() {
		EditorFileDialog *fd = memnew(EditorFileDialog);
		fd->set_file_mode(EditorFileDialog::FILE_MODE_OPEN_FILE);
		fd->set_access(EditorFileDialog::ACCESS_RESOURCES);
		fd->add_filter("*.gif", "GIF");
		add_child(fd);
		fd->connect("file_selected", callable_mp(this, &GIFInspectorControls::_file_loaded));
		fd->popup_file_dialog();
	}

	void _file_loaded(const String &p_path) {
		if (target.is_valid()) {
			target->load_from_path(p_path);
			_refresh();
		}
	}

	void _save_pressed() {
		EditorFileDialog *fd = memnew(EditorFileDialog);
		fd->set_file_mode(EditorFileDialog::FILE_MODE_SAVE_FILE);
		fd->set_access(EditorFileDialog::ACCESS_RESOURCES);
		fd->add_filter("*.gif", "GIF");
		add_child(fd);
		fd->connect("file_selected", callable_mp(this, &GIFInspectorControls::_file_saved));
		fd->popup_file_dialog();
	}

	void _file_saved(const String &p_path) {
		if (target.is_valid()) {
			target->save_to_path(p_path);
		}
	}

	void _rebake_pressed() {
		if (target.is_valid()) {
			target->bake_frames();
			_refresh();
		}
	}

	void _play_toggled(bool p_pressed) {
		if (target.is_valid()) {
			target->set_play(p_pressed);
		}
		if (play_btn) {
			play_btn->set_text(p_pressed ? TTR("Stop") : TTR("Play"));
			if (preview) {
				preview->queue_redraw();
			}
		}
	}

	void _loop_toggled(bool p_pressed) {
		if (target.is_valid()) {
			target->set_loop(p_pressed);
		}
	}

	void _frame_changed(double p_value) {
		if (target.is_valid()) {
			target->set_current_frame(int(p_value));
		}
		if (play_btn && target.is_valid()) {
			const bool playing = target->get_play();
			play_btn->set_text(playing ? TTR("Stop") : TTR("Play"));
			play_btn->set_pressed_no_signal(playing);
		}
		if (preview) {
			preview->queue_redraw();
		}
	}

	void _refresh() {
		if (target.is_null()) {
			return;
		}
		if (info) {
			info->set_text(vformat("%d frames, %dx%d, loop %d", target->get_frame_count(), target->get_canvas_size().x, target->get_canvas_size().y, target->get_netscape_loop_count()));
		}
		if (play_btn) {
			const bool playing = target->get_play();
			play_btn->set_pressed_no_signal(playing);
			play_btn->set_text(playing ? TTR("Stop") : TTR("Play"));
		}
		if (loop_btn) {
			loop_btn->set_pressed_no_signal(target->get_loop());
		}
		if (frame_slider) {
			frame_slider->set_block_signals(true);
			frame_slider->set_max(MAX(0, target->get_frame_count() - 1));
			frame_slider->set_value(target->get_current_frame());
			frame_slider->set_block_signals(false);
		}
	}

protected:
	void _notification(int p_what) {
		switch (p_what) {
			case NOTIFICATION_ENTER_TREE:
			case NOTIFICATION_VISIBILITY_CHANGED: {
				set_process(is_visible_in_tree());
			} break;
			case NOTIFICATION_PROCESS: {
				if (target.is_valid() && target->get_play() && frame_slider) {
					frame_slider->set_block_signals(true);
					frame_slider->set_value(target->get_current_frame());
					frame_slider->set_block_signals(false);
				}
				if (preview) {
					preview->queue_redraw();
				}
			} break;
		}
	}

public:
	void set_target(const Ref<GIFTexture> &p_texture) {
		target = p_texture;
		if (preview) {
			preview->set_texture(target);
		}
		_refresh();
	}

	GIFInspectorControls() {
		set_process(true);
		preview = memnew(TextureRect);
		preview->set_custom_minimum_size(Size2(0, 160) * EDSCALE);
		preview->set_stretch_mode(TextureRect::STRETCH_KEEP_ASPECT_CENTERED);
		preview->set_expand_mode(TextureRect::EXPAND_IGNORE_SIZE);
		add_child(preview);

		info = memnew(Label);
		add_child(info);

		HBoxContainer *row = memnew(HBoxContainer);
		add_child(row);

		play_btn = memnew(Button);
		play_btn->set_toggle_mode(true);
		play_btn->connect(SceneStringName(toggled), callable_mp(this, &GIFInspectorControls::_play_toggled));
		play_btn->set_custom_minimum_size(Size2(80, 0) * EDSCALE);
		row->add_child(play_btn);

		loop_btn = memnew(CheckButton);
		loop_btn->set_text(TTR("Loop"));
		loop_btn->connect(SceneStringName(toggled), callable_mp(this, &GIFInspectorControls::_loop_toggled));
		row->add_child(loop_btn);

		HBoxContainer *slider_row = memnew(HBoxContainer);
		add_child(slider_row);
		Label *frame_label = memnew(Label);
		frame_label->set_text(TTR("Frame"));
		slider_row->add_child(frame_label);
		frame_slider = memnew(HSlider);
		frame_slider->set_min(0);
		frame_slider->set_step(1);
		frame_slider->set_h_size_flags(Control::SIZE_EXPAND_FILL);
		frame_slider->set_v_size_flags(Control::SIZE_SHRINK_CENTER);
		frame_slider->set_custom_minimum_size(Size2(0, 20) * EDSCALE);
		frame_slider->connect(SNAME("value_changed"), callable_mp(this, &GIFInspectorControls::_frame_changed));
		slider_row->add_child(frame_slider);

		HBoxContainer *btns = memnew(HBoxContainer);
		add_child(btns);
		Button *load = memnew(Button);
		load->set_text(TTR("Load"));
		load->connect(SceneStringName(pressed), callable_mp(this, &GIFInspectorControls::_load_pressed));
		btns->add_child(load);
		Button *save = memnew(Button);
		save->set_text(TTR("Save as GIF"));
		save->connect(SceneStringName(pressed), callable_mp(this, &GIFInspectorControls::_save_pressed));
		btns->add_child(save);
		Button *rebake = memnew(Button);
		rebake->set_text(TTR("Rebake"));
		rebake->connect(SceneStringName(pressed), callable_mp(this, &GIFInspectorControls::_rebake_pressed));
		btns->add_child(rebake);

		_refresh();
	}
};

bool EditorInspectorPluginGIF::can_handle(Object *p_object) {
	return Object::cast_to<GIFTexture>(p_object) != nullptr;
}

void EditorInspectorPluginGIF::parse_begin(Object *p_object) {
	Ref<GIFTexture> tex = Object::cast_to<GIFTexture>(p_object);
	if (tex.is_null()) {
		return;
	}
	GIFInspectorControls *controls = memnew(GIFInspectorControls);
	controls->set_target(tex);
	add_custom_control(controls);
}

bool GIFPreviewGenerator::handles(const String &p_type) const {
	return p_type == "GIFTexture";
}

Ref<Texture2D> GIFPreviewGenerator::generate(const Ref<Resource> &p_from, const Size2 &p_size, Dictionary &p_metadata) const {
	Ref<GIFTexture> tex = p_from;
	if (tex.is_null() || tex->get_frame_count() == 0) {
		return Ref<Texture2D>();
	}
	Ref<Image> img = tex->get_baked_image(0);
	if (img.is_null()) {
		img = tex->get_source_image(0);
	}
	if (img.is_null()) {
		return Ref<Texture2D>();
	}
	img = img->duplicate();
	if (img->is_compressed()) {
		img->decompress();
	}
	if (p_size.x > 0 && p_size.y > 0) {
		img->resize(MAX(1, int(p_size.x)), MAX(1, int(p_size.y)), Image::INTERPOLATE_BILINEAR);
	}
	return ImageTexture::create_from_image(img);
}

Viewport *GIFEditorPlugin::_get_active_editor_viewport() const {
	EditorInterface *ei = EditorInterface::get_singleton();
	if (!ei) {
		return nullptr;
	}
	EditorMainScreen *screen = EditorNode::get_editor_main_screen();
	const int selected = screen ? screen->get_selected_index() : -1;
	if (selected == EditorMainScreen::EDITOR_3D) {
		return ei->get_editor_viewport_3d(0);
	}
	if (selected == EditorMainScreen::EDITOR_2D) {
		return ei->get_editor_viewport_2d();
	}
	return nullptr;
}

void GIFEditorPlugin::_set_status(const String &p_text) {
	if (status_label) {
		status_label->set_text(p_text);
	}
}

bool GIFEditorPlugin::_capture_active() const {
	return capturing_game || capturing_running || (recorder.is_valid() && recorder->is_recording());
}

void GIFEditorPlugin::_refresh_record_controls() {
	const bool active = _capture_active();
	if (viewport_button) {
		viewport_button->set_disabled(active);
	}
	if (game_button) {
		game_button->set_disabled(active);
	}
	if (window_button) {
		window_button->set_disabled(active);
	}
	if (running_button) {
		running_button->set_disabled(active);
	}
	if (pause_button) {
		pause_button->set_disabled(!active);
		const bool paused = recorder.is_valid() && recorder->is_paused();
		pause_button->set_text(paused ? TTR("Resume") : TTR("Pause"));
	}
	if (stop_button) {
		stop_button->set_disabled(!active);
	}
}

void GIFEditorPlugin::_save_texture(const Ref<GIFTexture> &p_texture) {
	if (p_texture.is_null()) {
		_set_status(TTR("Recording produced no frames."));
		return;
	}
	const String path = save_path_edit ? save_path_edit->get_text().strip_edges() : String();
	if (path.is_empty()) {
		if (!save_dialog) {
			_set_status(TTR("Choose a save location."));
			return;
		}
		choosing_save_path = false;
		save_dialog->set_meta("gif_texture", p_texture);
		save_dialog->set_current_file("capture.gif");
		save_dialog->popup_file_dialog();
		_set_status(TTR("Choose a save location."));
		return;
	}
	const Error err = p_texture->save_to_path(path);
	if (err == OK) {
		_set_status(vformat(TTR("Saved GIF: %s"), path));
	} else {
		_set_status(TTR("Failed to save GIF."));
	}
}

void GIFEditorPlugin::_stop_screen_capture() {
	capturing_game = false;
	capturing_running = false;
	SceneTree *tree = Object::cast_to<SceneTree>(OS::get_singleton() ? OS::get_singleton()->get_main_loop() : nullptr);
	if (tree && tree->is_connected(SNAME("process_frame"), callable_mp(this, &GIFEditorPlugin::_process_screen_capture))) {
		tree->disconnect(SNAME("process_frame"), callable_mp(this, &GIFEditorPlugin::_process_screen_capture));
	}
}

void GIFEditorPlugin::_on_pause_pressed() {
	if (!_capture_active() || recorder.is_null()) {
		return;
	}
	const bool paused = !recorder->is_paused();
	recorder->set_paused(paused);
	if (paused) {
		_set_status(vformat(TTR("Paused (%s)."), active_source));
	} else {
		_set_status(vformat(TTR("Recording %s."), active_source));
	}
	_refresh_record_controls();
}

void GIFEditorPlugin::_on_stop_pressed() {
	if (!_capture_active()) {
		return;
	}
	Ref<GIFTexture> tex;
	if (capturing_game || capturing_running) {
		_stop_screen_capture();
	}
	if (recorder.is_valid()) {
		recorder->set_paused(false);
		tex = recorder->stop();
	}
	active_source = String();
	_refresh_record_controls();
	pending_save_kind = "record";
	_save_texture(tex);
}

void GIFEditorPlugin::_on_browse_save_path() {
	if (!save_dialog) {
		return;
	}
	choosing_save_path = true;
	const String current = save_path_edit ? save_path_edit->get_text().strip_edges() : String();
	if (!current.is_empty()) {
		save_dialog->set_current_path(current);
	}
	save_dialog->popup_file_dialog();
}

void GIFEditorPlugin::_toggle_recording(GIFRecorder::Source p_source, Viewport *p_viewport) {
	if (_capture_active()) {
		return;
	}
	recorder.instantiate();
	Error err = OK;
	if (p_source == GIFRecorder::SOURCE_WINDOW) {
		active_source = TTR("editor window");
		err = recorder->start_window();
	} else if (p_viewport) {
		active_source = TTR("2D/3D viewport");
		err = recorder->start_viewport(p_viewport);
	} else {
		err = ERR_UNCONFIGURED;
	}
	if (err != OK) {
		recorder.unref();
		active_source = String();
		_set_status(TTR("Could not start GIF recording."));
		_refresh_record_controls();
		return;
	}
	_set_status(vformat(TTR("Recording %s."), active_source));
	_refresh_record_controls();
}

void GIFEditorPlugin::_record_editor_viewport() {
	if (_capture_active()) {
		return;
	}
	Viewport *vp = _get_active_editor_viewport();
	if (!vp) {
		_set_status(TTR("Open the 2D or 3D editor to record that viewport."));
		return;
	}
	_toggle_recording(GIFRecorder::SOURCE_VIEWPORT, vp);
}

void GIFEditorPlugin::_process_screen_capture() {
	if (recorder.is_null() || recorder->is_paused() || !DisplayServer::get_singleton()) {
		return;
	}
	Ref<Image> shot;
	if (capturing_game) {
		Control *gv = Object::cast_to<Control>(ObjectDB::get_instance(game_view_id));
		if (!gv) {
			return;
		}
		const Rect2 rect = gv->get_global_rect();
		Window *host = gv->get_window();
		const DisplayServer::WindowID window_id = host ? host->get_window_id() : DisplayServer::MAIN_WINDOW_ID;
		const Vector2i win = DisplayServer::get_singleton()->window_get_position(window_id);
		shot = DisplayServer::get_singleton()->screen_get_image_rect(Rect2i(win + Vector2i(rect.position), Vector2i(rect.size)));
	} else if (capturing_running) {
		if (!EditorRunBar::get_singleton()) {
			return;
		}
		const Rect2i rect = DisplayServer::get_singleton()->window_get_process_rect(EditorRunBar::get_singleton()->get_current_process());
		if (rect.size.x <= 0 || rect.size.y <= 0) {
			return;
		}
		shot = DisplayServer::get_singleton()->screen_get_image_rect(rect);
	}
	if (shot.is_valid()) {
		recorder->add_frame(shot);
	}
}

void GIFEditorPlugin::_record_editor_game_window() {
	if (_capture_active()) {
		return;
	}
	GameView *gv = nullptr;
	if (EditorNode::get_singleton() && EditorNode::get_singleton()->get_gui_base()) {
		TypedArray<Node> nodes = EditorNode::get_singleton()->get_gui_base()->find_children("*", "GameView", true, false);
		if (nodes.size()) {
			gv = Object::cast_to<GameView>(nodes[0]);
		}
	}
	if (!gv || !gv->is_inside_tree()) {
		_set_status(TTR("Editor Game Window is not available."));
		return;
	}
	recorder.instantiate();
	recorder->set_paused(false);
	game_view_id = gv->get_instance_id();
	capturing_game = true;
	active_source = TTR("editor Game window");
	SceneTree *tree = Object::cast_to<SceneTree>(OS::get_singleton()->get_main_loop());
	if (tree && !tree->is_connected(SNAME("process_frame"), callable_mp(this, &GIFEditorPlugin::_process_screen_capture))) {
		tree->connect(SNAME("process_frame"), callable_mp(this, &GIFEditorPlugin::_process_screen_capture));
	}
	_set_status(vformat(TTR("Recording %s."), active_source));
	_refresh_record_controls();
}

void GIFEditorPlugin::_record_editor_window() {
	_toggle_recording(GIFRecorder::SOURCE_WINDOW, nullptr);
}

void GIFEditorPlugin::_record_running_game() {
	if (_capture_active()) {
		return;
	}
	if (!EditorRunBar::get_singleton() || !EditorRunBar::get_singleton()->is_playing()) {
		_set_status(TTR("Start the project before recording the running game."));
		return;
	}
	if (!DisplayServer::get_singleton()) {
		_set_status(TTR("Running game window was not found."));
		return;
	}
	const Rect2i rect = DisplayServer::get_singleton()->window_get_process_rect(EditorRunBar::get_singleton()->get_current_process());
	if (rect.size.x <= 0 || rect.size.y <= 0) {
		_set_status(TTR("Running game window was not found."));
		return;
	}
	recorder.instantiate();
	recorder->set_paused(false);
	capturing_running = true;
	active_source = TTR("running game");
	SceneTree *tree = Object::cast_to<SceneTree>(OS::get_singleton()->get_main_loop());
	if (tree && !tree->is_connected(SNAME("process_frame"), callable_mp(this, &GIFEditorPlugin::_process_screen_capture))) {
		tree->connect(SNAME("process_frame"), callable_mp(this, &GIFEditorPlugin::_process_screen_capture));
	}
	_set_status(vformat(TTR("Recording %s."), active_source));
	_refresh_record_controls();
}

void GIFEditorPlugin::_export_animation_player() {
	EditorSelection *sel = EditorInterface::get_singleton()->get_selection();
	ERR_FAIL_NULL(sel);
	AnimationPlayer *ap = nullptr;
	for (Node *n : sel->get_selected_node_list()) {
		ap = Object::cast_to<AnimationPlayer>(n);
		if (ap) {
			break;
		}
	}
	if (!ap) {
		_set_status(TTR("Select an AnimationPlayer to export as GIF."));
		return;
	}
	const StringName current = ap->get_assigned_animation();
	if (String(current).is_empty()) {
		_set_status(TTR("AnimationPlayer has no current animation."));
		return;
	}
	Ref<Animation> anim = ap->get_animation(current);
	ERR_FAIL_COND(anim.is_null());
	Viewport *vp = _get_active_editor_viewport();
	if (!vp) {
		_set_status(TTR("No editor viewport is available to record."));
		return;
	}
	const double length = anim->get_length();
	const double step = MAX(1.0 / 12.0, anim->get_step() > 0.0 ? anim->get_step() : 1.0 / 12.0);
	Ref<GIFRecorder> rec;
	rec.instantiate();
	rec->set_fps(int(Math::round(1.0 / step)));
	for (double t = 0.0; t <= length + 0.0001; t += step) {
		ap->seek(t, true, true);
		if (vp->get_texture().is_valid()) {
			rec->add_frame(vp->get_texture()->get_image());
		}
	}
	Ref<GIFTexture> result = rec->stop();
	if (result.is_null()) {
		result.instantiate();
	}
	pending_save_kind = "anim";
	_save_texture(result);
}

void GIFEditorPlugin::_save_dialog_file_selected(const String &p_path) {
	if (choosing_save_path) {
		choosing_save_path = false;
		if (save_path_edit) {
			save_path_edit->set_text(p_path);
		}
		_set_status(vformat(TTR("Save location: %s"), p_path));
		return;
	}
	Ref<GIFTexture> tex = save_dialog->get_meta("gif_texture");
	if (tex.is_valid()) {
		const Error err = tex->save_to_path(p_path);
		if (save_path_edit) {
			save_path_edit->set_text(p_path);
		}
		if (err == OK) {
			_set_status(vformat(TTR("Saved GIF: %s"), p_path));
		} else {
			_set_status(TTR("Failed to save GIF."));
		}
	}
}

GIFEditorPlugin::GIFEditorPlugin() {
	inspector_plugin.instantiate();
	add_inspector_plugin(inspector_plugin);
	preview_generator.instantiate();
	EditorResourcePreview::get_singleton()->add_preview_generator(preview_generator);

	record_panel = memnew(VBoxContainer);
	status_label = memnew(Label);
	status_label->set_text(TTR("Idle."));
	status_label->set_autowrap_mode(TextServer::AUTOWRAP_WORD_SMART);
	record_panel->add_child(status_label);

	HBoxContainer *path_row = memnew(HBoxContainer);
	save_path_edit = memnew(LineEdit);
	save_path_edit->set_placeholder(TTR("Save location"));
	save_path_edit->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	path_row->add_child(save_path_edit);
	Button *browse_button = memnew(Button);
	browse_button->set_text(TTR("Browse..."));
	browse_button->connect(SceneStringName(pressed), callable_mp(this, &GIFEditorPlugin::_on_browse_save_path));
	path_row->add_child(browse_button);
	record_panel->add_child(path_row);

	game_button = memnew(Button);
	game_button->set_text(TTR("Record Editor Game Window"));
	game_button->connect(SceneStringName(pressed), callable_mp(this, &GIFEditorPlugin::_record_editor_game_window));
	record_panel->add_child(game_button);

	window_button = memnew(Button);
	window_button->set_text(TTR("Record Editor Window"));
	window_button->connect(SceneStringName(pressed), callable_mp(this, &GIFEditorPlugin::_record_editor_window));
	record_panel->add_child(window_button);

	running_button = memnew(Button);
	running_button->set_text(TTR("Record Running Game"));
	running_button->connect(SceneStringName(pressed), callable_mp(this, &GIFEditorPlugin::_record_running_game));
	record_panel->add_child(running_button);

	viewport_button = memnew(Button);
	viewport_button->set_text(TTR("Record 2D/3D Viewport"));
	viewport_button->connect(SceneStringName(pressed), callable_mp(this, &GIFEditorPlugin::_record_editor_viewport));
	record_panel->add_child(viewport_button);

	HBoxContainer *control_row = memnew(HBoxContainer);
	pause_button = memnew(Button);
	pause_button->set_text(TTR("Pause"));
	pause_button->set_disabled(true);
	pause_button->connect(SceneStringName(pressed), callable_mp(this, &GIFEditorPlugin::_on_pause_pressed));
	control_row->add_child(pause_button);
	stop_button = memnew(Button);
	stop_button->set_text(TTR("Stop"));
	stop_button->set_disabled(true);
	stop_button->connect(SceneStringName(pressed), callable_mp(this, &GIFEditorPlugin::_on_stop_pressed));
	control_row->add_child(stop_button);
	record_panel->add_child(control_row);

	export_button = memnew(Button);
	export_button->set_text(TTR("Export AnimationPlayer"));
	export_button->connect(SceneStringName(pressed), callable_mp(this, &GIFEditorPlugin::_export_animation_player));
	record_panel->add_child(export_button);

	add_blazium_window("GIF", "Record", record_panel);

	save_dialog = memnew(EditorFileDialog);
	save_dialog->set_file_mode(EditorFileDialog::FILE_MODE_SAVE_FILE);
	save_dialog->set_access(EditorFileDialog::ACCESS_FILESYSTEM);
	save_dialog->add_filter("*.gif", "GIF");
	save_dialog->connect("file_selected", callable_mp(this, &GIFEditorPlugin::_save_dialog_file_selected));
	EditorNode::get_singleton()->get_gui_base()->add_child(save_dialog);
}

GIFEditorPlugin::~GIFEditorPlugin() {
	const bool screen_was_capturing = capturing_game || capturing_running;
	if (screen_was_capturing) {
		_stop_screen_capture();
	}
	if (recorder.is_valid() && (recorder->is_recording() || screen_was_capturing)) {
		recorder->stop();
	}
	if (save_dialog && save_dialog->has_meta("gif_texture")) {
		save_dialog->remove_meta("gif_texture");
	}
	if (record_panel) {
		remove_blazium_item("GIF", "Record");
		memdelete(record_panel);
		record_panel = nullptr;
		status_label = nullptr;
		save_path_edit = nullptr;
		viewport_button = nullptr;
		game_button = nullptr;
		window_button = nullptr;
		running_button = nullptr;
		pause_button = nullptr;
		stop_button = nullptr;
		export_button = nullptr;
	}
	if (inspector_plugin.is_valid()) {
		remove_inspector_plugin(inspector_plugin);
		inspector_plugin.unref();
	}
	if (EditorResourcePreview::get_singleton() && preview_generator.is_valid()) {
		EditorResourcePreview::get_singleton()->remove_preview_generator(preview_generator);
		preview_generator.unref();
	}
	recorder.unref();
}

#endif
