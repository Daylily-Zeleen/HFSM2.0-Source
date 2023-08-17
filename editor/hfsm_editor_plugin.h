/**************************************************************************/
/*  hfsm_editor_plugin.h                                                  */
/**************************************************************************/
/*                         This file is part of:                          */
/*                   Hierarchical Finite State Machine                    */
/*            https://github.com/Daylily-Zeleen/HFSM2.0-Source            */
/**************************************************************************/
/* Copyright (c) 2023-present Daylily Zeleen.                             */
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

#pragma once

#ifdef GDEXTENSION_BUILD
#include <godot_cpp/classes/editor_inspector_plugin.hpp>
#include <godot_cpp/classes/editor_plugin.hpp>
#include <godot_cpp/classes/editor_property.hpp>
#include <godot_cpp/classes/editor_resource_picker.hpp>
#include <godot_cpp/classes/editor_undo_redo_manager.hpp>
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/classes/translation_server.hpp>
#include <godot_cpp/templates/hash_map.hpp>

using namespace godot;
namespace godot {
class ImageTexture;
};

#else // GDEXTENSION_BUILD
#include <core/string/translation.h>
#include <editor/editor_inspector.h>
#include <editor/editor_plugin.h>
#include <editor/editor_undo_redo_manager.h>

#include <editor/editor_resource_picker.h>

class ImageTexture;
#endif // GDEXTENSION_BUILD

#include "../hfsm_global.h"

namespace Hfsm {

class EditorPropertyVariableConfig : public EditorProperty {
	GDCLASS(EditorPropertyVariableConfig, EditorProperty)
public:
	class VariableConfigSelector : public EditorResourcePicker {
#ifdef GDEXTENSION_BUILD
		GDCLASS(VariableConfigSelector, EditorResourcePicker)
#endif // GDEXTENSION_BUILD
	private:
		class HFSM *hfsm = nullptr;
		Button *edit_button = nullptr;
		PopupMenu *edit_menu = nullptr;

		Ref<class VariableConfig> to_compare;

		Vector<Ref<VariableConfig>> variable_config_list;

		void _resource_selected(const Ref<Resource> &p_res, bool p_inspect);
		void _menu_popup();

	protected:
		static void _bind_methods();

	public:
		int op_ofs = 10;

		void GD_(set_create_options)(Object *p_menu_node) override;
		bool GD_(handle_menu_selected)(int p_which) override;

		VariableConfigSelector() = default;
		VariableConfigSelector(HFSM *p_hfsm, const Ref<VariableConfig> &p_to_compare);
	};

private:
	VariableConfigSelector *selector = nullptr;
	class HFSM *hfsm = nullptr;

	Ref<class VariableConfig> to_compare;
	bool updating = false;

	void _variable_selected(const Ref<Resource> &p_res);

	void update_property_internal();

protected:
	static void _bind_methods();

public:
	EditorPropertyVariableConfig();
	EditorPropertyVariableConfig(HFSM *p_hfsm, const Ref<VariableConfig> &p_to_compare = nullptr);

	void GD_(update_property)() override { update_property_internal(); }
};

class HfsmInspectorPlugin : public EditorInspectorPlugin {
	GDCLASS(HfsmInspectorPlugin, EditorInspectorPlugin)

	bool can_handle_internal(Object *p_object) const;
	bool parse_property_internal(Object *p_object, Variant::Type p_type, const String &p_name, PropertyHint p_hint_type, const String &p_hint_string, BitField<PropertyUsageFlags> p_usage_flags, bool p_wide);

protected:
	static void _bind_methods() {}

public:
#ifdef GDEXTENSION_BUILD
	bool _can_handle(Object *p_object) const override { return can_handle_internal(p_object); }
	bool _parse_property(Object *p_object, Variant::Type p_type, const String &p_name, PropertyHint p_hint_type, const String &p_hint_string, BitField<PropertyUsageFlags> p_usage_flags, bool p_wide) override {
		return parse_property_internal(p_object, p_type, p_name, p_hint_type, p_hint_string, p_usage_flags, p_wide);
	}
#else // GDEXTENSION_BUILD
	bool can_handle(Object *p_object) override { return can_handle_internal(p_object); }
	bool parse_property(Object *p_object, Variant::Type p_type, const String &p_name, PropertyHint p_hint_type, const String &p_hint_string, BitField<PropertyUsageFlags> p_usage_flags, bool p_wide) override {
		return parse_property_internal(p_object, p_type, p_name, p_hint_type, p_hint_string, p_usage_flags, p_wide);
	}
#endif // GDEXTENSION_BUILD
};

class HfsmEditorPlugin : public EditorPlugin {
	GDCLASS(HfsmEditorPlugin, EditorPlugin)
private:
	static HfsmEditorPlugin *instance;

	Ref<HfsmInspectorPlugin> inspector_plugin;
	Ref<class HfsmDebuggerPlugin> debugger_plugin;
	class HFSMEditor *hfsm_editor = nullptr;
	class Button *hfsm_editor_btn = nullptr;

	HashMap<String, const char *> translation;

	bool handles_internal(Object *p_object) const;

	void _referenced_script_saved(const Ref<Resource> &p_res);
	void _change_scene(Node *scene_root);
	void _filesystem_changed();

	static PackedStringArray get_animation_list_for_state_config();

	static Ref<ImageTexture> empty_icon_for_state_node;
	static Ref<ImageTexture> get_empty_icon_for_state_node();

protected:
	static void _bind_methods();

	void _notification(int p_what);

public:
	static HfsmEditorPlugin *get_singleton() { return instance; }

	HfsmEditorPlugin();
	~HfsmEditorPlugin() override;

#ifdef GDEXTENSION_BUILD
	bool _handles(Object *p_object) const override { return handles_internal(p_object); }
#else
	bool handles(Object *p_object) const override { return handles_internal(p_object); }
#endif // GDEXTENSION_BUILD

	HFSMEditor *get_hfsm_editor() { return hfsm_editor; }

	static bool is_zh() { return TranslationServer::get_singleton()->get_tool_locale().substr(0, 2) == "zh"; }

	static String str_localize(const String &p_en_key) {
		if (is_zh() && get_singleton()->translation.has(p_en_key)) {
			return String::utf8(get_singleton()->translation[p_en_key]);
		}
		return p_en_key;
	}

	static EditorUndoRedoManager *create_action(const String &p_action_name);
};

} //namespace Hfsm

#define HFSM_EDITOR_CREATE_ACTION(m_action) auto undo_redo = HfsmEditorPlugin::create_action(m_action)
