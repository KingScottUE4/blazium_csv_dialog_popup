/**************************************************************************/
/*  editor_split_view.cpp                                                 */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
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

#include "editor_split_view.h"

#include "core/config/project_settings.h"
#include "core/input/input.h"
#include "core/input/input_event.h"
#include "core/io/resource_loader.h"
#include "core/object/callable_mp.h"
#include "core/object/class_db.h"
#include "core/object/script_language.h"
#include "editor/docks/groups_dock.h"
#include "editor/docks/inspector_dock.h"
#include "editor/docks/scene_tree_dock.h"
#include "editor/docks/signals_dock.h"
#include "editor/editor_data.h"
#include "editor/editor_interface.h"
#include "editor/editor_main_screen.h"
#include "editor/editor_node.h"
#include "editor/editor_string_names.h"
#include "editor/editor_undo_redo_manager.h"
#include "editor/gui/code_editor.h"
#include "editor/gui/create_dialog.h"
#include "editor/gui/editor_quick_open_dialog.h"
#include "editor/inspector/editor_inspector.h"
#include "editor/scene/3d/node_3d_editor_plugin.h"
#include "editor/scene/canvas_item_editor_plugin.h"
#include "editor/scene/editor_scene_tabs.h"
#include "editor/scene/editor_split_view_tools.h"
#include "editor/scene/scene_tree_editor.h"
#include "editor/script/script_create_dialog.h"
#include "editor/script/script_editor_base.h"
#include "editor/script/script_editor_plugin.h"
#include "editor/script/syntax_highlighters.h"
#include "editor/settings/editor_settings.h"
#include "editor/themes/editor_scale.h"
#include "scene/3d/camera_3d.h"
#include "scene/3d/decal.h"
#include "scene/3d/light_3d.h"
#include "scene/3d/mesh_instance_3d.h"
#include "scene/3d/node_3d.h"
#include "scene/3d/visual_instance_3d.h"
#include "scene/3d/world_environment.h"
#include "scene/gui/button.h"
#include "scene/gui/code_edit.h"
#include "scene/gui/label.h"
#include "scene/gui/line_edit.h"
#include "scene/gui/menu_button.h"
#include "scene/gui/panel.h"
#include "scene/gui/panel_container.h"
#include "scene/gui/popup_menu.h"
#include "scene/gui/subviewport_container.h"
#include "scene/main/scene_tree.h"
#include "scene/main/viewport.h"
#include "scene/main/window.h"
#include "scene/resources/3d/sky_material.h"
#include "scene/resources/environment.h"
#include "scene/resources/material.h"
#include "scene/resources/mesh.h"
#include "scene/resources/packed_scene.h"
#include "scene/resources/sky.h"
#include "scene/resources/style_box_flat.h"
#include "servers/display/display_server.h"
#include "servers/rendering/rendering_server.h"

// The look of the 3D editor's viewport menu and panels, drawn over the view.
static void _split_view_dark_background_style(Control *p_control) {
	const Ref<StyleBox> stylebox = EditorNode::get_singleton()->get_gui_base()->get_theme_stylebox(SNAME("Information3dViewport"), EditorStringName(EditorStyles));
	const Ref<Theme> &editor_theme = EditorNode::get_singleton()->get_editor_theme();
	p_control->begin_bulk_theme_override();
	static const char *styles[] = { "normal", "normal_mirrored", "hover", "hover_mirrored", "hover_pressed", "hover_pressed_mirrored", "pressed", "pressed_mirrored", "focus", "focus_mirrored", "disabled", "disabled_mirrored", "panel" };
	for (const char *style : styles) {
		p_control->add_theme_style_override(style, stylebox);
	}
	p_control->add_theme_color_override(SceneStringName(font_color), editor_theme->get_color(SNAME("font_dark_background_color"), EditorStringName(Editor)));
	p_control->add_theme_color_override("font_hover_color", editor_theme->get_color(SNAME("font_dark_background_hover_color"), EditorStringName(Editor)));
	p_control->add_theme_color_override("font_focus_color", editor_theme->get_color(SNAME("font_dark_background_focus_color"), EditorStringName(Editor)));
	p_control->add_theme_color_override("font_pressed_color", editor_theme->get_color(SNAME("font_dark_background_pressed_color"), EditorStringName(Editor)));
	p_control->add_theme_color_override("font_hover_pressed_color", editor_theme->get_color(SNAME("font_dark_background_hover_pressed_color"), EditorStringName(Editor)));
	p_control->end_bulk_theme_override();
}

// EditorScenePane

void EditorScenePane::_draw_dot() {
	const Color color = active ? get_theme_color(SNAME("accent_color"), EditorStringName(Editor)) : get_theme_color(SNAME("font_disabled_color"), EditorStringName(Editor));
	const Size2 size = dot->get_size();
	dot->draw_circle(size / 2, MIN(size.x, size.y) / 2, color);
}

void EditorScenePane::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_THEME_CHANGED: {
			_update_style();
			const Color dim = get_theme_color(SNAME("font_disabled_color"), EditorStringName(Editor));
			type_label->add_theme_color_override(SceneStringName(font_color), dim);
			tree_title->add_theme_color_override(SceneStringName(font_color), dim);
			inspector_title->add_theme_color_override(SceneStringName(font_color), dim);
			file_label->add_theme_font_override(SceneStringName(font), get_theme_font(SNAME("bold"), EditorStringName(EditorFonts)));
			if (pick_button) {
				pick_button->add_theme_font_override(SceneStringName(font), get_theme_font(SNAME("bold"), EditorStringName(EditorFonts)));
				pick_button->set_button_icon(get_theme_icon(SNAME("arrow"), SNAME("OptionButton")));
			}
			if (close_button) {
				close_button->set_button_icon(get_editor_theme_icon(SNAME("Close")));
			}
			if (code_button) {
				code_button->set_button_icon(get_editor_theme_icon(SNAME("Script")));
			}
			if (swap_button) {
				swap_button->set_button_icon(get_editor_theme_icon(SNAME("MirrorX")));
			}
			if (float_button) {
				float_button->set_button_icon(get_editor_theme_icon(SNAME("MakeFloating")));
			}
			if (tree_toolbar) {
				add_node_button->set_button_icon(get_editor_theme_icon(SNAME("Add")));
				instantiate_button->set_button_icon(get_editor_theme_icon(SNAME("Instance")));
				tree_filter->set_right_icon(get_editor_theme_icon(SNAME("Search")));
				attach_script_button->set_button_icon(get_editor_theme_icon(SNAME("ScriptCreate")));
				detach_script_button->set_button_icon(get_editor_theme_icon(SNAME("ScriptRemove")));
				tree_menu->set_button_icon(get_editor_theme_icon(SNAME("GuiTabMenuHl")));
			}
			if (view_menu) {
				view_menu->set_button_icon(get_editor_theme_icon(SNAME("GuiTabMenuHlDarkBackground")));
				_split_view_dark_background_style(view_menu);
				_split_view_dark_background_style(info_panel);
				_split_view_dark_background_style(info_label);
				_split_view_dark_background_style(frame_time_panel);
			}
			dot->queue_redraw();
		} break;
	}
}

void EditorScenePane::_update_style() {
	Ref<StyleBoxFlat> header_style;
	header_style.instantiate();
	header_style->set_bg_color(get_theme_color(active ? SNAME("dark_color_1") : SNAME("dark_color_2"), EditorStringName(Editor)));
	header_style->set_content_margin_individual(8 * EDSCALE, 4 * EDSCALE, 4 * EDSCALE, 4 * EDSCALE);
	if (active) {
		// The side being edited gets the accent edge of a selected tab.
		const int edge = MAX(1, int(Math::round(2 * EDSCALE)));
		header_style->set_border_width(SIDE_TOP, edge);
		header_style->set_border_color(get_theme_color(SNAME("accent_color"), EditorStringName(Editor)));
		header_style->set_content_margin(SIDE_TOP, 4 * EDSCALE - edge);
	}
	header->add_theme_style_override(SceneStringName(panel), header_style);
	state_label->add_theme_color_override(SceneStringName(font_color), active ? get_theme_color(SNAME("accent_color"), EditorStringName(Editor)) : get_theme_color(SNAME("font_disabled_color"), EditorStringName(Editor)));
	dot->queue_redraw();
}

void EditorScenePane::set_active(bool p_active, bool p_editable) {
	if (active == p_active && editable == p_editable && !state_label->get_text().is_empty()) {
		return;
	}
	active = p_active;
	editable = p_editable;
	if (active) {
		state_label->set_text(TTRC("Editing"));
		state_label->set_tooltip_text(TTRC("Undo, redo and shortcuts go to this side."));
	} else if (editable) {
		state_label->set_text(TTRC("Editable"));
		state_label->set_tooltip_text(TTRC("Click this side to edit it."));
	} else {
		state_label->set_text(TTRC("View only"));
		state_label->set_tooltip_text(TTRC("This side is for looking. Click it to edit it."));
	}
	if (is_inside_tree()) {
		_update_style();
	}
}

void EditorScenePane::set_title(const String &p_title) {
	file_label->set_text(p_title);
	if (pick_button) {
		pick_button->set_text(p_title);
	}
}

EditorScenePane::EditorScenePane(bool p_focused) {
	focused = p_focused;
	set_h_size_flags(SIZE_EXPAND_FILL);
	set_v_size_flags(SIZE_EXPAND_FILL);
	add_theme_constant_override("separation", 0);

	header = memnew(PanelContainer);
	add_child(header);
	HBoxContainer *header_hb = memnew(HBoxContainer);
	header->add_child(header_hb);

	dot = memnew(Control);
	dot->set_custom_minimum_size(Size2(8, 8) * EDSCALE);
	dot->set_v_size_flags(SIZE_SHRINK_CENTER);
	dot->set_mouse_filter(MOUSE_FILTER_IGNORE);
	dot->connect(SceneStringName(draw), callable_mp(this, &EditorScenePane::_draw_dot));
	header_hb->add_child(dot);

	file_label = memnew(Label);
	file_label->set_auto_translate_mode(AUTO_TRANSLATE_MODE_DISABLED);
	header_hb->add_child(file_label);

	if (!focused) {
		file_label->hide();
		pick_button = memnew(MenuButton);
		pick_button->set_flat(true);
		pick_button->set_auto_translate_mode(AUTO_TRANSLATE_MODE_DISABLED);
		pick_button->set_icon_alignment(HORIZONTAL_ALIGNMENT_RIGHT);
		pick_button->set_tooltip_text(TTRC("Choose what to show here."));
		pick_button->set_accessibility_name(TTRC("Choose Split View Content"));
		header_hb->add_child(pick_button);
	}

	type_label = memnew(Label);
	type_label->set_auto_translate_mode(AUTO_TRANSLATE_MODE_DISABLED);
	type_label->set_h_size_flags(SIZE_EXPAND_FILL);
	type_label->set_text_overrun_behavior(TextServer::OVERRUN_TRIM_ELLIPSIS);
	header_hb->add_child(type_label);

	state_label = memnew(Label);
	state_label->set_mouse_filter(MOUSE_FILTER_PASS);
	header_hb->add_child(state_label);

	if (!focused) {
		code_button = memnew(Button);
		code_button->set_flat(true);
		code_button->set_toggle_mode(true);
		code_button->set_tooltip_text(TTRC("Show another open script here instead of the scene. The script you switch away from moves here; click it to edit it."));
		code_button->set_accessibility_name(TTRC("Compare Scripts"));
		code_button->hide();
		header_hb->add_child(code_button);

		swap_button = memnew(Button);
		swap_button->set_flat(true);
		swap_button->set_tooltip_text(TTRC("Edit the other side: the two scenes (or scripts) trade places, and the panes stay where they are."));
		swap_button->set_accessibility_name(TTRC("Swap Split View Sides"));
		swap_button->set_shortcut(ED_SHORTCUT("editor/swap_split_view_sides", TTRC("Swap Split View Sides"), KeyModifierMask::CMD_OR_CTRL | KeyModifierMask::ALT | KeyModifierMask::SHIFT | Key::BACKSLASH));
		swap_button->set_shortcut_in_tooltip(true);
		header_hb->add_child(swap_button);

		float_button = memnew(Button);
		float_button->set_flat(true);
		float_button->set_toggle_mode(true);
		float_button->set_tooltip_text(TTRC("Show this scene in a window of its own. Close the window to put it back beside the current scene."));
		float_button->set_accessibility_name(TTRC("Float Split View"));
		header_hb->add_child(float_button);

		close_button = memnew(Button);
		close_button->set_flat(true);
		close_button->set_tooltip_text(TTRC("Close the split view."));
		close_button->set_accessibility_name(TTRC("Close Split View"));
		header_hb->add_child(close_button);
	}

	HBoxContainer *body = memnew(HBoxContainer);
	body->set_v_size_flags(SIZE_EXPAND_FILL);
	body->add_theme_constant_override("separation", 0);
	add_child(body);

	tree_strip = memnew(VBoxContainer);
	tree_strip->set_custom_minimum_size(Size2(focused ? 160 : 200, 0) * EDSCALE);
	body->add_child(tree_strip);
	tree_title = memnew(Label);
	tree_title->set_text(TTRC("Scene"));
	tree_title->set_theme_type_variation("HeaderSmall");
	tree_strip->add_child(tree_title);
	if (!focused) {
		// The Scene dock's toolbar, for this pane's scene.
		tree_title->hide();
		tree_toolbar = memnew(HBoxContainer);
		tree_toolbar->add_theme_constant_override("separation", 0);
		tree_strip->add_child(tree_toolbar);

		add_node_button = memnew(Button);
		add_node_button->set_theme_type_variation("FlatMenuButton");
		add_node_button->set_tooltip_text(TTRC("Add/Create a New Node."));
		add_node_button->set_accessibility_name(TTRC("Add Child Node"));
		tree_toolbar->add_child(add_node_button);

		instantiate_button = memnew(Button);
		instantiate_button->set_theme_type_variation("FlatMenuButton");
		instantiate_button->set_tooltip_text(TTRC("Instantiate a scene file as a Node."));
		instantiate_button->set_accessibility_name(TTRC("Instantiate Child Scene"));
		tree_toolbar->add_child(instantiate_button);

		tree_filter = memnew(LineEdit);
		tree_filter->set_h_size_flags(SIZE_EXPAND_FILL);
		tree_filter->set_placeholder(TTRC("Filter Nodes"));
		tree_filter->set_accessibility_name(TTRC("Filter Nodes"));
		tree_filter->set_clear_button_enabled(true);
		tree_filter->add_theme_constant_override("minimum_character_width", 0);
		tree_filter->set_tooltip_text(TTRC("Filter nodes by entering a part of their name, type (if prefixed with \"type:\" or \"t:\")\nor group (if prefixed with \"group:\" or \"g:\"). Filtering is case-insensitive."));
		tree_toolbar->add_child(tree_filter);

		attach_script_button = memnew(Button);
		attach_script_button->set_theme_type_variation("FlatMenuButton");
		attach_script_button->set_tooltip_text(TTRC("Attach a new or existing script to the selected node."));
		attach_script_button->set_accessibility_name(TTRC("Attach Script"));
		tree_toolbar->add_child(attach_script_button);

		detach_script_button = memnew(Button);
		detach_script_button->set_theme_type_variation("FlatMenuButton");
		detach_script_button->set_tooltip_text(TTRC("Detach the script from the selected node."));
		detach_script_button->set_accessibility_name(TTRC("Detach Script"));
		detach_script_button->hide();
		tree_toolbar->add_child(detach_script_button);

		tree_menu = memnew(MenuButton);
		tree_menu->set_flat(false);
		tree_menu->set_theme_type_variation("FlatMenuButton");
		tree_menu->set_tooltip_text(TTRC("Extra scene options."));
		tree_toolbar->add_child(tree_menu);
		subresource_menu = memnew(PopupMenu);
		subresource_menu->set_auto_translate_mode(AUTO_TRANSLATE_MODE_DISABLED);
		// Owned by the menu from the start, even if it's never opened.
		tree_menu->get_popup()->add_child(subresource_menu);
	}
	tree = memnew(SceneTreeEditor(false, false, false));
	// The Scene dock's script, visibility and instance buttons.
	tree->set_show_node_buttons(!focused);
	tree->set_v_size_flags(SIZE_EXPAND_FILL);
	tree->set_update_when_invisible(false);
	tree_strip->add_child(tree);

	content = memnew(VBoxContainer);
	content->set_h_size_flags(SIZE_EXPAND_FILL);
	content->set_v_size_flags(SIZE_EXPAND_FILL);
	content->add_theme_constant_override("separation", 0);
	body->add_child(content);

	if (!focused) {
		tools = memnew(EditorPaneTools);
		content->add_child(tools);

		viewport_container = memnew(SubViewportContainer);
		viewport_container->set_stretch(true);
		viewport_container->set_v_size_flags(SIZE_EXPAND_FILL);
		viewport_container->set_h_size_flags(SIZE_EXPAND_FILL);
		viewport_container->set_mouse_filter(MOUSE_FILTER_STOP);
		// For the tools' shortcuts.
		viewport_container->set_focus_mode(FOCUS_CLICK);
		content->add_child(viewport_container);

		viewport = memnew(SubViewport);
		viewport->set_disable_input(true);
		viewport->set_use_own_world_3d(true);
		viewport->set_embedding_subwindows(true);
		viewport->set_update_mode(SubViewport::UPDATE_WHEN_VISIBLE);
		viewport->set_auto_translate_mode(AUTO_TRANSLATE_MODE_ALWAYS);
		viewport->set_translation_domain(StringName());
		viewport_container->add_child(viewport);

		overlay = memnew(Control);
		overlay->set_mouse_filter(MOUSE_FILTER_IGNORE);
		// Handles of things near the camera reach far past the pane.
		overlay->set_clip_contents(true);
		overlay->set_anchors_and_offsets_preset(PRESET_FULL_RECT);
		viewport_container->add_child(overlay);

		// For 3D scenes: the 3D editor's viewport menu and panels. The menu's
		// items are added by EditorSplitView, once the 3D editor's shortcuts exist.
		view_menu = memnew(MenuButton);
		view_menu->set_flat(false);
		view_menu->set_position(Point2(8, 8) * EDSCALE);
		view_menu->set_accessibility_name(TTRC("View"));
		view_menu->set_auto_translate_mode(AUTO_TRANSLATE_MODE_DISABLED);
		view_menu->get_popup()->set_auto_translate_mode(AUTO_TRANSLATE_MODE_ALWAYS);
		view_menu->get_popup()->set_hide_on_checkable_item_selection(false);
		// The pane handles the shortcuts, while it has focus.
		view_menu->set_disable_shortcuts(true);
		view_menu->hide();
		viewport_container->add_child(view_menu);
		view_advanced_menu = memnew(PopupMenu);
		view_advanced_menu->set_hide_on_checkable_item_selection(false);
		// Owned by the menu from the start, even if its items are never added.
		view_menu->get_popup()->add_child(view_advanced_menu);

		info_panel = memnew(PanelContainer);
		info_panel->set_mouse_filter(MOUSE_FILTER_IGNORE);
		info_panel->set_h_grow_direction(GROW_DIRECTION_BEGIN);
		info_panel->set_v_grow_direction(GROW_DIRECTION_BEGIN);
		info_panel->set_anchors_and_offsets_preset(PRESET_BOTTOM_RIGHT, PRESET_MODE_MINSIZE, 8 * EDSCALE);
		// Above the hint along the bottom.
		info_panel->set_offset(SIDE_BOTTOM, -56 * EDSCALE);
		info_panel->set_offset(SIDE_TOP, -56 * EDSCALE);
		info_panel->hide();
		viewport_container->add_child(info_panel);
		info_label = memnew(Label);
		info_label->set_auto_translate_mode(AUTO_TRANSLATE_MODE_DISABLED);
		info_panel->add_child(info_label);

		rotation_control = memnew(EditorPaneRotationControl);
		rotation_control->set_h_grow_direction(GROW_DIRECTION_BEGIN);
		rotation_control->set_anchors_and_offsets_preset(PRESET_TOP_RIGHT, PRESET_MODE_MINSIZE, 8 * EDSCALE);
		rotation_control->hide();
		viewport_container->add_child(rotation_control);

		frame_time_panel = memnew(PanelContainer);
		frame_time_panel->set_mouse_filter(MOUSE_FILTER_IGNORE);
		frame_time_panel->set_h_grow_direction(GROW_DIRECTION_BEGIN);
		frame_time_panel->set_anchors_and_offsets_preset(PRESET_TOP_RIGHT, PRESET_MODE_MINSIZE, 8 * EDSCALE);
		// Below the axis gizmo, as in the 3D editor.
		frame_time_panel->set_offset(SIDE_TOP, (8 + 80 + 8) * EDSCALE);
		frame_time_panel->set_offset(SIDE_BOTTOM, (8 + 80 + 8) * EDSCALE);
		frame_time_panel->hide();
		viewport_container->add_child(frame_time_panel);
		VBoxContainer *frame_time_vbox = memnew(VBoxContainer);
		frame_time_panel->add_child(frame_time_vbox);
		cpu_time_label = memnew(Label);
		frame_time_vbox->add_child(cpu_time_label);
		gpu_time_label = memnew(Label);
		frame_time_vbox->add_child(gpu_time_label);
		fps_label = memnew(Label);
		frame_time_vbox->add_child(fps_label);

		camera_3d = memnew(Camera3D);
		viewport->add_child(camera_3d);
		tools->setup(viewport, camera_3d, overlay);
		rotation_control->set_camera(camera_3d);

		live_container = memnew(SubViewportContainer);
		live_container->set_stretch(true);
		live_container->set_v_size_flags(SIZE_EXPAND_FILL);
		live_container->set_h_size_flags(SIZE_EXPAND_FILL);
		live_container->set_mouse_filter(MOUSE_FILTER_STOP);
		live_container->set_tooltip_text(TTRC("Click to edit this scene in the 2D or 3D editor."));
		live_container->hide();
		content->add_child(live_container);

		live_viewport = memnew(SubViewport);
		live_viewport->set_disable_input(true);
		live_viewport->set_update_mode(SubViewport::UPDATE_WHEN_VISIBLE);
		live_container->add_child(live_viewport);

		live_camera = memnew(Camera3D);
		live_viewport->add_child(live_camera);

		code_container = memnew(VBoxContainer);
		code_container->set_v_size_flags(SIZE_EXPAND_FILL);
		code_container->set_h_size_flags(SIZE_EXPAND_FILL);
		code_container->hide();
		content->add_child(code_container);

		code_view = memnew(CodeEdit);
		code_view->set_v_size_flags(SIZE_EXPAND_FILL);
		code_view->set_editable(false);
		code_view->set_draw_line_numbers(true);
		code_view->set_context_menu_enabled(false);
		code_view->set_tooltip_text(TTRC("Click to edit this script."));
		code_container->add_child(code_view);

		code_placeholder = memnew(Label);
		code_placeholder->set_v_size_flags(SIZE_EXPAND_FILL);
		code_placeholder->set_horizontal_alignment(HORIZONTAL_ALIGNMENT_CENTER);
		code_placeholder->set_vertical_alignment(VERTICAL_ALIGNMENT_CENTER);
		code_placeholder->set_autowrap_mode(TextServer::AUTOWRAP_WORD_SMART);
		code_placeholder->set_text(TTRC("Open another script to compare it here.\nThe script you switch away from appears here."));
		code_container->add_child(code_placeholder);
	}

	inspector_strip = memnew(VBoxContainer);
	inspector_strip->set_custom_minimum_size(Size2(200, 0) * EDSCALE);
	body->add_child(inspector_strip);
	inspector_title = memnew(Label);
	inspector_title->set_text(TTRC("Inspector"));
	inspector_title->set_theme_type_variation("HeaderSmall");
	inspector_title->set_text_overrun_behavior(TextServer::OVERRUN_TRIM_ELLIPSIS);
	inspector_strip->add_child(inspector_title);
	inspector = memnew(EditorInspector);
	inspector->set_v_size_flags(SIZE_EXPAND_FILL);
	inspector->set_use_folding(true);
	inspector->set_use_doc_hints(false);
	inspector->set_hide_script(true);
	inspector->set_hide_metadata(true);
	inspector->set_property_name_style(EditorPropertyNameProcessor::get_default_inspector_style());
	inspector_strip->add_child(inspector);

	set_active(focused);
}

// EditorSplitView

EditorSplitView *EditorSplitView::singleton = nullptr;

bool EditorSplitView::_is_scene_screen() const {
	const EditorMainScreen *ms = Object::cast_to<EditorMainScreen>(main_screen);
	if (!ms || ms->get_current_tab() < 0) {
		return false;
	}
	const EditorDock *dock = ms->get_dock(ms->get_current_tab());
	return dock && (dock == CanvasItemEditor::get_singleton() || dock == Node3DEditor::get_singleton());
}

bool EditorSplitView::_is_script_screen() const {
	const EditorMainScreen *ms = Object::cast_to<EditorMainScreen>(main_screen);
	if (!ms || ms->get_current_tab() < 0) {
		return false;
	}
	return ms->get_dock(ms->get_current_tab()) == ScriptEditor::get_singleton();
}

void EditorSplitView::_set_pane_tree_root(Node *p_root) {
	const ObjectID id = p_root ? p_root->get_instance_id() : ObjectID();
	if (id == pane_tree_root) {
		return;
	}
	pane_tree_root = id;
	preview_pane->tree->set_scene_root_override(p_root);
}

int EditorSplitView::_find_scene_index(ObjectID p_root) const {
	if (p_root.is_null()) {
		return -1;
	}
	EditorData &ed = EditorNode::get_editor_data();
	for (int i = 0; i < ed.get_edited_scene_count(); i++) {
		const Node *root = ed.get_edited_scene_root(i);
		if (root && root->get_instance_id() == p_root) {
			return i;
		}
	}
	return -1;
}

Node *EditorSplitView::get_preview_scene() const {
	if (_find_scene_index(preview_scene) < 0) {
		return nullptr;
	}
	return ObjectDB::get_instance<Node>(preview_scene);
}

void EditorSplitView::_take_into_preview(Node *p_root) {
	ERR_FAIL_NULL(p_root);
	if (p_root->get_parent() == preview_pane->viewport) {
		preview_scene = p_root->get_instance_id();
		return;
	}
	if (p_root->get_parent()) {
		p_root->get_parent()->remove_child(p_root);
	}
	// Before adding it, so its nodes keep their editor behavior from the start.
	preview_pane->viewport->set_editor_preview_scene_root(p_root);
	preview_pane->viewport->add_child(p_root);
	preview_scene = p_root->get_instance_id();
	preview_view_custom = false;

	_setup_preview_lighting(p_root);
	_set_pane_tree_root(p_root);
	preview_pane->tree->update_tree();
	preview_pane->tools->set_scene(p_root);

	// What was selected while it was edited stays selected.
	Node *selected = ObjectDB::get_instance<Node>(last_focus_selected);
	if (selected && (selected == p_root || p_root->is_ancestor_of(selected))) {
		preview_pane->tools->set_selected(selected, false);
	}
	_pane_selection_changed();
	_update_preview_camera();
}

void EditorSplitView::_release_preview() {
	Node *root = ObjectDB::get_instance<Node>(preview_scene);
	preview_pane->tools->set_scene(nullptr);
	preview_pane->inspector->edit(nullptr);
	_set_pane_tree_root(nullptr);
	if (preview_pane->fallback_lighting) {
		memdelete(preview_pane->fallback_lighting);
		preview_pane->fallback_lighting = nullptr;
	}
	if (root && root->get_parent() == preview_pane->viewport) {
		preview_pane->viewport->remove_child(root);
	}
	preview_pane->viewport->set_editor_preview_scene_root(nullptr);
	preview_scene = ObjectID();
}

void EditorSplitView::_collect_bounds(Node *p_node, AABB &r_bounds, bool &r_has_bounds) {
	const GeometryInstance3D *vi = Object::cast_to<GeometryInstance3D>(p_node);
	if (vi && vi->is_visible_in_tree()) {
		const AABB aabb = vi->get_global_transform().xform(vi->get_aabb());
		if (aabb.has_volume() || aabb.size.length() > 0) {
			if (r_has_bounds) {
				r_bounds.merge_with(aabb);
			} else {
				r_bounds = aabb;
				r_has_bounds = true;
			}
		}
	}
	for (Node *child : p_node->iterate_children()) {
		_collect_bounds(child, r_bounds, r_has_bounds);
	}
}

template <typename T>
static bool _split_view_has_node_of_type(Node *p_node) {
	if (Object::cast_to<T>(p_node)) {
		return true;
	}
	for (Node *child : p_node->iterate_children()) {
		if (_split_view_has_node_of_type<T>(child)) {
			return true;
		}
	}
	return false;
}

void EditorSplitView::_setup_preview_lighting(Node *p_root) {
	if (preview_pane->fallback_lighting) {
		memdelete(preview_pane->fallback_lighting);
		preview_pane->fallback_lighting = nullptr;
	}
	if (!Object::cast_to<Node3D>(p_root)) {
		return;
	}
	// Like the 3D editor's preview sun and environment: each is only added
	// when the scene has none of its own.
	const bool needs_sun = !_split_view_has_node_of_type<DirectionalLight3D>(p_root);
	const bool needs_environment = !_split_view_has_node_of_type<WorldEnvironment>(p_root);
	if (!needs_sun && !needs_environment) {
		return;
	}
	Node3D *lighting = memnew(Node3D);
	if (needs_sun) {
		DirectionalLight3D *sun = memnew(DirectionalLight3D);
		sun->set_rotation(Vector3(Math::deg_to_rad(-60.0), Math::deg_to_rad(150.0), 0));
		sun->set_shadow(true);
		lighting->add_child(sun);
	}
	if (needs_environment) {
		Ref<ProceduralSkyMaterial> sky_material;
		sky_material.instantiate();
		Ref<Sky> sky;
		sky.instantiate();
		sky->set_material(sky_material);
		Ref<Environment> env;
		env.instantiate();
		env->set_background(Environment::BG_SKY);
		env->set_sky(sky);
		env->set_tonemapper(Environment::TONE_MAPPER_FILMIC);
		WorldEnvironment *world_env = memnew(WorldEnvironment);
		world_env->set_environment(env);
		lighting->add_child(world_env);
	}
	preview_pane->viewport->add_child(lighting);
	preview_pane->fallback_lighting = lighting;
}

void EditorSplitView::_update_preview_camera() {
	Node *root = get_preview_scene();
	if (!root) {
		return;
	}
	if (preview_view_custom) {
		// Looked around in: keep that view.
		if (Object::cast_to<Node3D>(root)) {
			_apply_orbit(preview_orbit, preview_pane->camera_3d, preview_pane->viewport);
		} else {
			preview_pane->viewport->set_global_canvas_transform(preview_view_2d);
		}
		preview_pane->overlay->queue_redraw();
		preview_pane->rotation_control->queue_redraw();
		return;
	}
	const int idx = _find_scene_index(preview_scene);
	const Dictionary states = EditorNode::get_editor_data().get_scene_editor_states(idx);
	_frame_view(root, states.get("3D", Dictionary()), preview_pane->camera_3d, preview_pane->viewport, preview_pane->viewport_container->get_size());
	// The scene's own view: not a side view.
	preview_view_type = PANE_VIEW_TYPE_USER;
	preview_auto_orthogonal = false;
	preview_pane->overlay->queue_redraw();
	preview_pane->rotation_control->queue_redraw();
}

void EditorSplitView::_begin_custom_view() {
	if (preview_view_custom) {
		return;
	}
	Node *root = get_preview_scene();
	if (!root) {
		return;
	}
	// Start from what the pane shows now.
	const int idx = _find_scene_index(preview_scene);
	const Dictionary states = EditorNode::get_editor_data().get_scene_editor_states(idx);
	preview_orbit = _orbit_from_state(root, states.get("3D", Dictionary()));
	preview_view_2d = preview_pane->viewport->get_global_canvas_transform();
	preview_view_custom = true;
}

void EditorSplitView::_update_live_camera() {
	Node *root = EditorNode::get_singleton()->get_edited_scene();
	if (!root || !showing_live || showing_code) {
		return;
	}
	// The edited scene is drawn in the editor's own 3D world and the scene
	// root's 2D canvas; look into those rather than moving the scene.
	preview_pane->live_viewport->set_world_3d(get_tree()->get_root()->get_world_3d());
	preview_pane->live_viewport->set_world_2d(EditorNode::get_singleton()->get_scene_root()->get_world_2d());
	// Where the 3D editor's camera is now.
	_frame_view(root, Node3DEditor::get_singleton()->get_state(), preview_pane->live_camera, preview_pane->live_viewport, preview_pane->live_container->get_size());
}

EditorSplitView::OrbitView EditorSplitView::_orbit_from_state(Node *p_root, const Dictionary &p_state_3d) {
	OrbitView orbit;
	if (!p_state_3d.has("viewports")) {
		// Never opened in 3D: frame everything visible in the scene.
		AABB bounds;
		bool has_bounds = false;
		_collect_bounds(p_root, bounds, has_bounds);
		if (has_bounds) {
			orbit.target = bounds.get_center();
			orbit.distance = MAX(bounds.get_longest_axis_size(), (real_t)1.0) * 1.1;
			orbit.x_rot = 0.45;
			orbit.y_rot = -0.6;
		}
		return orbit;
	}
	// Where its 3D editor camera was.
	const Array viewports = p_state_3d["viewports"];
	if (!viewports.is_empty()) {
		const Dictionary vp = viewports[0];
		orbit.target = vp.get("position", orbit.target);
		orbit.x_rot = vp.get("x_rotation", orbit.x_rot);
		orbit.y_rot = vp.get("y_rotation", orbit.y_rot);
		orbit.distance = vp.get("distance", orbit.distance);
		orbit.orthogonal = vp.get("orthogonal", false);
	}
	orbit.fov = p_state_3d.get("fov", orbit.fov);
	return orbit;
}

void EditorSplitView::_apply_orbit(const OrbitView &p_orbit, Camera3D *p_camera, SubViewport *p_viewport) {
	Transform3D xform;
	xform.translate_local(p_orbit.target);
	xform.basis.rotate(Vector3(1, 0, 0), -p_orbit.x_rot);
	xform.basis.rotate(Vector3(0, 1, 0), -p_orbit.y_rot);
	xform.translate_local(0, 0, p_orbit.orthogonal ? 50.0 : p_orbit.distance);
	p_camera->set_global_transform(xform);
	if (p_orbit.orthogonal) {
		p_camera->set_orthogonal(2 * p_orbit.distance, 0.05, 4000);
	} else {
		p_camera->set_perspective(p_orbit.fov, 0.05, 4000);
	}
	p_camera->make_current();
	p_viewport->set_global_canvas_transform(Transform2D());
}

Transform2D EditorSplitView::_fit_project_window(const Size2 &p_size) {
	// Fit the project's window into the pane, like "Center View" in 2D.
	const Size2 project_size = Size2(GLOBAL_GET("display/window/size/viewport_width"), GLOBAL_GET("display/window/size/viewport_height"));
	Transform2D xform;
	if (project_size.x <= 0 || project_size.y <= 0 || p_size.x <= 0 || p_size.y <= 0) {
		return xform;
	}
	const real_t scale = MIN(p_size.x / project_size.x, p_size.y / project_size.y) * 0.9;
	xform.scale_basis(Size2(scale, scale));
	xform.columns[2] = (p_size - project_size * scale) / 2;
	return xform;
}

void EditorSplitView::_frame_view(Node *p_root, const Dictionary &p_state_3d, Camera3D *p_camera, SubViewport *p_viewport, const Size2 &p_size) {
	if (Object::cast_to<Node3D>(p_root)) {
		_apply_orbit(_orbit_from_state(p_root, p_state_3d), p_camera, p_viewport);
	} else if (p_size.x > 0 && p_size.y > 0) {
		p_viewport->set_global_canvas_transform(_fit_project_window(p_size));
	}
}

void EditorSplitView::_live_gui_input(const Ref<InputEvent> &p_event) {
	const Ref<InputEventMouseButton> mb = p_event;
	if (mb.is_valid() && mb->is_pressed() && mb->get_button_index() == MouseButton::LEFT) {
		callable_mp(this, &EditorSplitView::_open_live_scene_editor).call_deferred();
		preview_pane->live_container->accept_event();
	}
}

void EditorSplitView::_open_live_scene_editor() {
	Node *root = EditorNode::get_singleton()->get_edited_scene();
	if (root) {
		EditorInterface::get_singleton()->set_main_screen_editor(Object::cast_to<Node3D>(root) ? "3D" : "2D");
	}
}

void EditorSplitView::_track_current_script() {
	ScriptEditorBase *current = ScriptEditor::get_singleton()->get_current_editor();
	TextEditorBase *current_text = Object::cast_to<TextEditorBase>(current);
	const ObjectID current_id = current_text ? current_text->get_instance_id() : ObjectID();
	if (current_id == current_code_editor_id) {
		return;
	}
	// The script being left goes into the pane, like scenes do.
	if (ObjectDB::get_instance<TextEditorBase>(current_code_editor_id)) {
		code_editor_id = current_code_editor_id;
	}
	current_code_editor_id = current_id;
}

void EditorSplitView::_script_changed(const Ref<Script> &p_script) {
	_track_current_script();
	_queue_update();
}

void EditorSplitView::_set_code_source(CodeEdit *p_source) {
	CodeEdit *old_source = ObjectDB::get_instance<CodeEdit>(code_source_id);
	if (old_source == p_source && p_source) {
		return;
	}
	if (old_source && old_source->is_connected(SceneStringName(text_changed), callable_mp(this, &EditorSplitView::_code_source_changed))) {
		old_source->disconnect(SceneStringName(text_changed), callable_mp(this, &EditorSplitView::_code_source_changed));
	}
	code_source_id = p_source ? p_source->get_instance_id() : ObjectID();
	CodeEdit *view = preview_pane->code_view;
	if (!p_source) {
		view->set_syntax_highlighter(Ref<SyntaxHighlighter>());
		view->set_text(String());
		return;
	}
	p_source->connect(SceneStringName(text_changed), callable_mp(this, &EditorSplitView::_code_source_changed));
	code_static_script.unref();

	_style_code_view(p_source);
	Ref<EditorSyntaxHighlighter> highlighter = p_source->get_syntax_highlighter();
	if (highlighter.is_valid()) {
		Ref<EditorSyntaxHighlighter> copy = highlighter->_create();
		if (copy.is_valid()) {
			copy->_set_edited_resource(highlighter->_get_edited_resource());
		}
		view->set_syntax_highlighter(copy);
	} else {
		view->set_syntax_highlighter(Ref<SyntaxHighlighter>());
	}
	view->set_text(p_source->get_text());
	view->set_v_scroll(p_source->get_v_scroll());
}

void EditorSplitView::_style_code_view(CodeEdit *p_style_from) {
	// Look like the script editor: its font and colors.
	CodeEdit *view = preview_pane->code_view;
	if (!p_style_from) {
		return;
	}
	view->add_theme_font_override(SceneStringName(font), p_style_from->get_theme_font(SceneStringName(font)));
	view->add_theme_font_size_override(SceneStringName(font_size), p_style_from->get_theme_font_size(SceneStringName(font_size)));
	static const char *colors[] = { "background_color", "font_color", "line_number_color", "current_line_color", "selection_color", "word_highlighted_color", "caret_color", "brace_mismatch_color" };
	for (const char *color : colors) {
		view->add_theme_color_override(color, p_style_from->get_theme_color(color));
	}
	// Read-only text would otherwise be drawn dimmed.
	view->add_theme_color_override("font_readonly_color", p_style_from->get_theme_color(SceneStringName(font_color)));
}

void EditorSplitView::_set_code_static(const Ref<Script> &p_script) {
	// A script that isn't open in the script editor: show its source.
	CodeEdit *view = preview_pane->code_view;
	if (code_static_script == p_script && view->get_text() == p_script->get_source_code()) {
		return;
	}
	_set_code_source(nullptr);
	const bool same = code_static_script == p_script;
	code_static_script = p_script;
	// Borrow the look of any open script editor.
	DocumentEditorContainer *container = ScriptEditor::get_singleton()->get_script_container();
	for (const Ref<Script> &scr : ScriptEditor::get_singleton()->get_open_scripts()) {
		if (TextEditorBase *editor = Object::cast_to<TextEditorBase>(container->get_resource_editor(scr))) {
			_style_code_view(editor->get_code_editor()->get_text_editor());
			break;
		}
	}
	view->set_syntax_highlighter(container->create_syntax_highlighter(p_script));
	const double scroll = view->get_v_scroll();
	view->set_text(p_script->get_source_code());
	view->set_v_scroll(same ? scroll : 0);
}

Ref<Script> EditorSplitView::_selected_node_script() const {
	EditorSelection *selection = EditorNode::get_singleton()->get_editor_selection();
	for (Node *node : selection->get_top_selected_node_list()) {
		const Ref<Script> scr = node->get_script();
		if (scr.is_valid()) {
			return scr;
		}
	}
	Node *root = EditorNode::get_singleton()->get_edited_scene();
	return root ? Ref<Script>(root->get_script()) : Ref<Script>();
}

void EditorSplitView::_selection_changed() {
	// Follow the selection: show the script of the node just selected.
	EditorSelection *selection = EditorNode::get_singleton()->get_editor_selection();
	for (Node *node : selection->get_top_selected_node_list()) {
		const Ref<Script> scr = node->get_script();
		if (scr.is_valid()) {
			scene_code_script = scr;
			if (showing_scene_code) {
				_queue_update();
			}
			return;
		}
	}
}

void EditorSplitView::_update_scene_code_view() {
	if (scene_code_script.is_null()) {
		scene_code_script = _selected_node_script();
	}
	const Ref<Script> scr = scene_code_script;
	TextEditorBase *open = scr.is_valid() ? Object::cast_to<TextEditorBase>(ScriptEditor::get_singleton()->get_script_container()->get_resource_editor(scr)) : nullptr;
	preview_pane->code_view->set_visible(scr.is_valid());
	preview_pane->code_placeholder->set_visible(scr.is_null());
	preview_pane->code_placeholder->set_text(TTR("Select a node with a script, or pick a script from the menu above."));
	if (open) {
		// Open in the script editor: show its current text, unsaved edits included.
		_set_code_source(open->get_code_editor()->get_text_editor());
	} else if (scr.is_valid()) {
		_set_code_static(scr);
	} else {
		_set_code_source(nullptr);
	}

	if (scr.is_null()) {
		preview_pane->set_title(TTR("No script"));
		preview_pane->type_label->set_text(String());
	} else {
		String title = scr->get_path().is_empty() ? TTR("[unsaved]") : scr->get_path().get_file();
		if (open && open->is_unsaved()) {
			title += "(*)";
		}
		preview_pane->set_title(title);
		preview_pane->type_label->set_text(scr->get_class());
	}
	if (floating) {
		float_window->set_title(vformat(TTR("%s - Split View"), preview_pane->file_label->get_text()));
	}
}

void EditorSplitView::_set_setting(const String &p_name, const Variant &p_value) {
	EditorSettings::get_singleton()->set(p_name, p_value);
	EditorSettings::get_singleton()->notify_changes();
}

void EditorSplitView::_code_source_changed() {
	CodeEdit *source = ObjectDB::get_instance<CodeEdit>(code_source_id);
	if (!source) {
		return;
	}
	CodeEdit *view = preview_pane->code_view;
	const double scroll = view->get_v_scroll();
	view->set_text(source->get_text());
	view->set_v_scroll(scroll);
}

void EditorSplitView::_update_code_view() {
	_track_current_script();
	TextEditorBase *shown = ObjectDB::get_instance<TextEditorBase>(code_editor_id);
	if (!shown || code_editor_id == current_code_editor_id) {
		// Nothing left from before: take another open script.
		shown = nullptr;
		code_editor_id = ObjectID();
		DocumentEditorContainer *container = ScriptEditor::get_singleton()->get_script_container();
		for (const Ref<Script> &scr : ScriptEditor::get_singleton()->get_open_scripts()) {
			TextEditorBase *editor = Object::cast_to<TextEditorBase>(container->get_resource_editor(scr));
			if (editor && editor->get_instance_id() != current_code_editor_id) {
				shown = editor;
				code_editor_id = editor->get_instance_id();
				break;
			}
		}
	}
	preview_pane->code_view->set_visible(shown);
	preview_pane->code_placeholder->set_visible(!shown);
	preview_pane->code_placeholder->set_text(TTR("Open another script to compare it here.\nThe script you switch away from appears here."));
	_set_code_source(shown ? shown->get_code_editor()->get_text_editor() : nullptr);

	const Ref<Resource> res = shown ? shown->get_edited_resource() : Ref<Resource>();
	if (res.is_null()) {
		preview_pane->set_title(TTR("No other script"));
		preview_pane->type_label->set_text(String());
	} else {
		String title = res->get_path().is_empty() ? TTR("[unsaved]") : res->get_path().get_file();
		if (shown->is_unsaved()) {
			title += "(*)";
		}
		preview_pane->set_title(title);
		preview_pane->type_label->set_text(res->get_class());
	}
	if (floating) {
		float_window->set_title(vformat(TTR("%s - Split View"), preview_pane->file_label->get_text()));
	}
}

void EditorSplitView::_code_gui_input(const Ref<InputEvent> &p_event) {
	const Ref<InputEventMouseButton> mb = p_event;
	if (mb.is_valid() && mb->is_pressed() && mb->get_button_index() == MouseButton::LEFT) {
		const Point2i pos = preview_pane->code_view->get_line_column_at_pos(mb->get_position());
		callable_mp(this, &EditorSplitView::_edit_code_at).call_deferred(pos.y, pos.x);
		preview_pane->code_view->accept_event();
	}
}

void EditorSplitView::_edit_code_at(int p_line, int p_column) {
	if (showing_scene_code) {
		if (scene_code_script.is_valid()) {
			ScriptEditor::get_singleton()->edit(scene_code_script, p_line, p_column);
			EditorInterface::get_singleton()->set_main_screen_editor("Script");
		}
		return;
	}
	TextEditorBase *shown = ObjectDB::get_instance<TextEditorBase>(code_editor_id);
	if (!shown) {
		return;
	}
	ScriptEditor::get_singleton()->edit(shown->get_edited_resource(), p_line, p_column);
	_track_current_script();
	_queue_update();
}

void EditorSplitView::_fill_pick_menu() {
	PopupMenu *menu = preview_pane->pick_button->get_popup();
	menu->clear();
	if (showing_scene_code) {
		// Scripts used in the edited scene, then other open scripts.
		Vector<Ref<Script>> scripts;
		Node *root = EditorNode::get_singleton()->get_edited_scene();
		List<Node *> stack;
		if (root) {
			stack.push_back(root);
		}
		while (!stack.is_empty()) {
			Node *node = stack.front()->get();
			stack.pop_front();
			const Ref<Script> scr = node->get_script();
			if (scr.is_valid() && !scripts.has(scr)) {
				scripts.push_back(scr);
			}
			for (int i = 0; i < node->get_child_count(); i++) {
				Node *child = node->get_child(i);
				if (child->get_owner() == root || child->get_owner() == nullptr) {
					stack.push_back(child);
				}
			}
		}
		const int scene_count = scripts.size();
		for (const Ref<Script> &scr : ScriptEditor::get_singleton()->get_open_scripts()) {
			if (!scripts.has(scr)) {
				scripts.push_back(scr);
			}
		}
		for (int i = 0; i < scripts.size(); i++) {
			if (i == scene_count && i > 0) {
				menu->add_separator(TTR("Open Scripts"));
			}
			const Ref<Script> &scr = scripts[i];
			menu->add_radio_check_item(scr->get_path().is_empty() ? TTR("[unsaved]") : scr->get_path().get_file());
			const int idx = menu->get_item_count() - 1;
			menu->set_item_metadata(idx, scr);
			menu->set_item_tooltip(idx, scr->get_path());
			menu->set_item_checked(idx, scr == scene_code_script);
		}
		if (scripts.is_empty()) {
			menu->add_item(TTR("No scripts in this scene"));
			menu->set_item_disabled(-1, true);
		}
		return;
	}
	if (showing_code) {
		// Open scripts other than the one being edited.
		DocumentEditorContainer *container = ScriptEditor::get_singleton()->get_script_container();
		for (const Ref<Script> &scr : ScriptEditor::get_singleton()->get_open_scripts()) {
			TextEditorBase *editor = Object::cast_to<TextEditorBase>(container->get_resource_editor(scr));
			if (!editor || editor->get_instance_id() == current_code_editor_id) {
				continue;
			}
			const String name = scr->get_path().is_empty() ? TTR("[unsaved]") : scr->get_path().get_file();
			menu->add_radio_check_item(name);
			const int idx = menu->get_item_count() - 1;
			menu->set_item_metadata(idx, editor->get_instance_id());
			menu->set_item_tooltip(idx, scr->get_path());
			menu->set_item_checked(idx, editor->get_instance_id() == code_editor_id);
		}
		if (menu->get_item_count() == 0) {
			menu->add_item(TTR("No other open script"));
			menu->set_item_disabled(-1, true);
		}
		return;
	}
	if (showing_live) {
		menu->add_item(TTR("This is the scene being edited"));
		menu->set_item_disabled(-1, true);
		return;
	}
	// Open scenes other than the one being edited.
	EditorData &ed = EditorNode::get_editor_data();
	for (int i = 0; i < ed.get_edited_scene_count(); i++) {
		if (i == ed.get_edited_scene() || !ed.get_edited_scene_root(i)) {
			continue;
		}
		const String path = ed.get_scene_path(i);
		menu->add_radio_check_item(path.is_empty() ? TTR("[unsaved]") : path.get_file());
		const int idx = menu->get_item_count() - 1;
		menu->set_item_metadata(idx, i);
		menu->set_item_tooltip(idx, path);
		menu->set_item_checked(idx, ed.get_edited_scene_root(i)->get_instance_id() == preview_scene);
	}
	if (menu->get_item_count() == 0) {
		menu->add_item(TTR("No other open scene"));
		menu->set_item_disabled(-1, true);
	}
}

void EditorSplitView::_pick_menu_id_pressed(int p_id) {
	PopupMenu *menu = preview_pane->pick_button->get_popup();
	const int idx = menu->get_item_index(p_id);
	if (idx < 0) {
		return;
	}
	const Variant meta = menu->get_item_metadata(idx);
	if (showing_scene_code) {
		scene_code_script = meta;
	} else if (showing_code) {
		code_editor_id = ObjectID(uint64_t(meta));
	} else {
		show_scene_in_preview(int(meta));
	}
	_queue_update();
}

void EditorSplitView::_code_toggled(bool p_pressed) {
	if (_is_script_screen()) {
		compare_scripts = p_pressed;
		_set_setting("interface/editor/split_view/compare_scripts", p_pressed);
	} else {
		code_beside_scene = p_pressed;
		_set_setting("interface/editor/split_view/code_beside_scene", p_pressed);
	}
	_queue_update();
}

void EditorSplitView::show_script_in_preview(ScriptEditorBase *p_editor) {
	TextEditorBase *shown = Object::cast_to<TextEditorBase>(p_editor);
	ERR_FAIL_NULL(shown);
	_track_current_script();
	// Picking it in the script list made it the edited script; go back to the
	// one edited before it, which leaves this one in the pane.
	TextEditorBase *back = ObjectDB::get_instance<TextEditorBase>(current_code_editor_id == shown->get_instance_id() ? code_editor_id : current_code_editor_id);
	if (!back || back == shown) {
		back = nullptr;
		DocumentEditorContainer *container = ScriptEditor::get_singleton()->get_script_container();
		for (const Ref<Script> &scr : ScriptEditor::get_singleton()->get_open_scripts()) {
			TextEditorBase *editor = Object::cast_to<TextEditorBase>(container->get_resource_editor(scr));
			if (editor && editor != shown) {
				back = editor;
				break;
			}
		}
	}
	if (!back) {
		return;
	}
	if (!enabled) {
		enabled = true;
		_set_setting("interface/editor/split_view/enabled", true);
	}
	if (!compare_scripts) {
		compare_scripts = true;
		_set_setting("interface/editor/split_view/compare_scripts", true);
	}
	if (ScriptEditor::get_singleton()->get_current_editor() != back) {
		ScriptEditor::get_singleton()->edit(back->get_edited_resource());
	}
	_track_current_script();
	code_editor_id = shown->get_instance_id();
	_queue_update();
}

bool EditorSplitView::_can_drop(const Point2 &p_point, const Variant &p_data) const {
	const Dictionary d = p_data;
	const String type = d.get("type", "");
	const bool scene_screen = _is_scene_screen();
	const bool script_screen = _is_script_screen();
	if (type == "tab") {
		// A scene tab.
		const Node *from = get_node_or_null(NodePath(d.get("from_path", NodePath())));
		return scene_screen && from && EditorSceneTabs::get_singleton()->is_ancestor_of(from);
	}
	if (type == "script_list_element") {
		return script_screen && Object::cast_to<TextEditorBase>(d.get("script_list_element", Variant()).operator Object *());
	}
	if (type == "resource") {
		const Ref<Script> scr = d.get("resource", Variant());
		return scr.is_valid() && (scene_screen || script_screen);
	}
	if (type == "files") {
		const Vector<String> files = d.get("files", Vector<String>());
		if (files.is_empty()) {
			return false;
		}
		const String file_type = ResourceLoader::get_resource_type(files[0]);
		if (ClassDB::is_parent_class(file_type, "PackedScene")) {
			return scene_screen;
		}
		return ClassDB::is_parent_class(file_type, "Script") && (scene_screen || script_screen);
	}
	return false;
}

void EditorSplitView::_drop(const Point2 &p_point, const Variant &p_data) {
	const Dictionary d = p_data;
	const String type = d.get("type", "");
	EditorData &ed = EditorNode::get_editor_data();
	Ref<Script> scr;
	if (type == "tab") {
		int idx = d.get("tab_index", -1);
		if (idx < 0 || idx >= ed.get_edited_scene_count()) {
			return;
		}
		if (idx == ed.get_edited_scene()) {
			// Pressing the tab made it the edited scene; go back to the one
			// edited before it, which leaves the dragged scene in the pane.
			const ObjectID dragged = ed.get_edited_scene_root(idx) ? ed.get_edited_scene_root(idx)->get_instance_id() : ObjectID();
			const int back = _find_scene_index(preview_scene);
			if (back < 0 || back == idx) {
				return;
			}
			EditorSceneTabs::get_singleton()->set_current_tab(back);
			idx = _find_scene_index(dragged);
		}
		if (idx >= 0 && idx != ed.get_edited_scene()) {
			code_beside_scene = false;
			_set_setting("interface/editor/split_view/code_beside_scene", false);
			show_scene_in_preview(idx);
		}
		return;
	}
	if (type == "script_list_element") {
		show_script_in_preview(Object::cast_to<ScriptEditorBase>(d.get("script_list_element", Variant()).operator Object *()));
		return;
	}
	if (type == "resource") {
		scr = d.get("resource", Variant());
	} else if (type == "files") {
		const Vector<String> files = d.get("files", Vector<String>());
		ERR_FAIL_COND(files.is_empty());
		const String path = files[0];
		if (ClassDB::is_parent_class(ResourceLoader::get_resource_type(path), "PackedScene")) {
			int idx = -1;
			for (int i = 0; i < ed.get_edited_scene_count(); i++) {
				if (ed.get_scene_path(i) == path) {
					idx = i;
				}
			}
			if (idx < 0) {
				// Open it, then go back to the scene being edited.
				const int current = ed.get_edited_scene();
				if (EditorNode::get_singleton()->load_scene(path) != OK) {
					return;
				}
				for (int i = 0; i < ed.get_edited_scene_count(); i++) {
					if (ed.get_scene_path(i) == path) {
						idx = i;
					}
				}
				if (current >= 0 && current < ed.get_edited_scene_count()) {
					EditorSceneTabs::get_singleton()->set_current_tab(current);
				}
			}
			if (idx >= 0 && idx != ed.get_edited_scene()) {
				code_beside_scene = false;
				_set_setting("interface/editor/split_view/code_beside_scene", false);
				show_scene_in_preview(idx);
			}
			return;
		}
		scr = ResourceLoader::load(path);
	}
	if (scr.is_null()) {
		return;
	}
	if (_is_script_screen()) {
		// Open it in the script editor, then keep it beside the current one.
		ScriptEditor::get_singleton()->edit(scr, false);
		show_script_in_preview(ScriptEditor::get_singleton()->get_script_container()->get_resource_editor(scr));
		return;
	}
	scene_code_script = scr;
	if (!code_beside_scene) {
		code_beside_scene = true;
		_set_setting("interface/editor/split_view/code_beside_scene", true);
	}
	_queue_update();
}

void EditorSplitView::_draw_preview_overlay() {
	Control *overlay = preview_pane->overlay;
	Node *root = get_preview_scene();
	preview_pane->tools->draw_overlay();
	if (zoom_indicator_delay > 0.0 && Object::cast_to<Node3D>(root) && !preview_cinematic) {
		// The 3D editor's zoom bar (how far the view is from what it orbits)
		// or, while flying, its speed bar; on a log scale between the
		// camera's near and far limits.
		const real_t min_distance = MAX(preview_pane->camera_3d->get_near() * 4, (real_t)0.01);
		const real_t max_distance = MIN(preview_pane->camera_3d->get_far() / 4, (real_t)10000.0);
		const real_t scale_length = max_distance - min_distance;
		if (!Math::is_zero_approx(scale_length)) {
			const real_t distance = preview_freelook ? freelook_speed : preview_orbit.distance;
			const real_t log_fill = Math::log1p(MAX(distance - min_distance, (real_t)0.0)) / Math::log1p(scale_length);
			const real_t fill = CLAMP(preview_freelook ? log_fill : 1.0 - log_fill, 0.0, 1.0);
			const Color color = preview_freelook ? Color(1.0, 0.95, 0.7) : Color(0.7, 0.95, 1.0);
			const real_t height = overlay->get_size().y / 2.0;
			const Rect2 bar(10 * EDSCALE, (overlay->get_size().y - height) / 2.0, 6 * EDSCALE, height);
			const real_t filled = bar.size.y * fill;
			// Readable over any background: both a light and a dark edge.
			overlay->draw_rect(bar, color * Color(1, 1, 1, 0.2));
			overlay->draw_rect(Rect2(bar.position.x, bar.position.y + bar.size.y - filled, bar.size.x, filled), color * Color(1, 1, 1, 0.6));
			overlay->draw_rect(bar.grow(1), Color(0, 0, 0, 0.7), false, Math::round(EDSCALE));
			const Ref<Texture2D> icon = get_editor_theme_icon(preview_freelook ? SNAME("ViewportSpeed") : SNAME("ViewportZoom"));
			const Vector2 icon_size = icon->get_size();
			const Vector2 icon_pos = Vector2(bar.position.x - (icon_size.x - bar.size.x) / 2, bar.position.y + bar.size.y + 2 * EDSCALE);
			overlay->draw_texture(icon, icon_pos, color);
			const Ref<Font> font = get_theme_font(SNAME("bold"), EditorStringName(EditorFonts));
			const int font_size = get_theme_font_size(SceneStringName(font_size), SNAME("Label"));
			const String text = vformat(preview_freelook ? "%s m/s" : "%s m", String::num(distance).pad_decimals(distance < 1.0 ? 2 : 1));
			const Vector2 text_pos = Vector2(icon_pos.x, icon_pos.y + icon_size.y + 16 * EDSCALE);
			overlay->draw_string_outline(font, text_pos, text, HORIZONTAL_ALIGNMENT_LEFT, -1, font_size, Math::round(4 * EDSCALE), Color(0, 0, 0));
			overlay->draw_string(font, text_pos, text, HORIZONTAL_ALIGNMENT_LEFT, -1, font_size, color);
		}
	}
	if (root && !Object::cast_to<Node3D>(root)) {
		// The project's window, as the 2D editor draws it.
		const Size2 project_size = Size2(GLOBAL_GET("display/window/size/viewport_width"), GLOBAL_GET("display/window/size/viewport_height"));
		const Transform2D xform = preview_pane->viewport->get_global_canvas_transform();
		const Rect2 frame = xform.xform(Rect2(Point2(), project_size));
		overlay->draw_rect(frame, get_theme_color(SNAME("accent_color"), EditorStringName(Editor)) * Color(1, 1, 1, 0.6), false, Math::round(EDSCALE));
	}
	if (!preview_pane->tools->is_dragging() && overlay->get_global_rect().has_point(overlay->get_global_mouse_position())) {
		const Ref<Font> font = get_theme_font(SNAME("main"), EditorStringName(EditorFonts));
		const int font_size = get_theme_font_size(SNAME("main_size"), EditorStringName(EditorFonts));
		const int small_size = MAX(1, int(font_size * 0.85));
		const String hint = preview_pane->tools->get_hint();
		const String keys = root && Object::cast_to<Node3D>(root) ? TTR("Wheel: zoom  Middle-drag: orbit  Shift: pan  Right-drag: look, WASD to fly  Double middle-click: reset") : TTR("Wheel: zoom  Middle-drag: pan  Double middle-click: reset");
		const Size2 hint_size = font->get_string_size(hint, HORIZONTAL_ALIGNMENT_LEFT, -1, font_size);
		const Size2 keys_size = font->get_string_size(keys, HORIZONTAL_ALIGNMENT_LEFT, -1, small_size);
		const real_t width = MAX(hint_size.x, keys_size.x);
		const real_t line = font->get_height(small_size);
		const Point2 hint_pos = Point2((overlay->get_size().x - hint_size.x) / 2, overlay->get_size().y - 16 * EDSCALE - line);
		const Point2 keys_pos = Point2((overlay->get_size().x - keys_size.x) / 2, overlay->get_size().y - 16 * EDSCALE);
		const Rect2 bg = Rect2(Point2((overlay->get_size().x - width) / 2 - 8 * EDSCALE, hint_pos.y - font->get_ascent(font_size) - 4 * EDSCALE), Size2(width + 16 * EDSCALE, hint_size.y + line + 8 * EDSCALE));
		overlay->draw_rect(bg, get_theme_color(SNAME("dark_color_1"), EditorStringName(Editor)) * Color(1, 1, 1, 0.85));
		overlay->draw_string(font, hint_pos, hint, HORIZONTAL_ALIGNMENT_LEFT, -1, font_size, get_theme_color(SceneStringName(font_color), EditorStringName(Editor)));
		overlay->draw_string(font, keys_pos, keys, HORIZONTAL_ALIGNMENT_LEFT, -1, small_size, get_theme_color(SNAME("font_disabled_color"), EditorStringName(Editor)));
	}
}

// The second pane's Scene toolbar: the Scene dock's, for the pane's scene.

Node *EditorSplitView::_pane_target_node() const {
	// What's selected in the pane, or its scene's root.
	Node *selected = preview_pane->tools->get_selected();
	return selected ? selected : get_preview_scene();
}

void EditorSplitView::_pane_add_child(Node *p_parent, Node *p_child, const String &p_action) {
	Node *root = get_preview_scene();
	const int history_id = preview_pane->tools->get_history_id();
	if (!root || history_id == EditorUndoRedoManager::INVALID_HISTORY) {
		memdelete(p_child);
		return;
	}
	EditorUndoRedoManager *undo_redo = EditorUndoRedoManager::get_singleton();
	undo_redo->create_action_for_history(p_action, history_id);
	undo_redo->force_fixed_history();
	undo_redo->add_do_method(p_parent, "add_child", p_child, true);
	undo_redo->add_do_method(p_child, "set_owner", root);
	undo_redo->add_do_reference(p_child);
	undo_redo->add_undo_method(p_parent, "remove_child", p_child);
	undo_redo->commit_action();

	// Select it, as the Scene dock does.
	_set_preview_active(true);
	preview_pane->tree->update_tree();
	preview_pane->tools->set_selected(p_child, false);
	_pane_selection_changed();
}

void EditorSplitView::_pane_add_node() {
	Node *parent = _pane_target_node();
	if (!parent) {
		return;
	}
	_set_preview_active(true);
	pane_new_node_parent = parent->get_instance_id();
	if (!pane_create_dialog) {
		pane_create_dialog = memnew(CreateDialog);
		pane_create_dialog->set_base_type("Node");
		pane_create_dialog->connect("create", callable_mp(this, &EditorSplitView::_pane_create_node));
		add_child(pane_create_dialog);
	}
	pane_create_dialog->popup_create(true);
}

void EditorSplitView::_pane_create_node() {
	Node *parent = ObjectDB::get_instance<Node>(pane_new_node_parent);
	Node *root = get_preview_scene();
	if (!parent || !root || (parent != root && !root->is_ancestor_of(parent))) {
		return;
	}
	Node *child = pane_create_dialog->instantiate_selected<Node>();
	ERR_FAIL_NULL(child);

	// Named as the Scene dock names new nodes.
	String name = child->get_name();
	if (name.is_empty()) {
		name = child->get_class();
	}
	if (GLOBAL_GET("editor/naming/node_name_casing").operator int() != Node::NAME_CASING_PASCAL_CASE) {
		name = Node::adjust_name_casing(name);
	}
	child->set_name(parent->prevalidate_child_name(child, name));
	if (Control *control = Object::cast_to<Control>(child)) {
		// As the Scene dock does: not so small it can't be seen.
		Size2 size = control->get_minimum_size();
		size.width = size.width < 4 ? 40 : size.width;
		size.height = size.height < 4 ? 40 : size.height;
		control->set_size(size);
	}
	_pane_add_child(parent, child, TTR("Create Node"));
}

void EditorSplitView::_pane_instantiate_scene() {
	Node *parent = _pane_target_node();
	if (!parent) {
		return;
	}
	_set_preview_active(true);
	pane_new_node_parent = parent->get_instance_id();
	EditorNode::get_singleton()->get_quick_open_dialog()->popup_dialog({ "PackedScene" }, callable_mp(this, &EditorSplitView::_pane_scene_picked));
}

void EditorSplitView::_pane_scene_picked(const String &p_path) {
	Node *parent = ObjectDB::get_instance<Node>(pane_new_node_parent);
	Node *root = get_preview_scene();
	if (!parent || !root || (parent != root && !root->is_ancestor_of(parent))) {
		return;
	}
	Ref<PackedScene> packed = ResourceLoader::load(p_path);
	if (packed.is_null()) {
		EditorNode::get_singleton()->show_warning(vformat(TTR("Error loading scene from %s"), p_path));
		return;
	}
	Node *instance = packed->instantiate(PackedScene::GEN_EDIT_STATE_INSTANCE);
	if (!instance) {
		EditorNode::get_singleton()->show_warning(vformat(TTR("Error instantiating scene from %s"), p_path));
		return;
	}
	const String root_path = root->get_scene_file_path();
	if (!root_path.is_empty() && SceneTreeDock::get_singleton()->would_instantiate_cyclically(root_path, instance)) {
		memdelete(instance);
		EditorNode::get_singleton()->show_warning(vformat(TTR("Cannot instantiate the scene '%s' because the current scene exists within one of its nodes."), p_path));
		return;
	}
	instance->set_scene_file_path(ProjectSettings::get_singleton()->localize_path(p_path));
	_pane_add_child(parent, instance, TTR("Instantiate Scene"));
}

void EditorSplitView::_pane_attach_script() {
	Node *node = _pane_target_node();
	if (!node) {
		return;
	}
	if (ScriptServer::get_language_count() == 0) {
		EditorNode::get_singleton()->show_warning(TTR("Cannot attach a script: there are no languages registered.\nThis is probably because this editor was built with all language modules disabled."));
		return;
	}
	_set_preview_active(true);
	pane_script_target = node->get_instance_id();
	// Where the Scene dock suggests it: beside the scene.
	String path = node->get_scene_file_path();
	if (path.is_empty()) {
		const String root_path = get_preview_scene()->get_scene_file_path();
		path = (root_path.is_empty() ? String("res://") : root_path.get_base_dir()).path_join(node->get_name());
	}
	if (!pane_script_dialog) {
		pane_script_dialog = memnew(ScriptCreateDialog);
		pane_script_dialog->connect("script_created", callable_mp(this, &EditorSplitView::_pane_script_created));
		add_child(pane_script_dialog);
	}
	pane_script_dialog->set_inheritance_base_type("Node");
	pane_script_dialog->config(node->get_class(), path);
	pane_script_dialog->popup_centered();
}

void EditorSplitView::_pane_script_created(const Ref<Script> &p_script) {
	Node *node = ObjectDB::get_instance<Node>(pane_script_target);
	Node *root = get_preview_scene();
	const int history_id = preview_pane->tools->get_history_id();
	if (!node || !root || (node != root && !root->is_ancestor_of(node)) || history_id == EditorUndoRedoManager::INVALID_HISTORY) {
		return;
	}
	if (p_script->is_built_in()) {
		EditorNode::setup_built_in_resource(p_script, root->get_scene_file_path());
	}
	EditorUndoRedoManager *undo_redo = EditorUndoRedoManager::get_singleton();
	undo_redo->create_action_for_history(TTR("Attach Script"), history_id);
	undo_redo->force_fixed_history();
	undo_redo->add_do_method(InspectorDock::get_singleton(), "store_script_properties", node);
	undo_redo->add_undo_method(InspectorDock::get_singleton(), "store_script_properties", node);
	undo_redo->add_do_method(node, "set_script", p_script);
	undo_redo->add_undo_method(node, "set_script", node->get_script());
	undo_redo->add_do_method(InspectorDock::get_singleton(), "apply_script_properties", node);
	undo_redo->add_undo_method(InspectorDock::get_singleton(), "apply_script_properties", node);
	undo_redo->commit_action();
	// Open it, as the Scene dock does.
	EditorNode::get_singleton()->push_item_no_inspector(p_script.ptr());
	_update_pane_toolbar();
}

void EditorSplitView::_pane_detach_script() {
	Node *node = _pane_target_node();
	const int history_id = preview_pane->tools->get_history_id();
	if (!node || history_id == EditorUndoRedoManager::INVALID_HISTORY) {
		return;
	}
	_set_preview_active(true);
	const Ref<Script> existing = node->get_script();
	const Ref<Script> empty = EditorNode::get_singleton()->get_object_custom_type_base(node);
	if (existing == empty) {
		return;
	}
	EditorUndoRedoManager *undo_redo = EditorUndoRedoManager::get_singleton();
	undo_redo->create_action_for_history(TTR("Detach Script"), history_id);
	undo_redo->force_fixed_history();
	undo_redo->add_do_method(node, "set_script", empty);
	undo_redo->add_undo_method(node, "set_script", existing);
	// The script's properties come back with it.
	List<PropertyInfo> properties;
	node->get_property_list(&properties);
	for (const PropertyInfo &property : properties) {
		if (property.usage & (PROPERTY_USAGE_STORAGE | PROPERTY_USAGE_EDITOR)) {
			undo_redo->add_undo_property(node, property.name, node->get(property.name));
		}
	}
	undo_redo->commit_action();
	_update_pane_toolbar();
}

void EditorSplitView::_pane_open_scene(const String &p_path) {
	EditorNode::get_singleton()->open_scene(p_path);
}

void EditorSplitView::_pane_open_script(const Ref<Script> &p_script) {
	EditorNode::get_singleton()->push_item_no_inspector(p_script.ptr());
}

void EditorSplitView::_pane_filter_changed(const String &p_filter) {
	preview_pane->tree->set_filter(p_filter);
}

void EditorSplitView::_pane_tree_menu_about_to_popup() {
	_set_preview_active(true);
	PopupMenu *menu = preview_pane->tree_menu->get_popup();
	if (menu->get_item_count() == 0) {
		menu->add_check_item(TTRC("Auto Expand to Selected"), PANE_TREE_AUTO_EXPAND);
		menu->add_check_item(TTRC("Hide Filtered Out Parents"), PANE_TREE_HIDE_FILTERED_OUT_PARENTS);
		menu->add_separator();
		menu->add_check_item(TTRC("Show Accessibility Warnings"), PANE_TREE_ACCESSIBILITY_WARNINGS);
		menu->add_submenu_node_item(TTRC("All Scene Sub-Resources"), preview_pane->subresource_menu);
		menu->add_separator();
		menu->add_item(TTRC("Filter by Type"), PANE_TREE_FILTER_BY_TYPE);
		menu->set_item_tooltip(-1, TTRC("Selects all Nodes of the given type.\nInserts \"type:\". You can also use the shorthand \"t:\"."));
		menu->add_item(TTRC("Filter by Group"), PANE_TREE_FILTER_BY_GROUP);
		menu->set_item_tooltip(-1, TTRC("Selects all Nodes belonging to the given group.\nIf empty, selects any Node belonging to any group.\nInserts \"group:\". You can also use the shorthand \"g:\"."));
	}
	menu->set_item_checked(menu->get_item_index(PANE_TREE_AUTO_EXPAND), EDITOR_GET("docks/scene_tree/auto_expand_to_selected"));
	menu->set_item_checked(menu->get_item_index(PANE_TREE_HIDE_FILTERED_OUT_PARENTS), EDITOR_GET("docks/scene_tree/hide_filtered_out_parents"));
	menu->set_item_checked(menu->get_item_index(PANE_TREE_ACCESSIBILITY_WARNINGS), EDITOR_GET("docks/scene_tree/accessibility_warnings"));
}

void EditorSplitView::_pane_tree_menu_option(int p_option) {
	// The same settings as the Scene dock's, applied to both trees.
	SceneTreeEditor *dock_tree = SceneTreeDock::get_singleton()->get_tree_editor();
	switch (p_option) {
		case PANE_TREE_AUTO_EXPAND: {
			const bool expand = !EDITOR_GET("docks/scene_tree/auto_expand_to_selected");
			dock_tree->set_auto_expand_selected(expand, true);
			preview_pane->tree->set_auto_expand_selected(expand, false);
		} break;
		case PANE_TREE_HIDE_FILTERED_OUT_PARENTS: {
			const bool hide = !EDITOR_GET("docks/scene_tree/hide_filtered_out_parents");
			dock_tree->set_hide_filtered_out_parents(hide, true);
			preview_pane->tree->set_hide_filtered_out_parents(hide, false);
		} break;
		case PANE_TREE_ACCESSIBILITY_WARNINGS: {
			const bool warnings = !EDITOR_GET("docks/scene_tree/accessibility_warnings");
			dock_tree->set_accessibility_warnings(warnings, true);
			preview_pane->tree->set_accessibility_warnings(warnings, false);
		} break;
		case PANE_TREE_FILTER_BY_TYPE:
		case PANE_TREE_FILTER_BY_GROUP: {
			LineEdit *filter = preview_pane->tree_filter;
			const String parameter = p_option == PANE_TREE_FILTER_BY_TYPE ? "type" : "group";
			filter->set_text((filter->get_text() + " " + parameter + ":").strip_edges());
			_pane_filter_changed(filter->get_text());
			filter->set_caret_column(filter->get_text().length());
			filter->grab_focus();
		} break;
	}
}

void EditorSplitView::_pane_list_subresources() {
	SceneTreeDock::get_singleton()->list_subresources(preview_pane->subresource_menu, get_preview_scene());
}

void EditorSplitView::_pane_edit_subresource(int p_index) {
	const ObjectID id = preview_pane->subresource_menu->get_item_metadata(p_index);
	Object *object = ObjectDB::get_instance(id);
	if (object) {
		EditorNode::get_singleton()->push_item(object);
	}
}

void EditorSplitView::_update_pane_toolbar() {
	// On the script screen the pane's tree is the edited scene's, and the
	// Scene dock has the toolbar for that.
	const bool pane_scene = !showing_live && get_preview_scene();
	preview_pane->tree_toolbar->set_visible(pane_scene);
	preview_pane->tree_title->set_visible(!pane_scene);
	if (!pane_scene) {
		return;
	}
	Node *node = _pane_target_node();
	const Ref<Script> script = node ? node->get_script() : Variant();
	const bool has_script = script.is_valid() && script != EditorNode::get_singleton()->get_object_custom_type_base(node);
	preview_pane->attach_script_button->set_visible(!has_script);
	preview_pane->detach_script_button->set_visible(has_script);
}

// The second pane's view menu: the 3D editor viewport menu's items, for the
// scene shown in the pane.

void EditorSplitView::_build_view_menu() {
	PopupMenu *p = preview_pane->view_menu->get_popup();
	p->add_shortcut(ED_GET_SHORTCUT("spatial_editor/top_view"), PANE_VIEW_TOP);
	p->add_shortcut(ED_GET_SHORTCUT("spatial_editor/bottom_view"), PANE_VIEW_BOTTOM);
	p->add_shortcut(ED_GET_SHORTCUT("spatial_editor/left_view"), PANE_VIEW_LEFT);
	p->add_shortcut(ED_GET_SHORTCUT("spatial_editor/right_view"), PANE_VIEW_RIGHT);
	p->add_shortcut(ED_GET_SHORTCUT("spatial_editor/front_view"), PANE_VIEW_FRONT);
	p->add_shortcut(ED_GET_SHORTCUT("spatial_editor/rear_view"), PANE_VIEW_REAR);
	p->add_separator();
	p->add_shortcut(ED_GET_SHORTCUT("spatial_editor/switch_perspective_orthogonal"), PANE_VIEW_SWITCH_PERSPECTIVE_ORTHOGONAL);
	p->add_radio_check_item(TTRC("Perspective"), PANE_VIEW_PERSPECTIVE);
	p->add_radio_check_item(TTRC("Orthogonal"), PANE_VIEW_ORTHOGONAL);
	p->add_check_item(TTRC("Auto Orthogonal Enabled"), PANE_VIEW_AUTO_ORTHOGONAL);
	p->set_item_checked(p->get_item_index(PANE_VIEW_AUTO_ORTHOGONAL), true);
	p->add_separator();
	p->add_check_shortcut(ED_GET_SHORTCUT("spatial_editor/view_lock_rotation"), PANE_VIEW_LOCK_ROTATION);
	p->add_separator();
	p->add_radio_check_shortcut(ED_GET_SHORTCUT("spatial_editor/view_display_normal"), PANE_VIEW_DISPLAY_BASE + (int)Viewport::DEBUG_DRAW_DISABLED);
	p->add_radio_check_shortcut(ED_GET_SHORTCUT("spatial_editor/view_display_wireframe"), PANE_VIEW_DISPLAY_BASE + (int)Viewport::DEBUG_DRAW_WIREFRAME);
	p->add_radio_check_shortcut(ED_GET_SHORTCUT("spatial_editor/view_display_overdraw"), PANE_VIEW_DISPLAY_BASE + (int)Viewport::DEBUG_DRAW_OVERDRAW);
	p->add_radio_check_shortcut(ED_GET_SHORTCUT("spatial_editor/view_display_lighting"), PANE_VIEW_DISPLAY_BASE + (int)Viewport::DEBUG_DRAW_LIGHTING);
	p->add_radio_check_shortcut(ED_GET_SHORTCUT("spatial_editor/view_display_unshaded"), PANE_VIEW_DISPLAY_BASE + (int)Viewport::DEBUG_DRAW_UNSHADED);
	p->set_item_checked(p->get_item_index(PANE_VIEW_DISPLAY_BASE + (int)Viewport::DEBUG_DRAW_DISABLED), true);

	// As in the 3D editor: each mode, and the renderers that can draw it.
	enum Renderers {
		ALL = 0,
		FORWARD_PLUS = 1,
		FORWARD_PLUS_MOBILE = 2,
	};
	struct AdvancedMode {
		const char *name;
		Viewport::DebugDraw mode;
		Renderers renderers;
		bool separator_before;
	};
	static const AdvancedMode advanced_modes[] = {
		{ TTRC("Directional Shadow Splits"), Viewport::DEBUG_DRAW_PSSM_SPLITS, FORWARD_PLUS_MOBILE, false },
		{ TTRC("Normal Buffer"), Viewport::DEBUG_DRAW_NORMAL_BUFFER, FORWARD_PLUS, true },
		{ TTRC("Shadow Atlas"), Viewport::DEBUG_DRAW_SHADOW_ATLAS, ALL, true },
		{ TTRC("Directional Shadow Map"), Viewport::DEBUG_DRAW_DIRECTIONAL_SHADOW_ATLAS, ALL, false },
		{ TTRC("Decal Atlas"), Viewport::DEBUG_DRAW_DECAL_ATLAS, FORWARD_PLUS_MOBILE, true },
		{ TTRC("AreaLight3D Atlas"), Viewport::DEBUG_DRAW_AREA_LIGHT_ATLAS, FORWARD_PLUS_MOBILE, false },
		{ TTRC("VoxelGI Lighting"), Viewport::DEBUG_DRAW_VOXEL_GI_LIGHTING, FORWARD_PLUS, true },
		{ TTRC("VoxelGI Albedo"), Viewport::DEBUG_DRAW_VOXEL_GI_ALBEDO, FORWARD_PLUS, false },
		{ TTRC("VoxelGI Emission"), Viewport::DEBUG_DRAW_VOXEL_GI_EMISSION, FORWARD_PLUS, false },
		{ TTRC("SDFGI Cascades"), Viewport::DEBUG_DRAW_SDFGI, FORWARD_PLUS, true },
		{ TTRC("SDFGI Probes"), Viewport::DEBUG_DRAW_SDFGI_PROBES, FORWARD_PLUS, false },
		{ TTRC("Scene Luminance"), Viewport::DEBUG_DRAW_SCENE_LUMINANCE, FORWARD_PLUS_MOBILE, true },
		{ TTRC("SSAO"), Viewport::DEBUG_DRAW_SSAO, FORWARD_PLUS, true },
		{ TTRC("SSIL"), Viewport::DEBUG_DRAW_SSIL, FORWARD_PLUS, false },
		{ TTRC("VoxelGI/SDFGI Buffer"), Viewport::DEBUG_DRAW_GI_BUFFER, FORWARD_PLUS, true },
		{ TTRC("Disable Mesh LOD"), Viewport::DEBUG_DRAW_DISABLE_LOD, ALL, true },
		{ TTRC("OmniLight3D Cluster"), Viewport::DEBUG_DRAW_CLUSTER_OMNI_LIGHTS, FORWARD_PLUS, true },
		{ TTRC("SpotLight3D Cluster"), Viewport::DEBUG_DRAW_CLUSTER_SPOT_LIGHTS, FORWARD_PLUS, false },
		{ TTRC("AreaLight3D Cluster"), Viewport::DEBUG_DRAW_CLUSTER_AREA_LIGHTS, FORWARD_PLUS, false },
		{ TTRC("Decal Cluster"), Viewport::DEBUG_DRAW_CLUSTER_DECALS, FORWARD_PLUS, false },
		{ TTRC("ReflectionProbe Cluster"), Viewport::DEBUG_DRAW_CLUSTER_REFLECTION_PROBES, FORWARD_PLUS, false },
		{ TTRC("Occlusion Culling Buffer"), Viewport::DEBUG_DRAW_OCCLUDERS, FORWARD_PLUS_MOBILE, false },
		{ TTRC("Motion Vectors"), Viewport::DEBUG_DRAW_MOTION_VECTORS, FORWARD_PLUS, false },
		{ TTRC("Internal Buffer"), Viewport::DEBUG_DRAW_INTERNAL_BUFFER, FORWARD_PLUS_MOBILE, false },
	};
	const String method = RenderingServer::get_singleton()->get_current_rendering_method();
	PopupMenu *advanced = preview_pane->view_advanced_menu;
	for (const AdvancedMode &mode : advanced_modes) {
		if (mode.separator_before) {
			advanced->add_separator();
		}
		advanced->add_radio_check_item(mode.name, PANE_VIEW_DISPLAY_BASE + (int)mode.mode);
		const bool supported = mode.renderers == ALL || method == "forward_plus" || (mode.renderers == FORWARD_PLUS_MOBILE && method == "mobile");
		if (!supported) {
			advanced->set_item_disabled(-1, true);
			advanced->set_item_tooltip(-1, TTR("Not available when using the Compatibility renderer."));
		}
	}
	p->add_submenu_node_item(TTRC("Display Advanced..."), advanced, PANE_VIEW_DISPLAY_ADVANCED);

	p->add_separator();
	p->add_check_shortcut(ED_GET_SHORTCUT("spatial_editor/view_environment"), PANE_VIEW_ENVIRONMENT);
	p->add_check_shortcut(ED_GET_SHORTCUT("spatial_editor/view_gizmos"), PANE_VIEW_GIZMOS);
	p->add_check_shortcut(ED_GET_SHORTCUT("spatial_editor/view_transform_gizmo"), PANE_VIEW_TRANSFORM_GIZMO);
	p->add_check_shortcut(ED_GET_SHORTCUT("spatial_editor/view_grid_lines"), PANE_VIEW_GRID);
	p->add_check_shortcut(ED_GET_SHORTCUT("spatial_editor/view_information"), PANE_VIEW_INFORMATION);
	p->add_check_shortcut(ED_GET_SHORTCUT("spatial_editor/view_fps"), PANE_VIEW_FRAME_TIME);
	p->set_item_checked(p->get_item_index(PANE_VIEW_ENVIRONMENT), true);
	p->set_item_checked(p->get_item_index(PANE_VIEW_GIZMOS), true);
	p->set_item_checked(p->get_item_index(PANE_VIEW_TRANSFORM_GIZMO), true);
	p->set_item_checked(p->get_item_index(PANE_VIEW_GRID), true);
	p->add_separator();
	p->add_check_shortcut(ED_GET_SHORTCUT("spatial_editor/view_half_resolution"), PANE_VIEW_HALF_RESOLUTION);
	p->add_separator();
	p->add_check_shortcut(ED_GET_SHORTCUT("spatial_editor/view_audio_listener"), PANE_VIEW_AUDIO_LISTENER);
	p->add_check_shortcut(ED_GET_SHORTCUT("spatial_editor/view_audio_doppler"), PANE_VIEW_AUDIO_DOPPLER);
	p->add_separator();
	p->add_check_shortcut(ED_GET_SHORTCUT("spatial_editor/view_cinematic_preview"), PANE_VIEW_CINEMATIC_PREVIEW);
	p->add_separator();
	p->add_shortcut(ED_GET_SHORTCUT("spatial_editor/focus_origin"), PANE_VIEW_CENTER_TO_ORIGIN);
	p->add_shortcut(ED_GET_SHORTCUT("spatial_editor/focus_selection"), PANE_VIEW_CENTER_TO_SELECTION);
	p->add_shortcut(ED_GET_SHORTCUT("spatial_editor/focus_aabb"), PANE_VIEW_CENTER_TO_AABB);
	p->add_shortcut(ED_GET_SHORTCUT("spatial_editor/align_transform_with_view"), PANE_VIEW_ALIGN_TRANSFORM_WITH_VIEW);
	p->add_shortcut(ED_GET_SHORTCUT("spatial_editor/align_rotation_with_view"), PANE_VIEW_ALIGN_ROTATION_WITH_VIEW);

	p->connect(SceneStringName(id_pressed), callable_mp(this, &EditorSplitView::_view_menu_option));
	advanced->connect(SceneStringName(id_pressed), callable_mp(this, &EditorSplitView::_view_menu_option));
	p->connect("about_to_popup", callable_mp(this, &EditorSplitView::_set_preview_active).bind(true));
}

bool EditorSplitView::_view_menu_checked(int p_option) const {
	const PopupMenu *p = preview_pane->view_menu->get_popup();
	int idx = p->get_item_index(p_option);
	if (idx >= 0) {
		return p->is_item_checked(idx);
	}
	idx = preview_pane->view_advanced_menu->get_item_index(p_option);
	return idx >= 0 && preview_pane->view_advanced_menu->is_item_checked(idx);
}

void EditorSplitView::_set_view_menu_checked(int p_option, bool p_checked) {
	PopupMenu *p = preview_pane->view_menu->get_popup();
	int idx = p->get_item_index(p_option);
	if (idx >= 0) {
		p->set_item_checked(idx, p_checked);
	}
	idx = preview_pane->view_advanced_menu->get_item_index(p_option);
	if (idx >= 0) {
		preview_pane->view_advanced_menu->set_item_checked(idx, p_checked);
	}
}

bool EditorSplitView::_view_menu_shortcut(const Ref<InputEvent> &p_event) {
	// The 3D editor's viewport shortcuts, while the pane has focus.
	const Ref<InputEventKey> k = p_event;
	if (k.is_null() || !k->is_pressed() || k->is_echo()) {
		return false;
	}
	const PopupMenu *p = preview_pane->view_menu->get_popup();
	for (int i = 0; i < p->get_item_count(); i++) {
		const Ref<Shortcut> shortcut = p->get_item_shortcut(i);
		if (shortcut.is_valid() && !p->is_item_disabled(i) && shortcut->matches_event(p_event)) {
			_view_menu_option(p->get_item_id(i));
			return true;
		}
	}
	return false;
}

void EditorSplitView::_orbit_preview(const Vector2 &p_relative) {
	if (_view_menu_checked(PANE_VIEW_LOCK_ROTATION)) {
		return;
	}
	// Orbit, out of a side view.
	_begin_custom_view();
	_end_side_view();
	preview_orbit.x_rot = CLAMP(preview_orbit.x_rot + p_relative.y * 0.005, -Math::PI / 2, Math::PI / 2);
	preview_orbit.y_rot += p_relative.x * 0.005;
	_update_preview_camera();
}

Basis EditorSplitView::_orbit_basis(const OrbitView &p_orbit) {
	// As _apply_orbit() turns the camera.
	Basis basis;
	basis.rotate(Vector3(1, 0, 0), -p_orbit.x_rot);
	basis.rotate(Vector3(0, 1, 0), -p_orbit.y_rot);
	return basis;
}

bool EditorSplitView::_is_shortcut_held(const String &p_name) {
	const Ref<Shortcut> shortcut = ED_GET_SHORTCUT(p_name);
	if (shortcut.is_null()) {
		return false;
	}
	for (const Ref<InputEventKey> k : shortcut->get_events()) {
		if (k.is_valid() && (Input::get_singleton()->is_key_pressed(k->get_keycode()) || Input::get_singleton()->is_physical_key_pressed(k->get_physical_keycode()))) {
			return true;
		}
	}
	return false;
}

void EditorSplitView::_set_freelook(bool p_enabled, const Point2 &p_mouse_position) {
	if (preview_freelook == p_enabled) {
		return;
	}
	preview_freelook = p_enabled;
	if (p_enabled) {
		_set_preview_active(true);
		preview_pane->viewport_container->grab_focus();
		// Looking around from where the camera is, in perspective.
		_begin_custom_view();
		_end_side_view();
		if (preview_orbit.orthogonal) {
			preview_orbit.orthogonal = false;
			_update_preview_camera();
		}
		if (!bool(EDITOR_GET("editors/3d/freelook/freelook_speed_zoom_link"))) {
			freelook_speed = MAX((real_t)EDITOR_GET("editors/3d/freelook/freelook_base_speed"), (real_t)0.01);
		} else {
			freelook_speed = MAX((real_t)EDITOR_GET("editors/3d/freelook/freelook_base_speed") * preview_orbit.distance, (real_t)0.01);
		}
		freelook_mouse_start = p_mouse_position;
		Input::get_singleton()->set_mouse_mode(Input::MouseMode::MOUSE_MODE_CAPTURED);
	} else {
		Input::get_singleton()->set_mouse_mode(Input::MouseMode::MOUSE_MODE_VISIBLE);
		Input::get_singleton()->warp_mouse(freelook_mouse_start);
		zoom_indicator_delay = 0.0;
		preview_pane->overlay->queue_redraw();
	}
	_update_view_menu();
}

void EditorSplitView::_freelook_look(const Vector2 &p_relative) {
	// Turn around the camera, not around what it orbits.
	const Vector3 eye = preview_orbit.target + _orbit_basis(preview_orbit).xform(Vector3(0, 0, preview_orbit.distance));
	const real_t radians_per_pixel = Math::deg_to_rad((real_t)EDITOR_GET("editors/3d/freelook/freelook_sensitivity"));
	const bool invert_y = EDITOR_GET("editors/3d/freelook/freelook_invert_y_axis");
	preview_orbit.x_rot = CLAMP(preview_orbit.x_rot + p_relative.y * radians_per_pixel * (invert_y ? -1 : 1), -Math::PI / 2, Math::PI / 2);
	preview_orbit.y_rot += p_relative.x * radians_per_pixel;
	preview_orbit.target = eye - _orbit_basis(preview_orbit).xform(Vector3(0, 0, preview_orbit.distance));
	_update_preview_camera();
}

void EditorSplitView::_freelook_move(double p_delta) {
	const Basis basis = _orbit_basis(preview_orbit);
	Vector3 forward = basis.xform(Vector3(0, 0, -1));
	Vector3 up = basis.xform(Vector3(0, 1, 0));
	if (int(EDITOR_GET("editors/3d/freelook/freelook_navigation_scheme")) != 0) {
		// The axis-locked schemes: level forward, and straight up.
		forward = Vector3(0, 0, -1).rotated(Vector3(0, 1, 0), -preview_orbit.y_rot);
		up = Vector3(0, 1, 0);
	}
	const Vector3 right = basis.xform(Vector3(1, 0, 0));
	Vector3 direction;
	if (_is_shortcut_held("spatial_editor/freelook_left")) {
		direction -= right;
	}
	if (_is_shortcut_held("spatial_editor/freelook_right")) {
		direction += right;
	}
	if (_is_shortcut_held("spatial_editor/freelook_forward")) {
		direction += forward;
	}
	if (_is_shortcut_held("spatial_editor/freelook_backwards")) {
		direction -= forward;
	}
	if (_is_shortcut_held("spatial_editor/freelook_up")) {
		direction += up;
	}
	if (_is_shortcut_held("spatial_editor/freelook_down")) {
		direction -= up;
	}
	if (direction.is_zero_approx()) {
		return;
	}
	real_t speed = freelook_speed;
	if (_is_shortcut_held("spatial_editor/freelook_speed_modifier")) {
		speed *= 3.0;
	}
	if (_is_shortcut_held("spatial_editor/freelook_slow_modifier")) {
		speed *= 0.333333;
	}
	preview_orbit.target += direction * speed * p_delta;
	_update_preview_camera();
}

void EditorSplitView::_rotation_axis_clicked(int p_axis) {
	// As the 3D editor's gizmo: look from the axis clicked.
	static const PaneViewMenu views[6] = { PANE_VIEW_RIGHT, PANE_VIEW_TOP, PANE_VIEW_FRONT, PANE_VIEW_LEFT, PANE_VIEW_BOTTOM, PANE_VIEW_REAR };
	ERR_FAIL_INDEX(p_axis, 6);
	_view_menu_option(views[p_axis]);
}

void EditorSplitView::_end_side_view() {
	if (preview_view_type == PANE_VIEW_TYPE_USER && !preview_auto_orthogonal) {
		return;
	}
	preview_view_type = PANE_VIEW_TYPE_USER;
	if (preview_auto_orthogonal) {
		// Made orthogonal by the side view: back to perspective.
		preview_auto_orthogonal = false;
		preview_orbit.orthogonal = false;
	}
	_update_view_menu();
}

Camera3D *EditorSplitView::_find_scene_camera(Node *p_node) const {
	// The scene's current camera, or its first one.
	Camera3D *first = nullptr;
	List<Node *> pending;
	pending.push_back(p_node);
	while (!pending.is_empty()) {
		Node *node = pending.front()->get();
		pending.pop_front();
		if (Camera3D *camera = Object::cast_to<Camera3D>(node)) {
			if (camera->is_current()) {
				return camera;
			}
			if (!first) {
				first = camera;
			}
		}
		for (Node *child : node->iterate_children()) {
			pending.push_back(child);
		}
	}
	return first;
}

void EditorSplitView::_set_cinematic_preview(bool p_enabled) {
	preview_cinematic = p_enabled;
	_set_view_menu_checked(PANE_VIEW_CINEMATIC_PREVIEW, p_enabled);
	if (!p_enabled) {
		preview_cinematic_camera = ObjectID();
		RenderingServer::get_singleton()->viewport_attach_camera(preview_pane->viewport->get_viewport_rid(), preview_pane->camera_3d->get_camera());
	}
	_update_view_menu();
	_update_view_panels();
}

void EditorSplitView::_focus_preview_bounds(bool p_frame) {
	Node *root = get_preview_scene();
	if (!root) {
		return;
	}
	_begin_custom_view();
	Node *selected = preview_pane->tools->get_selected();
	Node3D *selected_3d = Object::cast_to<Node3D>(selected);
	if (!p_frame) {
		// Focus Selection: look at it from where the view is.
		if (selected_3d) {
			preview_orbit.target = selected_3d->get_global_position();
		}
	} else {
		// Focus and Frame: everything it shows, or the whole scene.
		AABB bounds;
		bool has_bounds = false;
		_collect_bounds(selected ? selected : root, bounds, has_bounds);
		if (has_bounds) {
			preview_orbit.target = bounds.get_center();
			preview_orbit.distance = MAX(bounds.get_longest_axis_size(), (real_t)0.1) * 1.1;
		} else if (selected_3d) {
			preview_orbit.target = selected_3d->get_global_position();
		}
	}
	_update_preview_camera();
}

void EditorSplitView::_align_selection_with_view(bool p_rotation_only) {
	Node3D *node = Object::cast_to<Node3D>(preview_pane->tools->get_selected());
	const int history_id = preview_pane->tools->get_history_id();
	if (!node || history_id == EditorUndoRedoManager::INVALID_HISTORY) {
		return;
	}
	const Transform3D camera_xform = preview_pane->camera_3d->get_global_transform();
	EditorUndoRedoManager *undo_redo = EditorUndoRedoManager::get_singleton();
	if (p_rotation_only) {
		Basis basis = camera_xform.basis;
		if (Object::cast_to<Decal>(node)) {
			// As in the 3D editor: a decal looks the way the camera does.
			basis.rotate_local(Vector3(1, 0, 0), Math::TAU * 0.25);
		}
		undo_redo->create_action_for_history(TTR("Align Rotation with View"), history_id);
		undo_redo->force_fixed_history();
		undo_redo->add_do_method(node, "set_rotation", basis.get_euler_normalized());
		undo_redo->add_undo_method(node, "set_rotation", node->get_rotation());
		undo_redo->commit_action();
		return;
	}
	Transform3D xform = camera_xform;
	if (preview_orbit.orthogonal) {
		xform.origin = preview_orbit.target + camera_xform.basis.xform(Vector3(0, 0, preview_orbit.distance));
	} else {
		xform.scale_basis(node->get_scale());
	}
	if (Object::cast_to<Decal>(node)) {
		xform.basis.rotate_local(Vector3(1, 0, 0), Math::TAU * 0.25);
	}
	Node3D *parent = node->get_parent_node_3d();
	const Transform3D local_xform = parent ? parent->get_global_transform().affine_inverse() * xform : xform;
	undo_redo->create_action_for_history(TTR("Align Transform with View"), history_id);
	undo_redo->force_fixed_history();
	undo_redo->add_do_method(node, "set_transform", local_xform);
	undo_redo->add_undo_method(node, "set_transform", node->get_transform());
	undo_redo->commit_action();
}

void EditorSplitView::_view_menu_option(int p_option) {
	_set_preview_active(true);
	static const real_t side_views[6][2] = {
		// x_rot, y_rot.
		{ Math::PI / 2.0, 0 }, // Top.
		{ -Math::PI / 2.0, 0 }, // Bottom.
		{ 0, Math::PI / 2.0 }, // Left.
		{ 0, -Math::PI / 2.0 }, // Right.
		{ 0, 0 }, // Front.
		{ 0, Math::PI }, // Rear.
	};
	if (p_option >= PANE_VIEW_DISPLAY_BASE) {
		const Viewport::DebugDraw mode = Viewport::DebugDraw(p_option - PANE_VIEW_DISPLAY_BASE);
		for (int i = 0; i <= Viewport::DEBUG_DRAW_AREA_LIGHT_ATLAS; i++) {
			_set_view_menu_checked(PANE_VIEW_DISPLAY_BASE + i, i == mode);
		}
		if (mode == Viewport::DEBUG_DRAW_WIREFRAME) {
			RenderingServer::get_singleton()->set_debug_generate_wireframes(true);
		}
		preview_pane->viewport->set_debug_draw(mode);
		return;
	}
	switch (p_option) {
		case PANE_VIEW_TOP:
		case PANE_VIEW_BOTTOM:
		case PANE_VIEW_LEFT:
		case PANE_VIEW_RIGHT:
		case PANE_VIEW_FRONT:
		case PANE_VIEW_REAR: {
			_begin_custom_view();
			preview_orbit.x_rot = side_views[p_option - PANE_VIEW_TOP][0];
			preview_orbit.y_rot = side_views[p_option - PANE_VIEW_TOP][1];
			preview_view_type = PaneViewType(PANE_VIEW_TYPE_TOP + p_option - PANE_VIEW_TOP);
			if (_view_menu_checked(PANE_VIEW_AUTO_ORTHOGONAL) && !preview_orbit.orthogonal) {
				preview_orbit.orthogonal = true;
				preview_auto_orthogonal = true;
			}
			_update_preview_camera();
		} break;
		case PANE_VIEW_SWITCH_PERSPECTIVE_ORTHOGONAL:
		case PANE_VIEW_PERSPECTIVE:
		case PANE_VIEW_ORTHOGONAL: {
			_begin_custom_view();
			if (p_option == PANE_VIEW_SWITCH_PERSPECTIVE_ORTHOGONAL) {
				preview_orbit.orthogonal = !preview_orbit.orthogonal;
			} else {
				preview_orbit.orthogonal = p_option == PANE_VIEW_ORTHOGONAL;
			}
			preview_auto_orthogonal = false;
			_update_preview_camera();
		} break;
		case PANE_VIEW_AUTO_ORTHOGONAL: {
			const bool checked = !_view_menu_checked(PANE_VIEW_AUTO_ORTHOGONAL);
			_set_view_menu_checked(PANE_VIEW_AUTO_ORTHOGONAL, checked);
			if (!checked && preview_auto_orthogonal) {
				preview_auto_orthogonal = false;
				preview_orbit.orthogonal = false;
				_update_preview_camera();
			}
		} break;
		case PANE_VIEW_ENVIRONMENT: {
			const bool checked = !_view_menu_checked(PANE_VIEW_ENVIRONMENT);
			_set_view_menu_checked(PANE_VIEW_ENVIRONMENT, checked);
			preview_pane->camera_3d->set_environment(checked ? Ref<Environment>() : Node3DEditor::get_singleton()->get_viewport_environment());
		} break;
		case PANE_VIEW_LOCK_ROTATION:
		case PANE_VIEW_GIZMOS:
		case PANE_VIEW_TRANSFORM_GIZMO:
		case PANE_VIEW_GRID:
		case PANE_VIEW_INFORMATION:
		case PANE_VIEW_FRAME_TIME: {
			_set_view_menu_checked(p_option, !_view_menu_checked(p_option));
			if (p_option == PANE_VIEW_FRAME_TIME) {
				for (int i = 0; i < FRAME_TIME_HISTORY; i++) {
					// As the 3D editor does: start from 120 FPS.
					cpu_time_history[i] = 8.333333;
					gpu_time_history[i] = 8.333333;
				}
				RenderingServer::get_singleton()->viewport_set_measure_render_time(preview_pane->viewport->get_viewport_rid(), _view_menu_checked(PANE_VIEW_FRAME_TIME));
			}
		} break;
		case PANE_VIEW_HALF_RESOLUTION: {
			const bool checked = !_view_menu_checked(PANE_VIEW_HALF_RESOLUTION);
			_set_view_menu_checked(PANE_VIEW_HALF_RESOLUTION, checked);
			preview_pane->viewport->set_scaling_3d_scale(checked ? 0.5 : 1.0);
		} break;
		case PANE_VIEW_AUDIO_LISTENER: {
			const bool checked = !_view_menu_checked(PANE_VIEW_AUDIO_LISTENER);
			_set_view_menu_checked(PANE_VIEW_AUDIO_LISTENER, checked);
			preview_pane->viewport->set_as_audio_listener_3d(checked);
		} break;
		case PANE_VIEW_AUDIO_DOPPLER: {
			const bool checked = !_view_menu_checked(PANE_VIEW_AUDIO_DOPPLER);
			_set_view_menu_checked(PANE_VIEW_AUDIO_DOPPLER, checked);
			preview_pane->camera_3d->set_doppler_tracking(checked ? Camera3D::DOPPLER_TRACKING_IDLE_STEP : Camera3D::DOPPLER_TRACKING_DISABLED);
		} break;
		case PANE_VIEW_CINEMATIC_PREVIEW: {
			_set_cinematic_preview(!preview_cinematic);
		} break;
		case PANE_VIEW_CENTER_TO_ORIGIN: {
			_begin_custom_view();
			preview_orbit.target = Vector3();
			_update_preview_camera();
		} break;
		case PANE_VIEW_CENTER_TO_SELECTION: {
			_focus_preview_bounds(false);
		} break;
		case PANE_VIEW_CENTER_TO_AABB: {
			_focus_preview_bounds(true);
		} break;
		case PANE_VIEW_ALIGN_TRANSFORM_WITH_VIEW: {
			_align_selection_with_view(false);
		} break;
		case PANE_VIEW_ALIGN_ROTATION_WITH_VIEW: {
			_align_selection_with_view(true);
		} break;
		default:
			break;
	}
	_update_view_menu();
}

void EditorSplitView::_update_view_menu() {
	if (!preview_pane->grid) {
		return; // Not built yet.
	}
	Node *root = get_preview_scene();
	const bool view_3d = enabled && Object::cast_to<Node3D>(root) && !showing_live && !showing_scene_code;
	MenuButton *menu = preview_pane->view_menu;
	menu->set_visible(view_3d);
	if (!view_3d && preview_cinematic) {
		_set_cinematic_preview(false);
		return;
	}

	bool orthogonal = preview_orbit.orthogonal;
	if (!preview_view_custom && root) {
		const int idx = _find_scene_index(preview_scene);
		orthogonal = _orbit_from_state(root, EditorNode::get_editor_data().get_scene_editor_states(idx).get("3D", Dictionary())).orthogonal;
	}
	_set_view_menu_checked(PANE_VIEW_PERSPECTIVE, !orthogonal);
	_set_view_menu_checked(PANE_VIEW_ORTHOGONAL, orthogonal);

	// Named as the 3D editor names its viewports.
	static const char *names[7][2] = {
		{ TTRC("Perspective"), TTRC("Orthogonal") },
		{ TTRC("Top Perspective"), TTRC("Top Orthogonal") },
		{ TTRC("Bottom Perspective"), TTRC("Bottom Orthogonal") },
		{ TTRC("Left Perspective"), TTRC("Left Orthogonal") },
		{ TTRC("Right Perspective"), TTRC("Right Orthogonal") },
		{ TTRC("Front Perspective"), TTRC("Front Orthogonal") },
		{ TTRC("Rear Perspective"), TTRC("Rear Orthogonal") },
	};
	String name = TTR(names[preview_view_type][orthogonal ? 1 : 0]);
	if (preview_auto_orthogonal) {
		name += " " + TTR("[auto]");
	}
	if (preview_cinematic) {
		name = TTR("Cinematic Preview");
		if (preview_cinematic_camera.is_null()) {
			name += " " + TTR("(no Camera3D)");
		}
	}
	if (menu->get_text() != name) {
		menu->set_text(name);
		menu->reset_size();
	}

	preview_pane->grid->set_visible(view_3d && !preview_cinematic && _view_menu_checked(PANE_VIEW_GRID));
	preview_pane->rotation_control->set_visible(view_3d && !preview_cinematic && bool(EDITOR_GET("editors/3d/navigation/show_viewport_rotation_gizmo")));
	preview_pane->rotation_control->queue_redraw();
	const bool scene_2d = !Object::cast_to<Node3D>(root);
	preview_pane->tools->set_show_selection(scene_2d || (!preview_cinematic && _view_menu_checked(PANE_VIEW_GIZMOS)));
	preview_pane->tools->set_show_handles(scene_2d || (!preview_cinematic && _view_menu_checked(PANE_VIEW_TRANSFORM_GIZMO)));
	preview_pane->info_panel->set_visible(view_3d && _view_menu_checked(PANE_VIEW_INFORMATION));
	preview_pane->frame_time_panel->set_visible(view_3d && _view_menu_checked(PANE_VIEW_FRAME_TIME));
	if (!view_3d && preview_freelook) {
		_set_freelook(false);
	}
	set_process(view_3d && (preview_cinematic || preview_freelook || zoom_indicator_delay > 0.0 || _view_menu_checked(PANE_VIEW_INFORMATION) || _view_menu_checked(PANE_VIEW_FRAME_TIME)));
}

void EditorSplitView::_update_view_panels() {
	Node *root = get_preview_scene();
	SubViewport *viewport = preview_pane->viewport;
	if (preview_cinematic && root) {
		// Look through the scene's camera, as the 3D editor does.
		Camera3D *camera = _find_scene_camera(root);
		const ObjectID id = camera ? camera->get_instance_id() : ObjectID();
		RenderingServer::get_singleton()->viewport_attach_camera(viewport->get_viewport_rid(), camera ? camera->get_camera() : preview_pane->camera_3d->get_camera());
		if (id != preview_cinematic_camera) {
			preview_cinematic_camera = id;
			_update_view_menu();
		}
	}
	if (preview_pane->info_panel->is_visible()) {
		const Camera3D *camera = ObjectDB::get_instance<Camera3D>(preview_cinematic_camera);
		if (!camera) {
			camera = preview_pane->camera_3d;
		}
		const Vector3 position = camera->get_global_position();
		const real_t scale = viewport->get_scaling_3d_scale();
		String text;
		text += vformat(TTR("X: %s"), rtos(position.x).pad_decimals(1)) + "\n";
		text += vformat(TTR("Y: %s"), rtos(position.y).pad_decimals(1)) + "\n";
		text += vformat(TTR("Z: %s"), rtos(position.z).pad_decimals(1)) + "\n\n";
		text += vformat(TTR("Size: %s (%.1fMP)") + "\n", vformat(U"%d × %d", viewport->get_size().x * scale, viewport->get_size().y * scale), viewport->get_size().x * viewport->get_size().y * scale * scale * 0.000001);
		text += "\n";
		text += vformat(TTR("Objects: %d"), viewport->get_render_info(Viewport::RENDER_INFO_TYPE_VISIBLE, Viewport::RENDER_INFO_OBJECTS_IN_FRAME)) + "\n";
		text += vformat(TTR("Primitives: %d"), viewport->get_render_info(Viewport::RENDER_INFO_TYPE_VISIBLE, Viewport::RENDER_INFO_PRIMITIVES_IN_FRAME)) + "\n";
		text += vformat(TTR("Draw Calls: %d"), viewport->get_render_info(Viewport::RENDER_INFO_TYPE_VISIBLE, Viewport::RENDER_INFO_DRAW_CALLS_IN_FRAME));
		preview_pane->info_label->set_text(text);
	}
	if (preview_pane->frame_time_panel->is_visible()) {
		const RID rid = viewport->get_viewport_rid();
		cpu_time_history[frame_time_history_index] = RenderingServer::get_singleton()->viewport_get_measured_render_time_cpu(rid);
		gpu_time_history[frame_time_history_index] = RenderingServer::get_singleton()->viewport_get_measured_render_time_gpu(rid);
		frame_time_history_index = (frame_time_history_index + 1) % FRAME_TIME_HISTORY;
		double cpu_time = 0.0;
		double gpu_time = 0.0;
		for (int i = 0; i < FRAME_TIME_HISTORY; i++) {
			cpu_time += cpu_time_history[i];
			gpu_time += gpu_time_history[i];
		}
		// As the 3D editor shows it: averaged, and never quite zero.
		cpu_time = MAX(0.01, cpu_time / FRAME_TIME_HISTORY);
		gpu_time = MAX(0.01, gpu_time / FRAME_TIME_HISTORY);
		const double fps = 1000.0 / gpu_time;
		const Color good = get_theme_color(SNAME("success_color"), EditorStringName(Editor));
		const Color ok = get_theme_color(SNAME("warning_color"), EditorStringName(Editor));
		const Color bad = get_theme_color(SNAME("error_color"), EditorStringName(Editor));
		const auto color_for = [&](double p_amount) {
			// 0: good, 0.5: OK, 1: bad.
			const double amount = CLAMP(p_amount, 0.0, 1.0);
			return amount < 0.5 ? good.lerp(ok, amount * 2.0) : ok.lerp(bad, (amount - 0.5) * 2.0);
		};
		preview_pane->cpu_time_label->set_text(vformat(TTR("CPU Time: %s ms"), rtos(cpu_time).pad_decimals(2)));
		preview_pane->cpu_time_label->add_theme_color_override(SceneStringName(font_color), color_for(cpu_time / 30.0));
		preview_pane->gpu_time_label->set_text(vformat(TTR("GPU Time: %s ms"), rtos(gpu_time).pad_decimals(2)));
		preview_pane->gpu_time_label->add_theme_color_override(SceneStringName(font_color), color_for(gpu_time / 30.0));
		preview_pane->fps_label->set_text(vformat(TTR("FPS: %d"), fps));
		preview_pane->fps_label->add_theme_color_override(SceneStringName(font_color), color_for((110.0 - fps) / 100.0));
	}
}

void EditorSplitView::_build_preview_grid() {
	// A grid on the floor, as the 3D editor draws one; in the pane's own world.
	const int extent = 100;
	const Ref<Theme> &theme = EditorNode::get_singleton()->get_editor_theme();
	const Color minor = Color(0.5, 0.5, 0.5, 0.18);
	const Color major = Color(0.5, 0.5, 0.5, 0.4);
	const Color x_axis = theme->get_color(SNAME("axis_x_color"), EditorStringName(Editor));
	const Color z_axis = theme->get_color(SNAME("axis_z_color"), EditorStringName(Editor));
	PackedVector3Array points;
	PackedColorArray colors;
	for (int i = -extent; i <= extent; i++) {
		const Color line = i % 10 == 0 ? major : minor;
		// Along X, and the X axis itself.
		points.push_back(Vector3(-extent, 0, i));
		points.push_back(Vector3(extent, 0, i));
		colors.push_back(i == 0 ? x_axis : line);
		colors.push_back(i == 0 ? x_axis : line);
		// Along Z, and the Z axis itself.
		points.push_back(Vector3(i, 0, -extent));
		points.push_back(Vector3(i, 0, extent));
		colors.push_back(i == 0 ? z_axis : line);
		colors.push_back(i == 0 ? z_axis : line);
	}
	Array arrays;
	arrays.resize(Mesh::ARRAY_MAX);
	arrays[Mesh::ARRAY_VERTEX] = points;
	arrays[Mesh::ARRAY_COLOR] = colors;
	Ref<ArrayMesh> mesh;
	mesh.instantiate();
	mesh->add_surface_from_arrays(Mesh::PRIMITIVE_LINES, arrays);
	Ref<StandardMaterial3D> grid_material;
	grid_material.instantiate();
	grid_material->set_shading_mode(BaseMaterial3D::SHADING_MODE_UNSHADED);
	grid_material->set_transparency(BaseMaterial3D::TRANSPARENCY_ALPHA);
	grid_material->set_flag(BaseMaterial3D::FLAG_ALBEDO_FROM_VERTEX_COLOR, true);
	grid_material->set_flag(BaseMaterial3D::FLAG_SRGB_VERTEX_COLOR, true);
	grid_material->set_cull_mode(BaseMaterial3D::CULL_DISABLED);
	mesh->surface_set_material(0, grid_material);

	MeshInstance3D *grid = memnew(MeshInstance3D);
	grid->set_mesh(mesh);
	grid->set_cast_shadows_setting(GeometryInstance3D::SHADOW_CASTING_SETTING_OFF);
	grid->hide();
	preview_pane->viewport->add_child(grid);
	preview_pane->grid = grid;
	_update_view_menu();
}

void EditorSplitView::_set_preview_active(bool p_active) {
	if (preview_active == p_active) {
		return;
	}
	preview_active = p_active;
	_update_active_pane();
}

void EditorSplitView::_update_active_pane() {
	// The second pane can only be edited while it shows a scene.
	const bool pane_scene = enabled && !showing_live && !showing_scene_code && get_preview_scene();
	focus_pane->set_active(!(preview_active && pane_scene));
	preview_pane->set_active(preview_active && pane_scene, pane_scene);
	// Undo and redo in the menu follow the side being edited.
	EditorNode::get_singleton()->update_undo_redo_allowed();
	_sync_main_inspector();
}

void EditorSplitView::_sync_main_inspector() {
	if (get_active_history_id() != EditorUndoRedoManager::INVALID_HISTORY) {
		// What's selected in the second pane, or its scene.
		Node *node = preview_pane->tools->get_selected();
		if (!node) {
			node = get_preview_scene();
		}
		pane_in_main_inspector = true;
		if (InspectorDock::get_inspector_singleton()->get_edited_object() != node) {
			InspectorDock::get_singleton()->edit_outside_history(node);
			SignalsDock::get_singleton()->set_object(node);
			GroupsDock::get_singleton()->set_selection(Vector<Node *>{ node });
		}
	} else if (pane_in_main_inspector) {
		// Back to what's selected in the scene being edited, once the editor
		// is done with whatever made the second pane stop being edited.
		pane_in_main_inspector = false;
		callable_mp(this, &EditorSplitView::_restore_main_inspector).call_deferred();
	}
}

void EditorSplitView::_restore_main_inspector() {
	if (!pane_in_main_inspector) {
		EditorNode::get_singleton()->edit_current();
	}
}

int EditorSplitView::get_active_history_id() const {
	if (!preview_active || !enabled || showing_live || showing_scene_code || !preview_pane->is_visible_in_tree() || !get_preview_scene()) {
		return EditorUndoRedoManager::INVALID_HISTORY;
	}
	return preview_pane->tools->get_history_id();
}

void EditorSplitView::_pane_selection_changed() {
	// The second pane's own selection: its tree and inspector follow it.
	Node *root = get_preview_scene();
	Node *selected = preview_pane->tools->get_selected();
	Node *inspected = selected ? selected : root;
	preview_pane->inspector->edit(inspected);
	preview_pane->inspector_title->set_text(inspected ? vformat(TTR("Inspector: %s"), inspected->get_name()) : TTR("Inspector"));
	if (preview_pane->tree->get_selected() != selected || !selected) {
		preview_pane->tree->set_selected(nullptr, false);
		preview_pane->tree->get_scene_tree()->deselect_all();
		if (selected) {
			preview_pane->tree->set_selected(selected, false);
		}
	}
	preview_pane->overlay->queue_redraw();
	_update_pane_toolbar();
	_sync_main_inspector();
}

void EditorSplitView::input(const Ref<InputEvent> &p_event) {
	// Undo and redo go to the side clicked last.
	const Ref<InputEventMouseButton> mb = p_event;
	if (mb.is_valid() && mb->is_pressed() && enabled && (mb->get_button_index() == MouseButton::LEFT || mb->get_button_index() == MouseButton::RIGHT || mb->get_button_index() == MouseButton::MIDDLE)) {
		const Point2 pos = mb->get_position();
		if (!floating && preview_pane->is_visible_in_tree() && preview_pane->get_global_rect().has_point(pos)) {
			_set_preview_active(true);
		} else if (focus_pane->get_global_rect().has_point(pos) || (SceneTreeDock::get_singleton()->is_visible_in_tree() && SceneTreeDock::get_singleton()->get_global_rect().has_point(pos))) {
			// The other docks, the Inspector among them, work on either side.
			_set_preview_active(false);
		}
	}
}

void EditorSplitView::_float_window_input(const Ref<InputEvent> &p_event) {
	const Ref<InputEventMouseButton> mb = p_event;
	if (mb.is_valid() && mb->is_pressed() && (mb->get_button_index() == MouseButton::LEFT || mb->get_button_index() == MouseButton::RIGHT || mb->get_button_index() == MouseButton::MIDDLE)) {
		_set_preview_active(true);
	}
}

void EditorSplitView::_trade_sides() {
	// Edit what the other pane shows. The panes stay; switching makes the
	// scene (or script) left behind the one the other pane shows.
	if (showing_code) {
		TextEditorBase *other = ObjectDB::get_instance<TextEditorBase>(code_editor_id);
		if (other && other->get_instance_id() != current_code_editor_id) {
			ScriptEditor::get_singleton()->edit(other->get_edited_resource());
			_track_current_script();
			_queue_update();
		}
		return;
	}
	if (showing_live || showing_scene_code) {
		return;
	}
	const int idx = _find_scene_index(preview_scene);
	if (idx >= 0 && idx != EditorNode::get_editor_data().get_edited_scene()) {
		// What's selected here stays selected once it's edited there.
		Node *root = get_preview_scene();
		Node *selected = preview_pane->tools->get_selected();
		if (root && selected) {
			pending_select = root->get_path_to(selected);
			pending_select_scene = preview_scene;
		}
		_set_preview_active(false);
		EditorSceneTabs::get_singleton()->set_current_tab(idx);
	}
}

void EditorSplitView::_preview_gui_input(const Ref<InputEvent> &p_event) {
	Node *root = get_preview_scene();
	const bool is_3d = Object::cast_to<Node3D>(root);
	const Ref<InputEventMouseButton> mb = p_event;
	if (mb.is_valid() && mb->is_pressed() && (mb->get_button_index() == MouseButton::LEFT || mb->get_button_index() == MouseButton::RIGHT || mb->get_button_index() == MouseButton::MIDDLE)) {
		_set_preview_active(true);
		preview_pane->viewport_container->grab_focus();
	}
	if (is_3d && _view_menu_shortcut(p_event)) {
		preview_pane->viewport_container->accept_event();
		return;
	}
	if (is_3d && !preview_cinematic && !preview_pane->tools->is_dragging()) {
		// Freelook, as in the 3D editor: hold the right mouse button to look
		// around, with WASD, Q and E to fly and the wheel for speed.
		if (mb.is_valid() && mb->get_button_index() == MouseButton::RIGHT) {
			if (mb->is_pressed() && !preview_freelook) {
				_set_freelook(true, mb->get_global_position());
			} else if (!mb->is_pressed() && preview_freelook) {
				_set_freelook(false);
			}
			preview_pane->viewport_container->accept_event();
			return;
		}
		if (preview_freelook) {
			const Ref<InputEventMouseMotion> motion = p_event;
			if (motion.is_valid()) {
				_freelook_look(motion->get_relative());
			} else if (mb.is_valid() && mb->is_pressed() && (mb->get_button_index() == MouseButton::WHEEL_UP || mb->get_button_index() == MouseButton::WHEEL_DOWN)) {
				// The wheel sets the speed while flying.
				const real_t factor = mb->get_button_index() == MouseButton::WHEEL_UP ? 1.08 : 1.0 / 1.08;
				freelook_speed = CLAMP(freelook_speed * factor, (real_t)0.01, (real_t)10000.0);
				zoom_indicator_delay = 1.5;
				preview_pane->overlay->queue_redraw();
			} else if (Ref<InputEventKey>(p_event).is_valid() && Ref<InputEventKey>(p_event)->is_action_pressed(SNAME("ui_cancel"), false, true)) {
				_set_freelook(false);
			}
			// Keys fly instead of picking tools.
			preview_pane->viewport_container->accept_event();
			return;
		}
	}
	if (is_3d && preview_cinematic) {
		// Looking through the scene's camera: nothing to move or pick.
		const Ref<InputEventMouseMotion> motion = p_event;
		if (mb.is_valid() || (motion.is_valid() && !motion->get_button_mask().is_empty())) {
			preview_pane->viewport_container->accept_event();
		}
		return;
	}
	if (preview_pane->tools->handle_input(p_event)) {
		preview_pane->viewport_container->accept_event();
		return;
	}
	if (mb.is_valid() && mb->is_pressed()) {
		switch (mb->get_button_index()) {
			case MouseButton::WHEEL_UP:
			case MouseButton::WHEEL_DOWN: {
				// Zoom, towards the mouse in 2D.
				_begin_custom_view();
				const real_t factor = mb->get_button_index() == MouseButton::WHEEL_UP ? 1.15 : 1.0 / 1.15;
				if (is_3d) {
					preview_orbit.distance = CLAMP(preview_orbit.distance / factor, 0.05, 10000.0);
					// Show how far it is, for a moment.
					zoom_indicator_delay = 1.5;
					_update_view_menu();
				} else {
					const Vector2 at = mb->get_position();
					preview_view_2d = Transform2D(0, at) * Transform2D().scaled(Size2(factor, factor)) * Transform2D(0, -at) * preview_view_2d;
				}
				_update_preview_camera();
			} break;
			case MouseButton::MIDDLE: {
				if (mb->is_double_click()) {
					// Back to the scene's own view.
					preview_view_custom = false;
					_update_preview_camera();
					_update_view_menu();
				}
			} break;
			default:
				break;
		}
		preview_pane->viewport_container->accept_event();
		return;
	}
	const Ref<InputEventMouseMotion> mm = p_event;
	if (mm.is_valid()) {
		if (root && mm->get_button_mask().has_flag(MouseButtonMask::MIDDLE)) {
			_begin_custom_view();
			const Vector2 rel = mm->get_relative();
			if (!is_3d) {
				preview_view_2d.columns[2] += rel;
			} else if (mm->is_shift_pressed()) {
				// Pan.
				const Basis basis = preview_pane->camera_3d->get_global_transform().basis;
				const real_t speed = preview_orbit.distance * 0.002;
				preview_orbit.target += (-basis.get_column(0) * rel.x + basis.get_column(1) * rel.y) * speed;
			} else {
				_orbit_preview(rel);
			}
			_update_preview_camera();
			preview_pane->viewport_container->accept_event();
		}
		preview_pane->overlay->queue_redraw();
	}
}

void EditorSplitView::_preview_tree_selected() {
	Node *selected = preview_pane->tree->get_selected();
	if (showing_live) {
		// It's the edited scene: select the node as the Scene dock would.
		if (selected) {
			EditorSelection *selection = EditorNode::get_singleton()->get_editor_selection();
			selection->clear();
			selection->add_node(selected);
			EditorNode::get_singleton()->push_node_item(selected);
		}
		return;
	}
	// Selects it in this pane; its scene stays where it is.
	_set_preview_active(true);
	if (selected != preview_pane->tools->get_selected()) {
		preview_pane->tools->set_selected(selected, false);
		_pane_selection_changed();
	}
}

void EditorSplitView::_focus_tree_selected() {
	Node *selected = focus_pane->tree->get_selected();
	if (selected) {
		last_focus_selected = selected->get_instance_id();
		EditorNode::get_singleton()->push_node_item(selected);
	}
}

void EditorSplitView::_close_preview() {
	_show_float_window(false);
	EditorSettings::get_singleton()->set("interface/editor/split_view/enabled", false);
	EditorSettings::get_singleton()->notify_changes();
	set_split_enabled(false);
}

void EditorSplitView::_inspected_object_changed() {
	if (pane_in_main_inspector) {
		return; // Showing the second pane's selection.
	}
	Object *edited = InspectorDock::get_inspector_singleton()->get_edited_object();
	if (Node *node = Object::cast_to<Node>(edited)) {
		last_focus_selected = node->get_instance_id();
	}
	if (focus_pane->inspector_strip->is_visible_in_tree()) {
		focus_pane->inspector->edit(edited);
		Node *node = Object::cast_to<Node>(edited);
		focus_pane->inspector_title->set_text(node ? vformat(TTR("Inspector: %s"), node->get_name()) : TTR("Inspector"));
	}
}

bool EditorSplitView::_uses_scene(const Node *p_node, const String &p_path) {
	// An instance of the scene, or a scene inheriting it, anywhere below.
	if (p_node->get_scene_file_path() == p_path) {
		return true;
	}
	for (Ref<SceneState> state = p_node->get_scene_inherited_state(); state.is_valid(); state = state->get_base_scene_state()) {
		if (state->get_path() == p_path) {
			return true;
		}
	}
	for (int i = 0; i < p_node->get_child_count(); i++) {
		if (_uses_scene(p_node->get_child(i), p_path)) {
			return true;
		}
	}
	return false;
}

void EditorSplitView::_scene_saved(const String &p_path) {
	// After the save is done: the "(*)" in the headers, and the second pane's
	// scene if it uses the saved scene.
	_queue_update();
	if (saved_scenes.is_empty()) {
		// On the next frame: rebuilding shows a progress dialog, which can't
		// be done from a deferred call.
		get_tree()->connect("process_frame", callable_mp(this, &EditorSplitView::_refresh_preview_instances), CONNECT_ONE_SHOT);
	}
	if (!saved_scenes.has(p_path)) {
		saved_scenes.push_back(p_path);
	}
}

bool EditorSplitView::_uses_any_scene(int p_idx, const Vector<String> &p_paths) {
	EditorData &data = EditorNode::get_editor_data();
	const Node *root = data.get_edited_scene_root(p_idx);
	if (!root) {
		return false;
	}
	for (const String &path : p_paths) {
		if (data.get_scene_path(p_idx) == path) {
			continue;
		}
		for (int i = 0; i < root->get_child_count(); i++) {
			if (_uses_scene(root->get_child(i), path)) {
				return true;
			}
		}
		for (Ref<SceneState> state = root->get_scene_inherited_state(); state.is_valid(); state = state->get_base_scene_state()) {
			if (state->get_path() == path) {
				return true;
			}
		}
	}
	return false;
}

void EditorSplitView::_refresh_preview_instances() {
	const Vector<String> saved = saved_scenes;
	saved_scenes.clear();

	// The scene being edited, when the saved scene was the one in the second
	// pane: the editor only rebuilds it when switching to its tab, so it would
	// keep showing the old version beside the new one.
	const int current = EditorNode::get_editor_data().get_edited_scene();
	if (current >= 0 && _uses_any_scene(current, saved)) {
		EditorNode::get_singleton()->update_edited_scene_instances();
	}

	Node *root = get_preview_scene();
	if (!root) {
		return;
	}
	const int idx = _find_scene_index(preview_scene);
	if (idx < 0 || !_uses_any_scene(idx, saved)) {
		return;
	}
	// The editor rebuilds such scenes when switching to them; do it now, as
	// this one is on screen. Keep the view looked around in.
	const bool keep_view = preview_view_custom;
	const OrbitView orbit = preview_orbit;
	const Transform2D view_2d = preview_view_2d;
	_release_preview();
	if (!EditorNode::get_editor_data().reload_scene_from_memory(idx, false)) {
		_queue_update();
		return;
	}
	Node *new_root = EditorNode::get_editor_data().get_edited_scene_root(idx);
	if (new_root && !new_root->get_parent()) {
		_take_into_preview(new_root);
		preview_view_custom = keep_view;
		preview_orbit = orbit;
		preview_view_2d = view_2d;
		_update_preview_camera();
	}
	_queue_update();
}

void EditorSplitView::_update_pane_header(EditorScenePane *p_pane, Node *p_root) {
	if (!p_root) {
		p_pane->set_title(String());
		p_pane->type_label->set_text(String());
		return;
	}
	const int idx = _find_scene_index(p_root->get_instance_id());
	const String path = idx >= 0 ? EditorNode::get_editor_data().get_scene_path(idx) : String();
	String title = path.is_empty() ? TTR("[unsaved]") : path.get_file();
	// Not EditorData::is_scene_changed(), which resets the change it reports.
	const bool unsaved = idx >= 0 && EditorUndoRedoManager::get_singleton()->is_history_unsaved(EditorNode::get_editor_data().get_scene_history_id(idx));
	if (unsaved) {
		title += "(*)";
	}
	if (p_pane == preview_pane && unsaved != preview_unsaved) {
		// Edited in the pane: the scene tabs only follow the edited scene.
		preview_unsaved = unsaved;
		EditorSceneTabs::get_singleton()->update_scene_tabs();
	}
	p_pane->set_title(title);
	p_pane->type_label->set_text(p_root->get_class());
	if (p_pane == preview_pane && floating) {
		float_window->set_title(vformat(TTR("%s - Split View"), title));
	}
	p_pane->dot->queue_redraw();
}

static bool _split_view_dock_open(Control *p_dock) {
	// A dock is open while it's in a slot that's shown, whether or not its
	// tab is the current one; a closed dock is out of the tree.
	if (!p_dock->is_inside_tree()) {
		return false;
	}
	const Control *parent = Object::cast_to<Control>(p_dock->get_parent());
	return parent ? parent->is_visible_in_tree() : p_dock->is_visible_in_tree();
}

void EditorSplitView::_update_strips() {
	const bool split = floating ? float_window->is_visible() : preview_pane->is_visible();
	// The focused scene already has the Scene and Inspector docks; only give
	// its pane its own strips when those docks are closed.
	// Open counts even behind another tab of their slot, such as the
	// Inspector while Signals or Groups is shown.
	const bool docks_open = _split_view_dock_open(SceneTreeDock::get_singleton()) && _split_view_dock_open(InspectorDock::get_singleton());
	// Beside the script editor, the focused pane is the script editor.
	const bool focus_strips = split && !showing_live && show_pane_docks && !docks_open;
	focus_pane->tree_strip->set_visible(focus_strips);
	focus_pane->inspector_strip->set_visible(focus_strips && focus_pane->get_size().x >= 900 * EDSCALE);
	if (!focus_pane->inspector_strip->is_visible()) {
		// It only follows the main inspector while shown; don't keep a
		// pointer to an object that may be freed meanwhile.
		focus_pane->inspector->edit(nullptr);
	} else if (focus_pane->inspector->get_edited_object() != InspectorDock::get_inspector_singleton()->get_edited_object()) {
		_inspected_object_changed();
	}
	// Keep the preview's viewport usable on narrow panes.
	const real_t preview_width = preview_pane->get_size().x;
	const bool pane_code = showing_code || showing_scene_code;
	preview_pane->tree_strip->set_visible(!pane_code && show_pane_docks && preview_width >= 520 * EDSCALE);
	preview_pane->inspector_strip->set_visible(!showing_live && !pane_code && show_pane_docks && preview_width >= 760 * EDSCALE);
}

void EditorSplitView::_queue_update() {
	if (update_queued) {
		return;
	}
	update_queued = true;
	callable_mp(this, &EditorSplitView::_update).call_deferred();
}

void EditorSplitView::_update() {
	update_queued = false;
	Node *preview = get_preview_scene();
	if (!preview && preview_scene.is_valid()) {
		_release_preview();
	}
	if (!preview && enabled) {
		// Scenes opened at startup or from the file system don't go through a
		// scene switch, so there may be nothing in the preview yet.
		_pick_default_preview();
		preview = get_preview_scene();
	}
	Node *edited = EditorNode::get_singleton()->get_edited_scene();
	// Beside the script editor, show the scene being edited, live.
	const bool was_live = showing_live;
	const bool script_screen = enabled && _is_script_screen();
	showing_code = script_screen && compare_scripts;
	showing_live = script_screen && (showing_code || edited);
	if (showing_live != was_live && !floating && preview_pane->get_parent() == this) {
		if (was_live) {
			script_split_offset = get_split_offset();
			has_script_split_offset = true;
			set_split_offset(scene_split_offset);
		} else {
			scene_split_offset = get_split_offset();
			if (!has_script_split_offset) {
				// Give the code about two thirds of the width.
				const int sixth = int(get_size().x / 6);
				script_split_offset = preview_pane->get_index() > focus_pane->get_index() ? sixth : -sixth;
			}
			set_split_offset(script_split_offset);
		}
	}
	const bool scene_screen = enabled && _is_scene_screen();
	showing_scene_code = scene_screen && code_beside_scene;
	const bool split = showing_live || showing_scene_code || (scene_screen && preview);

	preview_pane->viewport_container->set_visible(!showing_live && !showing_scene_code);
	preview_pane->tools->set_visible(!showing_live && !showing_scene_code);
	preview_pane->live_container->set_visible(showing_live && !showing_code);
	preview_pane->code_container->set_visible(showing_code || showing_scene_code);
	// Scene or code, on every screen; each screen keeps its own choice.
	preview_pane->code_button->set_visible(script_screen || scene_screen);
	preview_pane->code_button->set_pressed_no_signal(script_screen ? compare_scripts : code_beside_scene);
	// Trading sides needs something to edit on the other side.
	preview_pane->swap_button->set_visible(showing_code || (scene_screen && !showing_scene_code));
	preview_pane->code_button->set_tooltip_text(script_screen ? TTR("Show another open script here instead of the scene. The script you switch away from moves here; click it to edit it.") : TTR("Show the selected node's script here instead of another scene. Click it to edit it."));
	if (!showing_code && !showing_scene_code) {
		_set_code_source(nullptr);
		code_static_script.unref();
	}
	_set_pane_tree_root(showing_live ? (showing_code ? nullptr : edited) : (showing_scene_code ? nullptr : preview));

	if (floating) {
		preview_pane->show();
		_show_float_window(split);
	} else {
		preview_pane->set_visible(split);
	}
	focus_pane->header->set_visible(split && !showing_live);
	_update_active_pane();
	_update_strips();
	_update_view_menu();
	_update_pane_toolbar();

	if (!split) {
		focus_pane->inspector->edit(nullptr);
		return;
	}

	if (showing_code) {
		_update_code_view();
		return;
	}
	if (showing_scene_code) {
		_update_pane_header(focus_pane, edited);
		focus_pane->tree->update_tree();
		_inspected_object_changed();
		_update_scene_code_view();
		return;
	}
	_update_pane_header(preview_pane, showing_live ? edited : preview);
	preview_pane->tree->update_tree();
	if (showing_live) {
		preview_pane->tree->get_scene_tree()->deselect_all();
		_update_live_camera();
		return;
	}
	_update_pane_header(focus_pane, edited);
	focus_pane->tree->update_tree();
	_inspected_object_changed();
	_update_preview_camera();
}

Rect2i EditorSplitView::_default_float_rect() const {
	// Where the pane was, or the right half of the editor's screen.
	if (preview_pane->is_visible_in_tree() && preview_pane->get_size().x > 0) {
		return Rect2i(preview_pane->get_screen_position(), preview_pane->get_size());
	}
	const Window *editor_window = EditorNode::get_singleton()->get_window();
	const int screen = editor_window ? editor_window->get_current_screen() : DisplayServer::get_singleton()->get_primary_screen();
	const Rect2i usable = DisplayServer::get_singleton()->screen_get_usable_rect(screen);
	const Size2i size = Size2i(usable.size.x / 2, usable.size.y * 3 / 4);
	return Rect2i(usable.position + Point2i(usable.size.x - size.x, (usable.size.y - size.y) / 2), size);
}

void EditorSplitView::_show_float_window(bool p_show) {
	if (!float_window || p_show == float_window->is_visible()) {
		return;
	}
	if (!p_show) {
		float_rect = Rect2i(float_window->get_position(), float_window->get_size());
		float_window->hide();
		return;
	}
	const Rect2i rect = float_rect.has_area() ? float_rect : _default_float_rect();
	float_window->set_position(rect.position);
	float_window->set_size(rect.size);
	float_window->show();
	// Applied while hidden; apply again now that the native window exists,
	// or it can stay blank until it's resized.
	float_window->set_size(rect.size);
}

void EditorSplitView::_set_floating(bool p_floating) {
	if (p_floating && !EditorNode::get_singleton()->is_multi_window_enabled()) {
		p_floating = false;
	}
	preview_pane->float_button->set_pressed_no_signal(p_floating);
	if (floating == p_floating) {
		return;
	}
	if (p_floating) {
		if (!float_rect.has_area()) {
			float_rect = _default_float_rect();
		}
		remove_child(preview_pane);
		float_window->add_child(preview_pane);
		preview_pane->set_anchors_and_offsets_preset(PRESET_FULL_RECT);
		preview_pane->show();
		floating = true;
	} else {
		_show_float_window(false);
		float_window->remove_child(preview_pane);
		preview_pane->set_anchors_and_offsets_preset(PRESET_TOP_LEFT);
		add_child(preview_pane);
		// The second pane is always on the right, away from the Scene dock.
		move_child(preview_pane, focus_pane->get_index() + 1);
		floating = false;
	}
	_queue_update();
}

void EditorSplitView::_apply_float_setting() {
	preview_pane->float_button->set_visible(EditorNode::get_singleton()->is_multi_window_enabled());
	_set_floating(EDITOR_GET("interface/editor/split_view/floating"));
}

void EditorSplitView::_float_toggled(bool p_pressed) {
	_set_floating(p_pressed);
	EditorSettings::get_singleton()->set("interface/editor/split_view/floating", floating);
	EditorSettings::get_singleton()->notify_changes();
}

void EditorSplitView::_float_window_close_requested() {
	_float_toggled(false);
}

void EditorSplitView::set_main_screen(Control *p_main_screen) {
	ERR_FAIL_COND(main_screen != nullptr);
	main_screen = p_main_screen;
	focus_pane->content->add_child(main_screen);
	main_screen->set_v_size_flags(SIZE_EXPAND_FILL);
	main_screen->connect("tab_changed", callable_mp(this, &EditorSplitView::_queue_update).unbind(1));
}

void EditorSplitView::set_split_enabled(bool p_enabled) {
	if (enabled == p_enabled) {
		return;
	}
	enabled = p_enabled;
	if (!enabled) {
		_release_preview();
	} else {
		_pick_default_preview();
	}
	_queue_update();
}

void EditorSplitView::_pick_default_preview() {
	if (!enabled || get_preview_scene()) {
		return;
	}
	// The tab next to the current one: the previous tab if there is one.
	EditorData &ed = EditorNode::get_editor_data();
	const int current = ed.get_edited_scene();
	const int other = current > 0 ? current - 1 : (ed.get_edited_scene_count() > 1 ? 1 : -1);
	Node *root = other >= 0 && other != current ? ed.get_edited_scene_root(other) : nullptr;
	// Only take a scene nothing else holds (it is out of the tree while it
	// isn't the edited one).
	if (root && !root->get_parent()) {
		_take_into_preview(root);
	}
}

void EditorSplitView::set_show_pane_docks(bool p_show) {
	show_pane_docks = p_show;
	_queue_update();
}

void EditorSplitView::edited_scene_changed(Node *p_old_root, Node *p_new_root) {
	if (!enabled) {
		return;
	}
	// Show the new scene's script, not one from the scene left behind.
	scene_code_script.unref();
	Node *preview = ObjectDB::get_instance<Node>(preview_scene);
	if (preview && preview == p_new_root) {
		// The previewed scene is being edited now; EditorNode moved it.
		_set_pane_tree_root(nullptr);
		preview_pane->tools->set_scene(nullptr);
		preview_pane->inspector->edit(nullptr);
		preview_pane->viewport->set_editor_preview_scene_root(nullptr);
		preview_scene = ObjectID();
		preview = nullptr;
	}

	// A scene being closed is about to be freed; don't bring it into the tree.
	const bool old_closing = p_old_root && p_old_root->get_instance_id() == closing_scene;
	if (p_old_root && !old_closing && p_old_root != p_new_root && _find_scene_index(p_old_root->get_instance_id()) >= 0) {
		if (preview) {
			_release_preview();
		}
		_take_into_preview(p_old_root);
	}

	if (pending_select_scene.is_valid() && p_new_root && p_new_root->get_instance_id() == pending_select_scene) {
		// EditorNode restores the scene's saved selection after this, so
		// select once the switch is done.
		callable_mp(this, &EditorSplitView::_apply_pending_select).call_deferred();
	} else {
		pending_select = NodePath();
		pending_select_scene = ObjectID();
	}
	_queue_update();
}

void EditorSplitView::_apply_pending_select() {
	Node *root = EditorNode::get_singleton()->get_edited_scene();
	if (root && root->get_instance_id() == pending_select_scene) {
		Node *node = root->get_node_or_null(pending_select);
		if (node) {
			EditorSelection *selection = EditorNode::get_singleton()->get_editor_selection();
			selection->clear();
			selection->add_node(node);
			EditorNode::get_singleton()->push_node_item(node);
		}
	}
	pending_select = NodePath();
	pending_select_scene = ObjectID();
}

void EditorSplitView::release_scene(Node *p_root) {
	if (p_root && p_root->get_instance_id() == closing_scene) {
		closing_scene = ObjectID();
	}
	if (p_root && p_root->get_instance_id() == preview_scene) {
		_release_preview();
		_queue_update();
	}
}

void EditorSplitView::set_closing_scene(Node *p_root) {
	closing_scene = p_root ? p_root->get_instance_id() : ObjectID();
}

void EditorSplitView::queue_update() {
	_queue_update();
}

void EditorSplitView::show_scene_in_preview(int p_idx) {
	EditorData &ed = EditorNode::get_editor_data();
	ERR_FAIL_INDEX(p_idx, ed.get_edited_scene_count());
	if (p_idx == ed.get_edited_scene()) {
		return;
	}
	Node *root = ed.get_edited_scene_root(p_idx);
	ERR_FAIL_NULL(root);
	if (!enabled) {
		EditorSettings::get_singleton()->set("interface/editor/split_view/enabled", true);
		EditorSettings::get_singleton()->notify_changes();
		enabled = true;
	}
	if (root->get_instance_id() != preview_scene) {
		_release_preview();
		_take_into_preview(root);
	}
	_queue_update();
}

void EditorSplitView::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
			EditorNode::get_singleton()->connect("scene_changed", callable_mp(this, &EditorSplitView::_queue_update));
			EditorNode::get_singleton()->connect("scene_saved", callable_mp(this, &EditorSplitView::_scene_saved));
			// Keeps the "(*)" unsaved marks in the pane headers current.
			EditorUndoRedoManager::get_singleton()->connect("history_changed", callable_mp(this, &EditorSplitView::_queue_update));
			EditorUndoRedoManager::get_singleton()->connect("version_changed", callable_mp(this, &EditorSplitView::_queue_update));
			InspectorDock::get_inspector_singleton()->connect("edited_object_changed", callable_mp(this, &EditorSplitView::_inspected_object_changed));
			focus_pane->tree->set_editor_selection(EditorNode::get_singleton()->get_editor_selection());
			SceneTreeDock::get_singleton()->connect(SceneStringName(visibility_changed), callable_mp(this, &EditorSplitView::_queue_update));
			InspectorDock::get_singleton()->connect(SceneStringName(visibility_changed), callable_mp(this, &EditorSplitView::_queue_update));
			set_show_pane_docks(EDITOR_GET("interface/editor/split_view/show_pane_docks"));
			compare_scripts = EDITOR_GET("interface/editor/split_view/compare_scripts");
			code_beside_scene = EDITOR_GET("interface/editor/split_view/code_beside_scene");
			ScriptEditor::get_singleton()->connect("editor_script_changed", callable_mp(this, &EditorSplitView::_script_changed));
			EditorNode::get_singleton()->get_editor_selection()->connect("selection_changed", callable_mp(this, &EditorSplitView::_selection_changed));
			set_split_enabled(EDITOR_GET("interface/editor/split_view/enabled"));
			set_process_input(true);
			_build_view_menu();
			_build_preview_grid();
			// The Scene dock's tree settings.
			preview_pane->tree->set_auto_expand_selected(EDITOR_GET("docks/scene_tree/auto_expand_to_selected"), false);
			preview_pane->tree->set_hide_filtered_out_parents(EDITOR_GET("docks/scene_tree/hide_filtered_out_parents"), false);
			preview_pane->tree->set_accessibility_warnings(EDITOR_GET("docks/scene_tree/accessibility_warnings"), false);
			// Whether the editor uses native windows is only settled once
			// startup is done.
			callable_mp(this, &EditorSplitView::_apply_float_setting).call_deferred();
		} break;

		case EditorSettings::NOTIFICATION_EDITOR_SETTINGS_CHANGED: {
			if (EditorSettings::get_singleton()->check_changed_settings_in_group("editors/3d/navigation")) {
				_update_view_menu(); // Showing the axis gizmo or not.
			}
			if (EditorSettings::get_singleton()->check_changed_settings_in_group("interface/editor/split_view")) {
				set_show_pane_docks(EDITOR_GET("interface/editor/split_view/show_pane_docks"));
				compare_scripts = EDITOR_GET("interface/editor/split_view/compare_scripts");
				code_beside_scene = EDITOR_GET("interface/editor/split_view/code_beside_scene");
				set_split_enabled(EDITOR_GET("interface/editor/split_view/enabled"));
				// Can't move the pane while this notification goes through the tree.
				callable_mp(this, &EditorSplitView::_apply_float_setting).call_deferred();
			}
		} break;

		case NOTIFICATION_PREDELETE: {
			// Open scenes are owned by EditorData; children are freed right
			// after this, so never let the viewport free one with the pane.
			Node *root = ObjectDB::get_instance<Node>(preview_scene);
			if (root && root->get_parent() == preview_pane->viewport) {
				preview_pane->viewport->remove_child(root);
			}
			preview_scene = ObjectID();
		} break;

		case NOTIFICATION_PROCESS: {
			if (preview_freelook) {
				_freelook_move(get_process_delta_time());
			}
			if (zoom_indicator_delay > 0.0) {
				zoom_indicator_delay -= get_process_delta_time();
				if (zoom_indicator_delay <= 0.0) {
					zoom_indicator_delay = 0.0;
					preview_pane->overlay->queue_redraw();
					_update_view_menu();
				}
			}
			_update_view_panels();
		} break;

		case NOTIFICATION_RESIZED: {
			if (preview_pane && preview_pane->is_visible()) {
				callable_mp(this, &EditorSplitView::_update_strips).call_deferred();
				callable_mp(this, &EditorSplitView::_update_preview_camera).call_deferred();
			}
		} break;
	}
}

EditorSplitView::EditorSplitView() {
	singleton = this;
	set_v_size_flags(SIZE_EXPAND_FILL);
	set_h_size_flags(SIZE_EXPAND_FILL);

	focus_pane = memnew(EditorScenePane(true));
	add_child(focus_pane);
	focus_pane->tree->connect("node_selected", callable_mp(this, &EditorSplitView::_focus_tree_selected));

	preview_pane = memnew(EditorScenePane(false));
	add_child(preview_pane);
	preview_pane->tree->connect("node_selected", callable_mp(this, &EditorSplitView::_preview_tree_selected));
	// As the Scene dock: open an instance's scene, or a node's script.
	preview_pane->tree->connect("open", callable_mp(this, &EditorSplitView::_pane_open_scene));
	preview_pane->tree->connect("open_script", callable_mp(this, &EditorSplitView::_pane_open_script));
	preview_pane->tools->connect("selection_changed", callable_mp(this, &EditorSplitView::_pane_selection_changed));
	preview_pane->rotation_control->connect(SceneStringName(pressed), callable_mp(this, &EditorSplitView::_set_preview_active).bind(true));
	preview_pane->rotation_control->connect("axis_clicked", callable_mp(this, &EditorSplitView::_rotation_axis_clicked));
	preview_pane->rotation_control->connect("orbit", callable_mp(this, &EditorSplitView::_orbit_preview));
	preview_pane->add_node_button->connect(SceneStringName(pressed), callable_mp(this, &EditorSplitView::_pane_add_node));
	preview_pane->instantiate_button->connect(SceneStringName(pressed), callable_mp(this, &EditorSplitView::_pane_instantiate_scene));
	preview_pane->tree_filter->connect(SceneStringName(text_changed), callable_mp(this, &EditorSplitView::_pane_filter_changed));
	preview_pane->attach_script_button->connect(SceneStringName(pressed), callable_mp(this, &EditorSplitView::_pane_attach_script));
	preview_pane->detach_script_button->connect(SceneStringName(pressed), callable_mp(this, &EditorSplitView::_pane_detach_script));
	preview_pane->tree_menu->connect("about_to_popup", callable_mp(this, &EditorSplitView::_pane_tree_menu_about_to_popup));
	preview_pane->tree_menu->get_popup()->connect(SceneStringName(id_pressed), callable_mp(this, &EditorSplitView::_pane_tree_menu_option));
	preview_pane->subresource_menu->connect("about_to_popup", callable_mp(this, &EditorSplitView::_pane_list_subresources));
	preview_pane->subresource_menu->connect("index_pressed", callable_mp(this, &EditorSplitView::_pane_edit_subresource));
	preview_pane->tree->set_scene_root_override(nullptr);
	preview_pane->viewport_container->connect(SceneStringName(gui_input), callable_mp(this, &EditorSplitView::_preview_gui_input));
	preview_pane->viewport_container->connect(SceneStringName(mouse_exited), callable_mp((CanvasItem *)preview_pane->overlay, &CanvasItem::queue_redraw));
	preview_pane->viewport_container->connect(SceneStringName(resized), callable_mp(this, &EditorSplitView::_update_preview_camera));
	preview_pane->live_container->connect(SceneStringName(gui_input), callable_mp(this, &EditorSplitView::_live_gui_input));
	preview_pane->live_container->connect(SceneStringName(resized), callable_mp(this, &EditorSplitView::_update_live_camera));
	preview_pane->overlay->connect(SceneStringName(draw), callable_mp(this, &EditorSplitView::_draw_preview_overlay));
	preview_pane->close_button->connect(SceneStringName(pressed), callable_mp(this, &EditorSplitView::_close_preview));
	preview_pane->float_button->connect(SceneStringName(toggled), callable_mp(this, &EditorSplitView::_float_toggled));
	preview_pane->swap_button->connect(SceneStringName(pressed), callable_mp(this, &EditorSplitView::_trade_sides));
	preview_pane->code_button->connect(SceneStringName(toggled), callable_mp(this, &EditorSplitView::_code_toggled));
	preview_pane->pick_button->connect("about_to_popup", callable_mp(this, &EditorSplitView::_fill_pick_menu));
	preview_pane->pick_button->get_popup()->connect(SceneStringName(id_pressed), callable_mp(this, &EditorSplitView::_pick_menu_id_pressed));
	preview_pane->code_view->connect(SceneStringName(gui_input), callable_mp(this, &EditorSplitView::_code_gui_input));
	// Drop scene tabs, scenes, scripts and script list entries on the pane.
	const Callable can_drop = callable_mp(this, &EditorSplitView::_can_drop);
	const Callable drop = callable_mp(this, &EditorSplitView::_drop);
	Control *drop_targets[] = { preview_pane, preview_pane->header, preview_pane->viewport_container, preview_pane->live_container, preview_pane->code_view, preview_pane->code_container };
	for (Control *target : drop_targets) {
		target->set_drag_forwarding(Callable(), can_drop, drop);
	}
	preview_pane->connect(SceneStringName(resized), callable_mp(this, &EditorSplitView::_update_strips), CONNECT_DEFERRED);
	preview_pane->hide();

	float_window = memnew(Window);
	float_window->set_visible(false);
	float_window->set_title(TTR("Split View"));
	float_window->set_wrap_controls(true);
	float_window->set_transient(true);
	float_window->set_propagate_shortcuts_to_parent(true);
	float_window->connect("close_requested", callable_mp(this, &EditorSplitView::_float_window_close_requested));
	float_window->connect("window_input", callable_mp(this, &EditorSplitView::_float_window_input));
	Panel *float_background = memnew(Panel);
	float_background->set_anchors_and_offsets_preset(PRESET_FULL_RECT);
	float_window->add_child(float_background);
	add_child(float_window);
}

EditorSplitView::~EditorSplitView() {
	singleton = nullptr;
}
