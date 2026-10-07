/**************************************************************************/
/*  tiled_editor_plugin.cpp                                               */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             BLAZIUM ENGINE                             */
/*                          https://blazium.app                           */
/**************************************************************************/
/* Copyright (c) 2024-present Blazium Engine contributors.                */
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

#include "tiled_editor_plugin.h"

#include "modules/tiled_importer/tiled_tilemap_creator.h"

#include "core/input/shortcut.h"
#include "editor/editor_node.h"
#include "editor/filesystem_dock.h"
#include "scene/gui/box_container.h"
#include "scene/gui/separator.h"
#include "scene/main/node.h"

static void _set_scene_owner(Node *p_node, Node *p_owner) {
	if (!p_node) {
		return;
	}
	p_node->set_owner(p_owner);
	for (int i = 0; i < p_node->get_child_count(); i++) {
		_set_scene_owner(p_node->get_child(i), p_owner);
	}
}

void TiledEditorPlugin::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_on_reimport_pressed"), &TiledEditorPlugin::_on_reimport_pressed);
}

void TiledEditorPlugin::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			main_panel = memnew(PanelContainer);
			main_panel->set_v_size_flags(Control::SIZE_EXPAND_FILL);

			VBoxContainer *vbox = memnew(VBoxContainer);
			main_panel->add_child(vbox);

			status_label = memnew(Label);
			status_label->set_text("Tiled Importer: Native Tiled Architecture Loaded");
			status_label->set_horizontal_alignment(HORIZONTAL_ALIGNMENT_CENTER);
			vbox->add_child(status_label);

			HSeparator *sep = memnew(HSeparator);
			vbox->add_child(sep);

			btn_reimport = memnew(Button);
			btn_reimport->set_text("Force Rebuild Selected Tiled Map");
			btn_reimport->connect("pressed", callable_mp(this, &TiledEditorPlugin::_on_reimport_pressed));
			vbox->add_child(btn_reimport);

			add_blazium_window("Tiled", "Tiled Maps", main_panel);
		} break;
		case NOTIFICATION_EXIT_TREE: {
			if (main_panel) {
				remove_blazium_item("Tiled", "Tiled Maps");
				memdelete(main_panel);
				main_panel = nullptr;
			}
		} break;
	}
}

void TiledEditorPlugin::_on_reimport_pressed() {
	String tmx_path;
	FileSystemDock *dock = FileSystemDock::get_singleton();
	if (dock) {
		const Vector<String> paths = dock->get_selected_paths();
		for (const String &path : paths) {
			if (path.get_extension().to_lower() == "tmx") {
				tmx_path = path;
				break;
			}
		}
	}
	if (tmx_path.is_empty()) {
		if (status_label) {
			status_label->set_text("Select a .tmx file in the FileSystem dock first.");
		}
		return;
	}

	TiledTilemapCreator creator;
	Node *map = creator.create_tilemap(tmx_path);
	if (!map) {
		if (status_label) {
			status_label->set_text(vformat("Failed to rebuild %s.", tmx_path));
		}
		return;
	}

	Node *edited = EditorNode::get_singleton() ? EditorNode::get_singleton()->get_edited_scene() : nullptr;
	if (edited) {
		edited->add_child(map);
		_set_scene_owner(map, edited);
	} else if (EditorNode::get_singleton()) {
		EditorNode::get_singleton()->set_edited_scene(map);
	}
	if (status_label) {
		status_label->set_text(vformat("Rebuilt %s.", tmx_path));
	}
}

void TiledEditorPlugin::make_visible(bool p_visible) {
	// The maps panel lives in its own window. Hiding it here blanks that window
	// when the editor changes main screens.
	(void)p_visible;
}

TiledEditorPlugin::TiledEditorPlugin() {
}

TiledEditorPlugin::~TiledEditorPlugin() {
}
