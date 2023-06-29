#pragma once

#ifdef GDEXTENSION_BUILD
#include <godot_cpp/classes/editor_undo_redo_manager.h>
#include <godot_cpp/classes/editor_inspector_plugin.hpp>
#include <godot_cpp/classes/editor_plugin.hpp>
#include <godot_cpp/classes/translation_server.hpp>

using namespace godot;
#else
#include <core/string/translation.h>
#include <editor/editor_inspector.h>
#include <editor/editor_plugin.h>
#include <editor/editor_undo_redo_manager.h>

#endif // GDEXTENSION_BUILD

namespace Hfsm {

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
protected:
	static void _bind_methods();

	void _notification(int p_what);

private:
	static HfsmEditorPlugin *instance;

	Ref<HfsmInspectorPlugin> inspector_plugin;
	Ref<class HfsmDebuggerPlugin> debugger_plugin;
	class HFSMEditor *hfsm_editor = nullptr;
	class Button *hfsm_editor_btn = nullptr;

	HashMap<String, const char *> translation;

	bool handles_internal(Object *p_object) const;

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