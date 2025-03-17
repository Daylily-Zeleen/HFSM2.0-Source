/**************************************************************************/
/*  state_node.cpp                                                        */
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

#include "state_node.h"

#ifdef GDEXTENSION_BUILD
#include <godot_cpp/classes/time.hpp>

#include <godot_cpp/classes/h_box_container.hpp>
#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/classes/spin_box.hpp>
#include <godot_cpp/classes/v_box_container.hpp>

#include <godot_cpp/classes/editor_interface.hpp>
#include <godot_cpp/classes/image_texture.hpp>
#include <godot_cpp/classes/script.hpp>
#include <godot_cpp/classes/spin_box.hpp>

#else
#include <core/os/time.h>

#include <editor/editor_interface.hpp>

#endif // GDEXTENSION_BUILD

#include "../src/state_config.h"

#include "hfsm_editor.h"
#include "hfsm_editor_plugin.h"

namespace HFSM2 {

#define s_edit_fsm_requested "_edit_fsm_requested"

Ref<ImageTexture> (*StateNode::get_empty_icon)() = nullptr;

#define EDIT_RESOURCE(p_resource) EditorInterface::get_singleton()->edit_resource(p_resource)

String StateNode::str_localize(const String &p_en_key) const {
	return HFSMEditorPlugin::str_localize(p_en_key);
}

void StateNode::_bind_methods() {
	GDBIND_BEGIN(StateNode);

	// GDBIND_METHOD(_adjust_size);

	GDBIND_CALLBACK(_cancel_name_changed);
	GDBIND_CALLBACK(_accept_name_changed);
	GDBIND_CALLBACK(_type_option_btn_item_selected, "idx");
	GDBIND_CALLBACK(_set_has_sub_fsm_check_box, "pressed");
	GDBIND_CALLBACK(_request_edit_sub_fsm_config);

	GDBIND_CALLBACK(_script_selected, "script", "edit");
	GDBIND_CALLBACK(_script_changed, "script");
	GDBIND_CALLBACK(_setup_state_config);

	ADD_SIGNAL(MethodInfo(s_edit_fsm_requested, PropertyInfo(Variant::OBJECT, "sub_fsm_config", PROPERTY_HINT_RESOURCE_TYPE, FSMConfig::get_class_static())));
	ADD_SIGNAL(MethodInfo("_reconnected_requested", PropertyInfo(Variant::STRING_NAME, "old_node_name"), PropertyInfo(Variant::STRING_NAME, "new_node_name")));
}

Ref<StateConfig> StateNode::get_state_config() const { return state_config; }

void StateNode::set_state_config(const Ref<class StateConfig> &p_state_config) {
	state_config = p_state_config;
	state_config->connect(s_changed, TCALLABLE(_setup_state_config));
	_setup_state_config();
}

StateNode *StateNode::create_state_node(const Ref<StateConfig> &p_target_state_config, const Ref<FSMConfig> &p_nested_fsm_config, bool p_debug) {
	if (p_target_state_config.is_null()) {
		return nullptr;
	}
	auto ret = memnew(StateNode(p_debug));
	ret->initialize();
	ret->nested_fsm_config = p_nested_fsm_config;
	ret->set_state_config(p_target_state_config);
	// ret->set_name(String("@StateNode@") + uitos(Time::get_singleton()->get_ticks_msec() + uint64_t(ret->get_instance_id())));
	p_target_state_config->set_state_node(ret);
	return ret;
}

bool StateNode::__has_duplicate_name(const String &p_to_test_name) {
	auto brothers = nested_fsm_config->get_state_config_list();
	for (size_t i = 0; i < brothers.size(); i++) {
		if (auto sc = cast_to<StateConfig>(brothers[i])) {
			if (sc != state_config.ptr() && sc->get_state_name() == p_to_test_name) {
				return true;
			}
		}
	}
	return false;
}

void StateNode::_setup_state_config() {
	ERR_FAIL_COND(state_config.is_null());

	auto old_name = get_name();
	set_name("@StateNode@" + state_config->get_state_name());
	emit_signal(SNAME("_reconnected_requested"), old_name, get_name());

	name_line_edit->set_text(state_config->get_state_name());
	set_title(name_line_edit->get_text());
	// 类型
	type_option_btn->clear();
	switch (state_config->get_type()) {
		case State::STATE_TYPE_ENTRY: {
			type_option_btn->add_item("Entry", State::STATE_TYPE_ENTRY);
		} break;
		case State::STATE_TYPE_NORMAL:
		case State::STATE_TYPE_EXIT: {
			type_option_btn->add_item("Entry", State::STATE_TYPE_ENTRY);
			type_option_btn->add_item("Normal", State::STATE_TYPE_NORMAL);
			type_option_btn->add_item("Exit", State::STATE_TYPE_EXIT);
		} break;
		default:
			break;
	}
	type_option_btn->select(type_option_btn->get_item_index(state_config->get_type()));
	// 子状态机
	has_sub_fsm_check_box->set_pressed(state_config->get_sub_fsm_config().is_valid());
	sub_fsm_btn->set_disabled(!has_sub_fsm_check_box->is_pressed());
	// 脚本
	script_picker->set_edited_resource(state_config->get_state_script());
	if (state_config->is_script_valid()) {
		const Color white = Color::named("white");
		script_picker->set_modulate(white);
		set_self_modulate(white);
	} else {
		script_picker->set_modulate(Color::named("INDIAN_RED"));
		set_self_modulate(Color::named("ORANGE"));
	}
	// 位置
	if (is_inside_tree()) {
		set_position_offset(state_config->get_editor_offset());
	}
	set_deferred(SNAME("size"), Vector2());
}

// ==================
void StateNode::_cancel_name_changed() {
	name_line_edit->set_text(state_config->get_state_name());
	set_deferred(SNAME("size"), Vector2());
}

void StateNode::_accept_name_changed(const String &new_name) {
	if (debug_mode) {
		return;
	}

	if (name_line_edit->get_text() == state_config->get_state_name()) {
		return;
	}

	if (__has_duplicate_name(name_line_edit->get_text())) {
		name_line_edit->set_text(state_config->get_state_name());
		ERR_FAIL_MSG(str_localize("HFSM: has duplicated State name: ") + name_line_edit->get_text());
	}
	// Undo Redo
	HFSM_EDITOR_CREATE_ACTION("Change state name");
	ADD_DO_METHOD(state_config.ptr(), set_state_name, name_line_edit->get_text());
	ADD_DO_METHOD(this, set_title, name_line_edit->get_text());
	ADD_UNDO_METHOD(this, set_title, state_config->get_state_name());
	ADD_UNDO_METHOD(state_config.ptr(), set_state_name, state_config->get_state_name());
	COMMIT_ACTION();
}

void StateNode::_type_option_btn_item_selected(int32_t p_idx) {
	if (debug_mode) {
		return;
	}

	auto id = type_option_btn->get_item_id(p_idx);
	ERR_FAIL_COND(id < 0 || id > 3);

	auto target_type = (State::StateType)id;

	if (state_config->get_type() == target_type) {
		return;
	}

	ERR_FAIL_COND_MSG(state_config->get_type() == State::STATE_TYPE_ENTRY, "HFSM::" + itos(state_config->get_type()) + str_localize(": this state is Entry State, can't set to other type."));

	switch (target_type) {
		case State::STATE_TYPE_NORMAL: {
			HFSM_EDITOR_CREATE_ACTION("Change state type");
			ADD_DO_METHOD(state_config.ptr(), set_type, State::STATE_TYPE_NORMAL);
			ADD_UNDO_METHOD(state_config.ptr(), set_type, state_config->get_type());
			COMMIT_ACTION();
		} break;
		case State::STATE_TYPE_ENTRY: {
			HFSM_EDITOR_CREATE_ACTION("Change state type");
			auto brother_state_config_list = nested_fsm_config->get_state_config_list();
			for (size_t i = 0; i < brother_state_config_list.size(); i++) {
				Ref<StateConfig> sc = brother_state_config_list[i];
				if (sc.is_valid() && sc != state_config.ptr() &&
						sc->get_type() == State::STATE_TYPE_ENTRY) {
					ADD_DO_METHOD(sc.ptr(), set_type, State::STATE_TYPE_NORMAL);
					ADD_UNDO_METHOD(sc.ptr(), set_type, state_config->get_type());
				}
			}
			ADD_DO_METHOD(state_config.ptr(), set_type, State::STATE_TYPE_ENTRY);
			ADD_UNDO_METHOD(state_config.ptr(), set_type, state_config->get_type());
			COMMIT_ACTION();
		} break;
		case State::STATE_TYPE_EXIT: {
			HFSM_EDITOR_CREATE_ACTION("Change state type");
			ADD_DO_METHOD(state_config.ptr(), set_type, State::STATE_TYPE_EXIT);
			ADD_UNDO_METHOD(state_config.ptr(), set_type, state_config->get_type());
			COMMIT_ACTION();
		} break;
		default:
			break;
	}
}

void StateNode::_set_has_sub_fsm_check_box(bool p_pressed) {
	if (debug_mode) {
		return;
	}

	if (state_config->get_sub_fsm_config().is_null() && !p_pressed) {
		return;
	}
	if (state_config->get_sub_fsm_config().is_valid() && p_pressed) {
		return;
	}
	HFSM_EDITOR_CREATE_ACTION("Set Sub-FSM");
	Ref<FSMConfig> new_sub_fsm;
	new_sub_fsm.instantiate();
	new_sub_fsm->set_nested_state_config(state_config);
	ADD_DO_METHOD(state_config.ptr(), set_sub_fsm_config, p_pressed ? new_sub_fsm : nullptr);
	ADD_UNDO_METHOD(state_config.ptr(), set_sub_fsm_config, state_config->get_sub_fsm_config());
	COMMIT_ACTION();
}

void StateNode::_request_edit_sub_fsm_config() {
	if (state_config->get_sub_fsm_config().is_valid()) {
		emit_signal(SNAME(s_edit_fsm_requested), state_config->get_sub_fsm_config());
	}
}

void StateNode::_script_selected(const Ref<Script> &p_script, bool p_edit) {
	if (debug_mode) {
		return;
	}

	EDIT_RESOURCE(p_script);
}

void StateNode::_script_changed(const Ref<Script> &p_script) {
	if (debug_mode) {
		return;
	}

	if (p_script == state_config->get_state_script()) {
		return;
	}
	HFSM_EDITOR_CREATE_ACTION(p_script.is_valid() ? "Attach State Script" : "Remove State Script");
	ADD_DO_METHOD(state_config.ptr(), set_state_script, p_script);
	ADD_UNDO_METHOD(state_config.ptr(), set_state_script, state_config->get_state_script());
	COMMIT_ACTION();
}

void _set_mouse_filter_ignore(Node *p_node) {
	auto children = p_node->get_children(true);
	for (auto i = 0; i < children.size(); ++i) {
		Node *child = Object::cast_to<Node>(children[i]);
		if (auto ctrl = Object::cast_to<Control>(child)) {
			ctrl->set_mouse_filter(Control::MOUSE_FILTER_IGNORE);
		}
		_set_mouse_filter_ignore(child);
	}
}

void StateNode::initialize() {
	// 节点结构====
	{
		auto v_box = memnew(VBoxContainer);
		add_child(v_box);
		// 名称输入行
		name_line_edit = memnew(LineEdit);
		name_line_edit->set_auto_translate(false);
		name_line_edit->set_placeholder("state name");
		name_line_edit->set_expand_to_text_length_enabled(true);
		name_line_edit->set_h_size_flags(SIZE_EXPAND_FILL);
		v_box->add_child(name_line_edit);
		// 类型选择框
		type_option_btn = memnew(OptionButton);
		v_box->add_child(type_option_btn);
		// 子状态机
		auto h_box = memnew(HBoxContainer);
		v_box->add_child(h_box);
		has_sub_fsm_check_box = memnew(CheckBox);
		h_box->add_child(has_sub_fsm_check_box);
		has_sub_fsm_check_box->set_h_size_flags(SIZE_EXPAND);
		sub_fsm_btn = memnew(Button);
		sub_fsm_btn->set_text(str_localize("Sub FSM"));
		h_box->add_child(sub_fsm_btn);

		// 脚本拾取器
		script_picker = memnew(EditorResourcePicker);
		script_picker->set_base_type(Script::get_class_static());
		v_box->add_child(script_picker);

		if (debug_mode) {
			name_line_edit->set_selecting_enabled(false);
			name_line_edit->set_editable(false);
			script_picker->set_editable(false);
			type_option_btn->set_disabled(true);
			has_sub_fsm_check_box->set_disabled(true);
			sub_fsm_btn->set_disabled(true);

			_set_mouse_filter_ignore(script_picker);
			script_picker->set_mouse_filter(MOUSE_FILTER_IGNORE);
			sub_fsm_btn->set_mouse_filter(MOUSE_FILTER_IGNORE);
			has_sub_fsm_check_box->set_mouse_filter(MOUSE_FILTER_IGNORE);
			type_option_btn->set_mouse_filter(MOUSE_FILTER_IGNORE);
			name_line_edit->set_mouse_filter(MOUSE_FILTER_IGNORE);
		}
	}

	if (get_empty_icon) {
		add_theme_icon_override(SNAME("port"), get_empty_icon());
	}

	set_slot(0, true, IN_TYPE, IN_COLOR(), true, OUT_TYPE, OUT_COLOR());
	set_resizable(false);

	// 信号功能连接
	{ // 名称输入行
		name_line_edit->connect("focus_exited", TCALLABLE(_cancel_name_changed));
		name_line_edit->connect("text_change_rejected", TCALLABLE(_cancel_name_changed));
		name_line_edit->connect("text_submitted", TCALLABLE(_accept_name_changed));
		// 类型选择框
		type_option_btn->connect("item_selected", TCALLABLE(_type_option_btn_item_selected));
		// 子状态机
		has_sub_fsm_check_box->connect("toggled", TCALLABLE(_set_has_sub_fsm_check_box));
		sub_fsm_btn->connect("pressed", TCALLABLE(_request_edit_sub_fsm_config), CONNECT_DEFERRED);
		// 脚本拾取器
		script_picker->connect("resource_selected", TCALLABLE(_script_selected));
		script_picker->connect("resource_changed", TCALLABLE(_script_changed));
	}
}

void StateNode::_notification(int p_what) {
	if (p_what == NOTIFICATION_READY) {
		ERR_FAIL_COND(state_config.is_null());
		set_size(Vector2());
		set_position_offset(state_config->get_editor_offset());

	} else if (p_what == NOTIFICATION_RESIZED) {
		static const StringName &port_h_offset = ([]() -> const StringName & {
			static StringName sname = "port_h_offset";
			IF_GDE_COMPATIBLE({
				if (likely(!HFSMGlobal::is_4_point_2_or_later())) {
					sname = "port_offset";
				}
			})
			return sname;
		})();

		add_theme_constant_override(port_h_offset, int(get_size().x / 2.0f));
	}
}

void StateNode::set_debug_activated(bool p_activated) {
	debug_activated = p_activated;
}

bool StateNode::is_debug_activated() const {
	return debug_activated;
}

StateNode::StateNode(bool p_debug_mode) :
		debug_mode(p_debug_mode) {}

} // namespace HFSM2
