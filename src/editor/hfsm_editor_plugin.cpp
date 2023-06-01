#include "hfsm_editor_plugin.hpp"

#include <godot_cpp/classes/editor_interface.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "core/hfsm.hpp"
#include "core/transitions/variable_expressions/variable_expression_res.hpp"
#include "inspector_plugin/variable_res_selector.hpp"

using namespace godot;

namespace Hfsm {

void HfsmInspectorPlugin::_bind_methods() {}
HfsmInspectorPlugin::HfsmInspectorPlugin() = default;

bool HfsmInspectorPlugin::_can_handle(Object *p_object) const {
	return Object::cast_to<VariableExpressionRes>(p_object) != nullptr;
}

bool HfsmInspectorPlugin::_parse_property(Object *p_object, Variant::Type p_type, const String &p_name, PropertyHint p_hint_type,
		const String &p_hint_string, BitField<PropertyUsageFlags> p_usage_flags, bool p_wide) {
	// ERR_FAIL_COND_V(object == nullptr, false);
	if (auto ver = Object::cast_to<VariableExpressionRes>(p_object)) {
		if (auto hfsm = HfsmEditorPlugin::get_singleton()->get_hfsm_editor()->get_editing_hfsm()) {
			if (p_name == "variable_res") {
				auto editor = memnew(VariableResSelector);
				editor->setup(hfsm);
				add_property_editor(p_name, editor);
				return true;

			} else if (p_name == "value") {
				if (ver->is_variable_as_value() && ver->get_variable_res().is_valid()) {
					auto editor = memnew(VariableResSelector);
					editor->setup(hfsm, ver->get_variable_res());
					add_property_editor(p_name, editor);
					return true;
				}
			}
		}
	}
	return false;
}

// HfsmEditorPlugin
HfsmEditorPlugin *HfsmEditorPlugin::instance = nullptr;

HfsmEditorPlugin::HfsmEditorPlugin() {
	CRASH_COND(instance);
	inspector_plugin.instantiate();
	instance = this;

	translation.insert("HFSM Editor", "HFSM 编辑器");
	translation.insert("Animation:", "动画:");
	translation.insert("Animation Blend Time:", "动画混合时间:");
	translation.insert("Animation Speed:", "动画播放速度:");
	translation.insert("Animation Reverse:", "动画反向播放:");
	translation.insert("Plese select a 'HFSM' node to start edit.", "请选中一个 HFSM 节点开始编辑");
	translation.insert("HFSM: has duplicated State name: ", "HFSM: 存在重复的状态名称: ");
	translation.insert("Change state name", "改变状态名称");
	translation.insert(": this state is Entry State, can't set to other type.", ": 该状态当前为入口状态，不能设置为其他类型。");
	translation.insert("Change state type", "改变状态类型");
	translation.insert("set Sub-FSM", "设置子状态机");
	translation.insert("Attach state script", "附加状态脚本");
	translation.insert("resized", "重设尺寸");
	translation.insert("move states", "移动状态");
	translation.insert("Sub FSM", "子状态机");
	translation.insert("select states", "选择状态");
	translation.insert("Delete State Transitions", "删除状态转换");
	translation.insert("HFSM::Invalid FsmRes", "HFSM::非法情况，要编辑的 FsmRes 无效");
	translation.insert("The current FSM has not contain a State.\n\n ", "当前状态机不存在状态\n\n ");
	translation.insert("Click here to create a Entry State", "点击创建一个 起始状态");
	translation.insert("Add State", "添加状态");
	translation.insert("Cut States", "剪切状态");
	translation.insert("Copy States", "复制状态");
	translation.insert("Paste States", "粘贴状态");
	translation.insert("Duplicate States", "创建状态副本");
	translation.insert("Delete States", "删除状态");
	translation.insert("Delete State Transitions", "删除状态转换");
	translation.insert("Convert To Sub-FSM", "转换为子状态机");
	translation.insert("Create State Transition", "创建状态转换");
	translation.insert("Delete", "删除");
	translation.insert("Edit Sub-FSM", "编辑子状态机");
	translation.insert("Select State Transitions", "选择转换");
	translation.insert("Deselect", "取消选择");
	translation.insert(R"("value" can't convert to the type of "HFSMVariableRes".)", "\"value\"的无法转化为\"HFSMVariableRes\"的类型");
	translation.insert(R"("value" is not a valid "HFSMVariableRes".)", "\"value\" 不是一个有效的 \"HFSMVariableRes\"");
	translation.insert(R"("value" can't convert to the type of "variable_res".)", "\"value\"的无法转化为\"variable_res\"的类型");
	translation.insert("Trigger: ", "触发器: ");
	translation.insert("Solo Trigger: ", "独立触发器: ");
	translation.insert("Union Trigger: ", "联合触发器: ");
	translation.insert("Invalid Trigger Type:", "无效触发器类型");
	translation.insert("Has not valid 'variable_res'", "没有有效的变量资源");
	translation.insert("Script: ", "脚本: ");
	translation.insert("Script isn't extends from 'Transition'.", "脚本不是扩展自'Transition'");
	translation.insert("You can use other type of script if this is intended.", "如果是有意的，请使用其他类型的脚本");
	translation.insert("Script is invalid!", "脚本无效!");
	translation.insert("Empty expression!", "表达式为空!");
	translation.insert("Have not valid HFSMVariable Expression.", "没有合法的变量表达式");
	translation.insert("HFSMVariable Expressions: ", "变量表达式:");
	translation.insert("Invalid \"VariableExpressionRes\".", "无效的\"VariableExpressionRes\"");
	translation.insert("Auto: ", "自动: ");
	translation.insert("Delay ", "延迟 ");
	translation.insert(" msec.", " 毫秒");
	translation.insert("When sub Fsm exit.", "子状态机退出时");
	translation.insert("After calling \"manual_exit()\".", "调用\"manual_exit()\"后");
	translation.insert("After \"_update()\" being called ", "\"_update()\"被调用 ");
	translation.insert("After \"_physics_update()\" being called ", "\"_physics_update()\"被调用 ");
	translation.insert(" times.", " 次后");
}

HfsmEditorPlugin::~HfsmEditorPlugin() {
	instance = nullptr;
}

bool HfsmEditorPlugin::_handles(Object *p_object) const {
	auto obj = Object::cast_to<Node>(p_object);
	if (obj) {
		if (obj->is_class("HFSM")) {
			hfsm_editor->edit(Object::cast_to<HFSM>(obj));
		} else {
			hfsm_editor->edit(nullptr);
		}
		return true;
	}
	return false;
}

void HfsmEditorPlugin::_enter_tree() {
	if (!Engine::get_singleton()->is_editor_hint()) {
		return;
	}
	hfsm_editor = HFSMEditor::create_hfsm_editor();
	hfsm_editor_btn = add_control_to_bottom_panel(hfsm_editor, str_localize("HFSM Editor"));
	add_inspector_plugin(inspector_plugin);
}

void HfsmEditorPlugin::_exit_tree() {
	if (!Engine::get_singleton()->is_editor_hint()) {
		return;
	}
	remove_inspector_plugin(inspector_plugin);
	remove_control_from_bottom_panel(hfsm_editor);
}

EditorUndoRedoManager *HfsmEditorPlugin::create_action(const String &p_action_name) {
	auto undo_redo = get_singleton()->get_undo_redo();
	undo_redo->create_action(str_localize(p_action_name), UndoRedo::MERGE_DISABLE, get_singleton()->get_hfsm_editor()->get_editing_hfsm());
	return undo_redo;
}

} // namespace Hfsm