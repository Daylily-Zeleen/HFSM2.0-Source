#include "hfsm_editor_plugin.hpp"

#include <godot_cpp/classes/editor_interface.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/templates/vector.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include <godot_cpp/classes/editor_plugin.hpp>

#include "godot_cpp/core/binder_common.hpp"
#include "godot_cpp/core/method_ptrcall.hpp"
#include "godot_cpp/variant/variant.hpp"
#include "inspector_plugin/variable_res_selector.hpp"
#include "src/core/hfsm.hpp"
#include "src/core/transitions/variable_expressions/variable_expression_res.hpp"

using namespace godot;

namespace Hfsm {

void HfsmInspectorPlugin::_bind_methods() {}
HfsmInspectorPlugin::HfsmInspectorPlugin() = default;

bool HfsmInspectorPlugin::_can_handle(const Variant &object) const {
    if (object.get_type() != Variant::OBJECT) {
        return false;
    }
    return Object::cast_to<VariableExpressionRes>(object) != nullptr;
}

bool HfsmInspectorPlugin::_parse_property(Object *object, Variant::Type type, const String &name, PropertyHint hint_type, const String &hint_string, BitField<PropertyUsageFlags> usage_flags,
                                          bool wide) {

    ERR_FAIL_COND_V(object == nullptr, false);
    if (auto ver = Object::cast_to<VariableExpressionRes>(object)) {
        if (auto hfsm = HfsmEditorPlugin::get_singleton()->get_hfsm_editor()->get_editing_hfsm()) {
            if (name == "variable_res") {
                auto editor = memnew(VariableResSelector);
                editor->__setup(hfsm);
                add_property_editor(name, editor);
                return true;

            } else if (name == "value") {
                if (ver->is_variable_as_value() && ver->get_variable_res().is_valid()) {
                    auto editor = memnew(VariableResSelector);
                    editor->__setup(hfsm, ver->get_variable_res());
                    add_property_editor(name, editor);
                    return true;
                }
            }
        }
    }
    return false;
}

#ifndef HACK
void HfsmEditorPlugin::_bind_methods() {}

String HfsmEditorPlugin::_to_string() const { return String("[HfsmEditorPlugin:{0}]").replace("{0}", itos(get_instance_id())); }

HfsmEditorPlugin::HfsmEditorPlugin() {
    if (instance)
        instance->queue_free();
    inspector_plugin.instantiate();
    instance = this;
}

bool HfsmEditorPlugin::_handles(const Variant &object) const {
    auto obj = Object::cast_to<Node>(object);
    if (obj) {
        auto node = Object::cast_to<HFSM>(obj);
        hfsm_editor->edit(node);
        return true; // ????
    }
    return false;
}

void HfsmEditorPlugin::_enter_tree() {
    if (!Engine::get_singleton()->is_editor_hint()) {
        queue_free();
        return;
    }

    instance->hfsm_editor = HFSMEditor::create_hfsm_editor();
    instance->hfsm_editor_btn = instance->add_control_to_bottom_panel(instance->hfsm_editor, str_localize("HFSM Editor"));
    add_inspector_plugin(inspector_plugin);
}
void HfsmEditorPlugin::_exit_tree() {
    if (!Engine::get_singleton()->is_editor_hint()) {
        return;
    }
    remove_inspector_plugin(inspector_plugin);
    remove_control_from_bottom_panel(hfsm_editor);
    hfsm_editor->queue_free();
}

bool HfsmEditorPlugin::is_zh() { return TranslationServer::get_singleton()->get_tool_locale().substr(0, 2) == "zh"; }

HfsmEditorPlugin *HfsmEditorPlugin::instance = nullptr;
#else

HfsmEditorPlugin *HfsmEditorPlugin::instance = nullptr;
#endif
} // namespace Hfsm