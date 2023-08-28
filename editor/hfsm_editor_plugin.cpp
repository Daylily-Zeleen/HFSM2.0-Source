/**************************************************************************/
/*  hfsm_editor_plugin.cpp                                                */
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

#include "hfsm_editor_plugin.h"

#ifdef GDEXTENSION_BUILD
#include <godot_cpp/classes/editor_file_system.hpp>
#include <godot_cpp/classes/editor_interface.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/image_texture.hpp>
#include <godot_cpp/classes/popup_menu.hpp>
#include <godot_cpp/templates/local_vector.hpp>

using namespace godot;

#else
#include <editor/editor_file_system.h>
#include <editor/editor_interface.h>
#include <scene/gui/button.h>
#include <scene/resources/texture.h>

#endif // GDEXTENSION_BUILD

#include "hfsm_editor.h"
#include "state_node.h"

#include "../src/hfsm.h"
#include "../src/transitions/variable_expressions/variable_expression_config.h"

#ifdef DEBUG_ENABLED
#include "hfsm_debugger_plugin.h"
#endif // DEBUG_ENABLED

namespace HFSM2 {

void EditorPropertyVariableConfig::VariableConfigSelector::GD_(set_create_options)(Object *p_menu_node) {
	PopupMenu *m = Object::cast_to<PopupMenu>(p_menu_node);
	if (!m) {
		return;
	}

	if (!edit_menu) {
		edit_menu = m;
		edit_menu->connect("about_to_popup", TCALLABLE(_menu_popup));
	}

	Array variable_list = hfsm->get("variable_list");

	variable_config_list.clear();
	auto idx = 0;
	for (size_t i = 0; i < variable_list.size(); i++) {
		Ref<VariableConfig> vc = variable_list[i];
		if (to_compare.is_valid() &&
				(to_compare == vc || to_compare->get_type() != vc->get_type())) {
			continue;
		}
		edit_menu->add_item(
				vformat("%s: %s%s",
						vc->get_variable_name(),
						vc->get_type_text(),
						vc->get_comment().is_empty() ? "" : (" - " + vc->get_comment())),
				idx + op_ofs);

		variable_config_list.push_back(vc);
		idx += 1;
	}

	edit_menu->add_separator();
}

bool EditorPropertyVariableConfig::VariableConfigSelector::GD_(handle_menu_selected)(int p_which) {
	auto idx = p_which - op_ofs;
	if (idx >= 0 && idx < variable_config_list.size()) {
		set_edited_resource(variable_config_list[idx]);
		emit_signal(SNAME("resource_changed"), variable_config_list[idx]);
		return true;
	}

	return false;
}

void EditorPropertyVariableConfig::VariableConfigSelector::_resource_selected(const Ref<Resource> &p_res, bool p_inspect) {
	if (p_res.is_valid() && !edit_button->is_pressed()) {
		edit_button->set_pressed_no_signal(true);
		edit_button->emit_signal(SNAME("pressed"));
	}
}

void EditorPropertyVariableConfig::VariableConfigSelector::_menu_popup() {
	// Hack
	enum MenuOption {
		OBJ_MENU_LOAD,
		OBJ_MENU_QUICKLOAD,
		OBJ_MENU_INSPECT,
		OBJ_MENU_CLEAR,
		OBJ_MENU_MAKE_UNIQUE,
		OBJ_MENU_MAKE_UNIQUE_RECURSIVE,
		OBJ_MENU_SAVE,
		OBJ_MENU_COPY,
		OBJ_MENU_PASTE,
		OBJ_MENU_SHOW_IN_FILE_SYSTEM,

		TYPE_BASE_ID = 100,
		CONVERT_BASE_ID = 1000,
	};

	ERR_FAIL_COND(!edit_menu);

	constexpr int to_remove_ids[] = {
		OBJ_MENU_LOAD,
		OBJ_MENU_QUICKLOAD,
		OBJ_MENU_INSPECT,
		OBJ_MENU_MAKE_UNIQUE,
		OBJ_MENU_MAKE_UNIQUE_RECURSIVE,
		OBJ_MENU_SAVE,
		OBJ_MENU_SHOW_IN_FILE_SYSTEM,
	};

	for (const auto id : to_remove_ids) {
		auto idx = edit_menu->get_item_index(id);
		if (idx >= 0) {
			edit_menu->remove_item(idx);
		}
	}

	edit_menu->reset_size();
}

void EditorPropertyVariableConfig::VariableConfigSelector::_bind_methods() {
	GDBIND_BEGIN(VariableConfigSelector);

	GDBIND_CALBACK(_resource_selected);
	GDBIND_CALBACK(_menu_popup);
}

EditorPropertyVariableConfig::VariableConfigSelector::VariableConfigSelector(HFSM *p_hfsm, const Ref<VariableConfig> &p_to_compare) {
	set_base_type(VariableConfig::get_class_static());
	hfsm = p_hfsm;
	if (to_compare.is_valid()) {
		to_compare = p_to_compare;
	}

	connect(SNAME("resource_selected"), TCALLABLE(_resource_selected));
	// Hack
	for (auto i = get_child_count(true) - 1; i >= 0; --i) {
		if (auto btn = cast_to<Button>(get_child(i, true))) {
			if (btn->is_toggle_mode()) {
				edit_button = btn;
				break;
			}
		}
	}
}

//
void EditorPropertyVariableConfig::_bind_methods() {
	GDBIND_BEGIN(EditorPropertyVariableConfig);
	GDBIND_CALBACK(_variable_selected);
}

void EditorPropertyVariableConfig::_variable_selected(const Ref<Resource> &p_res) {
	if (updating) {
		return;
	}
	Ref<VariableConfig> vc = p_res;
	auto obj = get_edited_object();
	auto prop = get_edited_property();
	if (to_compare.is_valid() && (to_compare == vc || to_compare->get_type() != vc->get_type())) {
		return;
	}
	emit_changed(prop, vc);
}

EditorPropertyVariableConfig::EditorPropertyVariableConfig() = default;

EditorPropertyVariableConfig::EditorPropertyVariableConfig(HFSM *p_hfsm, const Ref<VariableConfig> &p_to_compare) :
		EditorPropertyVariableConfig() {
	if (hfsm) {
		return;
	}
	hfsm = p_hfsm;
	if (to_compare.is_valid()) {
		to_compare = p_to_compare;
	}
	selector = memnew(VariableConfigSelector(p_hfsm, p_to_compare));
	selector->connect("resource_changed", TCALLABLE(_variable_selected));
	selector->set_h_size_flags(SizeFlags::SIZE_EXPAND_FILL);
	add_child(selector);
}

void EditorPropertyVariableConfig::update_property_internal() {
	updating = true;
	Ref<VariableConfig> vc = get_edited_object()->get(get_edited_property());
	selector->set_edited_resource(vc);
	updating = false;
}

//
bool HFSMInspectorPlugin::can_handle_internal(Object *p_object) const {
	return cast_to<VariableExpressionConfig>(p_object);
}

bool HFSMInspectorPlugin::parse_property_internal(Object *p_object, Variant::Type p_type, const String &p_name, PropertyHint p_hint_type,
		const String &p_hint_string, BitField<PropertyUsageFlags> p_usage_flags, bool p_wide) {
	if (auto vec = cast_to<VariableExpressionConfig>(p_object)) {
		if (auto hfsm = HFSMEditorPlugin::get_singleton()->get_hfsm_editor()->get_editing_hfsm()) {
			if ((p_name == "variable_config") ||
					(p_name == "value" && vec->is_variable_as_value() && vec->get_variable_config().is_valid())) {
				auto editor = memnew(EditorPropertyVariableConfig(hfsm, p_name == "value" ? vec->get_variable_config() : nullptr));
				add_property_editor(p_name, editor);
				return true;
			}
		}
	}
	return false;
}

// HFSMEditorPlugin
HFSMEditorPlugin *HFSMEditorPlugin::instance = nullptr;
Ref<ImageTexture> HFSMEditorPlugin::empty_icon_for_state_node = nullptr;

HFSMEditorPlugin::HFSMEditorPlugin() {
	CRASH_COND(instance);
	instance = this;

	StateConfig::get_animation_list = &get_animation_list_for_state_config;
	StateNode::get_empty_icon = &get_empty_icon_for_state_node;
	empty_icon_for_state_node.instantiate();

	connect("resource_saved", TCALLABLE(_referenced_script_saved));
	connect("scene_changed", TCALLABLE(_change_scene));

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
	translation.insert("Set Sub-FSM", "设置子状态机");
	translation.insert("Attach State Script", "附加状态脚本");
	translation.insert("Remove State Script", "移除状态脚本");
	// translation.insert("Resize", "重设尺寸");
	translation.insert("Move States", "移动状态");
	translation.insert("Sub FSM", "子状态机");
	translation.insert("Select States", "选择状态");
	translation.insert("Delete State Transitions", "删除状态转换");
	translation.insert("HFSM::Invalid FSMConfig", "HFSM::非法情况，要编辑的 FSMConfig 无效");
	translation.insert("Please set up a FSMConfig for selected HFSM node to start edit.", " ");
	// translation.insert("The current FSM has not contain a State.\n\n ", "当前状态机不存在状态\n\n ");
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
	translation.insert(R"("value" can't convert to the type of "VariableConfig".)", "\"value\"的无法转化为\"VariableConfig\"的类型");
	translation.insert(R"("value" is not a valid "VariableConfig".)", "\"value\" 不是一个有效的 \"VariableConfig\"");
	translation.insert(R"("value" can't convert to the type of "variable_config".)", "\"value\"的无法转化为\"variable_config\"的类型");
	translation.insert("Trigger: ", "触发器: ");
	translation.insert("Solo Trigger: ", "独立触发器: ");
	translation.insert("Union Trigger: ", "联合触发器: ");
	translation.insert("Invalid Trigger Type:", "无效触发器类型");
	translation.insert("Has not valid 'variable_config'", "没有有效的变量资源");
	translation.insert("Script: ", "脚本: ");
	translation.insert("Built-in Script: ", "内置脚本: ");
	translation.insert("Script isn't extends from 'Transition'.", "脚本不是扩展自'Transition'");
	translation.insert("You can use other type of script if this is intended.", "如果是有意的，请使用其他类型的脚本");
	translation.insert("Script is invalid!", "脚本无效!");
	translation.insert("Empty expression!", "表达式为空!");
	translation.insert("Have not valid Variable Expression.", "没有合法的变量表达式");
	translation.insert("Variable Expressions: ", "变量表达式:");
	translation.insert("Invalid \"VariableExpressionConfig\", index %d.", "无效的\"VariableExpressionConfig\", 索引: %d");
	translation.insert(R"("VariableConfig" %s is not contained in editing HFSM.)", R"("VariableConfig" %s 不存在于当前编辑中的HFSM)");
	translation.insert("Auto: ", "自动: ");
	translation.insert("Delay %d msec.", "延迟 %d 毫秒");
	translation.insert("After playing animation \"%s\" finish.", "在动画\"%s\"播放结束后");
	translation.insert("When sub FSM exit.", "子状态机退出时");
	translation.insert("After calling \"manual_exit()\".", "调用\"manual_exit()\"后");
	translation.insert("After \"_update()\" being called %d times.", "\"_update()\"被调用 %d 次后");
	translation.insert("After \"_physics_update()\" being called %d times.", "\"_physics_update()\"被调用 %d 次后");
	translation.insert("Error: The AnimationPlayer which is setted to editing HFSM has not animation \"%s\".", "Error: 设置给编辑中的HFSM的AnimationPlayer不含有指定动画\"%s\"");
	translation.insert("Warning: The editing HFSM has not setted an AnimationPlayer.", "Warning: 编辑中的HFSM未设置AnimationPlayer");
}

void emit_button_toggled(Button *p_btn, bool p_toggled) {
	p_btn->emit_signal(SNAME("toggled"), p_toggled);
}

void HFSMEditorPlugin::_referenced_script_saved(const Ref<Resource> &p_res) {
	// TODO:: Can we find a way to avoid emiting this signal for all Scripts?
	// We can't use meta to refer its TransitionConfig/StateConfig, it will be saved and cause cycle save.
	// TODO:: Detect builtin scripts change.
	// Hint: builtin scripts in inspector are not change automatically when it first time be saved, too.
	// Currently, I can only notify scripts which have real file in disk.
	// And refresh the fsm editor to update when any change of filesystem to ensure I can detect built-in script changed.
	if (auto s = cast_to<Script>(p_res.ptr())) {
		s->emit_signal(SNAME("changed"));
	}
}

void HFSMEditorPlugin::disable_hfsm_editor() const {
	hfsm_editor->edit_hfsm(nullptr);
	hfsm_editor_btn->set_pressed(false);
	emit_button_toggled(hfsm_editor_btn, false);
	hfsm_editor_btn->hide();
}

void HFSMEditorPlugin::_change_scene(Node *p_secne_root) {
	// handles_internal() is called before change_scne(), here is only disable if p_scene_root is nullptr.
	if (!p_secne_root) {
		disable_hfsm_editor();
	}
}

void HFSMEditorPlugin::_filesystem_changed() {
	ERR_FAIL_COND(!hfsm_editor);
	hfsm_editor->queue_refresh();
}

PackedStringArray HFSMEditorPlugin::get_animation_list_for_state_config() {
	ERR_FAIL_COND_V(!get_singleton(), {});
	ERR_FAIL_COND_V(!get_singleton()->get_hfsm_editor(), {});
	if (auto hfsm = get_singleton()->get_hfsm_editor()->get_editing_hfsm()) {
		if (auto player = hfsm->get_animation_player()) {
			IF_GDE(return player->get_animation_list();)
			IF_GDM({
				List<StringName> animations;
				player->get_animation_list(&animations);
				PackedStringArray ret;
				for (const auto &E : animations) {
					ret.push_back(E);
				}
				return ret;
			})
		}
	}
	return {};
}

Ref<ImageTexture> HFSMEditorPlugin::get_empty_icon_for_state_node() { return empty_icon_for_state_node; }

HFSMEditorPlugin::~HFSMEditorPlugin() {
	StateConfig::get_animation_list = nullptr;
	empty_icon_for_state_node.unref();
	StateNode::get_empty_icon = nullptr;

	inspector_plugin.unref();
	if (hfsm_editor && !hfsm_editor->is_queued_for_deletion()) {
		hfsm_editor->queue_free();
		hfsm_editor = nullptr;
	}
	instance = nullptr;
}

bool HFSMEditorPlugin::handles_internal(Object *p_object) const {
	if (auto node = cast_to<Node>(p_object)) {
		hfsm_editor->edit_hfsm(cast_to<HFSM>(node));
		if (auto hfsm = cast_to<HFSM>(node)) {
			hfsm_editor_btn->show();
			emit_button_toggled(hfsm_editor_btn, true);
		}
		return true;
	} else {
		StringName type = p_object->get_class();
		const LocalVector<StringName> hfsm_types = {
			FSMConfig::get_class_static(),
			StateConfig::get_class_static(),
			TransitionConfig::get_class_static(),
			Script::get_class_static(),
			VariableConfig::get_class_static(),
			VariableExpressionConfig::get_class_static(),
		};

		for (const auto &E : hfsm_types) {
			if (E == type) {
				return false;
			}
			IF_GDM(if (ClassDB::is_parent_class(type, E)) { return false; })
		}
	}
	disable_hfsm_editor();
	return false;
}

void HFSMEditorPlugin::_bind_methods() {
	GDBIND_BEGIN(HFSMEditorPlugin);
	GDBIND_CALBACK(_referenced_script_saved);
	GDBIND_CALBACK(_change_scene);
	GDBIND_CALBACK(_filesystem_changed);
}

void HFSMEditorPlugin::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			if (!Engine::get_singleton()->is_editor_hint()) {
				return;
			}
			if (inspector_plugin.is_null()) {
				inspector_plugin.instantiate();
			}
			hfsm_editor = HFSMEditor::create_hfsm_editor();
			hfsm_editor_btn = add_control_to_bottom_panel(hfsm_editor, str_localize("HFSM Editor"));
			add_inspector_plugin(inspector_plugin);

			EditorFileSystem *fs;
			IF_GDM(fs = get_editor_interface()->get_resource_file_system();)
			IF_GDE(fs = get_editor_interface()->get_resource_filesystem();)
			fs->connect("filesystem_changed", TCALLABLE(_filesystem_changed));

#ifdef DEBUG_ENABLED
			if (debugger_plugin.is_null()) {
				debugger_plugin.instantiate();
			}
			add_debugger_plugin(debugger_plugin);
#endif // DEBUG_ENABLED
		} break;
#ifdef GDEXTENSION_BUILD
		case NOTIFICATION_EXIT_TREE: {
			if (!Engine::get_singleton()->is_editor_hint()) {
				return;
			}
			remove_inspector_plugin(inspector_plugin);
			remove_control_from_bottom_panel(hfsm_editor);
#ifdef DEBUG_ENABLED
			remove_debugger_plugin(debugger_plugin);
#endif // DEBUG_ENABLED
		} break;
#endif // GDEXTENSION_BUILD
		default:
			break;
	}
}

EditorUndoRedoManager *HFSMEditorPlugin::create_action(const String &p_action_name) {
	auto undo_redo = get_singleton()->get_undo_redo();
	undo_redo->create_action(str_localize(p_action_name), UndoRedo::MERGE_DISABLE, get_singleton()->get_hfsm_editor()->get_editing_hfsm());
	IF_DEV(ERR_FAIL_COND_V(!get_singleton()->hfsm_editor, undo_redo);)
	get_singleton()->hfsm_editor->add_undo_redo_text(undo_redo, p_action_name);
	return undo_redo;
}

} // namespace HFSM2
