#pragma once

#include <godot_cpp/classes/editor_inspector_plugin.hpp>
#include <godot_cpp/classes/editor_plugin.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/translation_server.hpp>

#include <godot_cpp/classes/editor_undo_redo_manager.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include <godot_cpp/templates/hash_map.hpp>

#include "hfsm_editor.hpp"

using namespace godot;
namespace Hfsm {

class HfsmInspectorPlugin : public EditorInspectorPlugin {
	GDCLASS(HfsmInspectorPlugin, EditorInspectorPlugin)
protected:
	static void _bind_methods() {}

public:
	bool _can_handle(Object *p_object) const override;
	bool _parse_property(Object *p_object, Variant::Type p_type, const String &p_name, PropertyHint p_hint_type, const String &p_hint_string, BitField<PropertyUsageFlags> p_usage_flags, bool p_wide) override;
};

class HfsmEditorPlugin : public EditorPlugin {
	GDCLASS(HfsmEditorPlugin, EditorPlugin)
protected:
	static void _bind_methods() {}

private:
	static HfsmEditorPlugin *instance;

	Ref<HfsmInspectorPlugin> inspector_plugin;
	HFSMEditor *hfsm_editor = nullptr;
	Button *hfsm_editor_btn = nullptr;

	HashMap<String, const char *> translation;

public:
	static HfsmEditorPlugin *get_singleton() { return instance; }

	HfsmEditorPlugin();
	~HfsmEditorPlugin() override;

	bool _handles(Object *p_object) const override;
	void _enter_tree() override;
	void _exit_tree() override;

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