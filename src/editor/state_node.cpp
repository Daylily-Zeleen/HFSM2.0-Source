#include "state_node.hpp"

#include <godot_cpp/classes/time.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include <godot_cpp/classes/editor_interface.hpp>
#include <godot_cpp/classes/h_box_container.hpp>
#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/classes/spin_box.hpp>
#include <godot_cpp/classes/v_box_container.hpp>

#include "core/fsm_res.hpp"
#include "core/state_res.hpp"
#include "hfsm_editor_plugin.hpp"
#include "state_nodes_editor.hpp"

// #include <hfsm_global.hpp>

using namespace godot;
namespace Hfsm {

String StateNode::str_localize(const String &p_en_key) const {
	return HfsmEditorPlugin::str_localize(p_en_key);
}

void StateNode::_bind_methods() {
	GDBIND_BEGIN(StateNode);
	GDBIND_METHOD(__cancel_name_changed);
	GDBIND_METHOD(__accept_name_changed);
	GDBIND_METHOD(__type_option_btn_item_selected, "idx");
	GDBIND_METHOD(__set_has_sub_fsm_check_box, "pressed");
	GDBIND_METHOD(__request_edit_sub_fsm_res);

	GDBIND_METHOD(__script_selected, "script", "edit");
	GDBIND_METHOD(__script_changed, "script");
	GDBIND_METHOD(__resize_requested, "size");
	GDBIND_METHOD(__reset_state_res);
	GDBIND_METHOD(__resize);
	GDBIND_METHOD(__dragged);
}

StateNode *StateNode::create_state_node(const Ref<StateRes> &p_target_state_res) {
	if (p_target_state_res.is_null()) {
		return nullptr;
	}
	auto r = memnew(StateNode);
	r->__setup_structure();
	r->__setup_state_res(p_target_state_res);
	r->set_name(String("@") + uitos(Time::get_singleton()->get_ticks_msec() + r->get_instance_id()));
	p_target_state_res->set_state_node(r);
	return r;
}
bool StateNode::__has_duplicate_name(const String &p_to_test_name) {
	auto brothers = __get_brother_state_res_list();
	for (size_t i = 0; i < brothers.size(); i++) {
		auto sr = Object::cast_to<StateRes>(brothers[i]);
		if (sr && sr != state_res.ptr()) {
			if (sr->get_state_name() == p_to_test_name) {
				return true;
			}
		}
	}
	return false;
}
Array StateNode::__get_brother_state_res_list() {
	auto nested_fsm_res = HfsmEditorPlugin::get_singleton()
								  ->get_hfsm_editor()
								  ->get_nested_fsm_res(state_res);
	if (nested_fsm_res.is_null()) {
		return {};
	} else {
		return nested_fsm_res->get_state_res_list();
	}
}

void StateNode::__reset_state_res() { __setup_state_res(state_res); }

void StateNode::__setup_state_res(const Ref<StateRes> &p_to_set) {
	if (state_res.is_valid()) {
		if (state_res->TIS_CONNECTED("changed", __reset_state_res)) {
			state_res->TDISCONNECT("changed", __reset_state_res);
		}
	}
	state_res = p_to_set;
	if (state_res.is_valid()) {
		if (!state_res->TIS_CONNECTED("changed", __reset_state_res)) {
			state_res->connect("changed", TCALLABLE(__reset_state_res));
		}
	} else {
		name_line_edit->set_text("<error>");
		set_title("<error>");
		return;
	}
	//
	name_line_edit->set_text(state_res->get_state_name());
	set_title(name_line_edit->get_text());
	// 类型
	type_option_btn->clear();
	switch (state_res->get_type()) {
		case State::STATE_TYPE_ENTRY: {
			type_option_btn->add_item("Entry", State::STATE_TYPE_ENTRY);
			break;
		}
		case State::STATE_TYPE_NORMAL:
		case State::STATE_TYPE_EXIT: {
			type_option_btn->add_item("Entry", State::STATE_TYPE_ENTRY);
			type_option_btn->add_item("Normal", State::STATE_TYPE_NORMAL);
			type_option_btn->add_item("Exit", State::STATE_TYPE_EXIT);
			break;
		}
		default:
			break;
	}
	type_option_btn->select(
			type_option_btn->get_item_index(state_res->get_type()));
	notify_property_list_changed();
	// 子状态机
	has_sub_fsm_check_box->set_pressed(state_res->get_fsm_res().is_valid());
	sub_fsm_btn->set_disabled(!has_sub_fsm_check_box->is_pressed());
	// 脚本
	script_picker->set_edited_resource(state_res->get_state_script());
	// 位置s
	if (!is_inside_tree()) {
		return;
	}
	__set_pos_from_res();
}

void StateNode::__set_pos_from_res() {
	auto parent = Object::cast_to<StateNodesEditor>(get_parent());
	if (parent) {
		auto zoom = parent->get_zoom();
		auto scroll_offsetr = parent->get_scroll_ofs();
		set_position_offset(state_res->get_editor_offset());
	}
}
// ==================
void StateNode::__on_resize() {
	auto size = get_size();
	size.y = 0;
	set_size(size);
}
void StateNode::__cancel_name_changed() {
	name_line_edit->set_text(state_res->get_state_name());
	__on_resize();
}
void StateNode::__accept_name_changed(const String &new_name) {
	if (name_line_edit->get_text() == state_res->get_state_name()) {
		return;
	}
	if (__has_duplicate_name(name_line_edit->get_text())) {
		UtilityFunctions::printerr(
				str_localize("HFSM: has duplicated State name: "),
				name_line_edit->get_text());
		name_line_edit->set_text(state_res->get_state_name());
		return;
	}
	// undoredo
	auto unro_redo = HfsmEditorPlugin::create_action("Change state name");
	unro_redo->add_do_property(state_res.ptr(), "name",
			name_line_edit->get_text());
	unro_redo->add_do_property(this, "title", name_line_edit->get_text());
	unro_redo->add_undo_property(state_res.ptr(), "name",
			state_res->get_state_name());
	unro_redo->add_undo_property(this, "title", state_res->get_state_name());
	unro_redo->commit_action();
}
void StateNode::__type_option_btn_item_selected(int32_t p_idx) {
	auto id = type_option_btn->get_item_id(p_idx);
	if (id < 0 || id > 3) {
		return;
	}
	auto target_type = (State::StateType)id;
	if (state_res->get_type() == target_type) {
		return;
	}
	switch (target_type) {
		case State::STATE_TYPE_NORMAL: {
			if (state_res->get_type() == State::STATE_TYPE_ENTRY) {
				UtilityFunctions::printerr(
						"HFSM::", state_res->get_type(),
						str_localize(
								": this state is Entry State, can't set to other type."));
				return;
			}

			CREATE_ACTION("Change state type");
			ADD_DO_METHOD(state_res.ptr(), set_type, State::STATE_TYPE_NORMAL);
			ADD_UNDO_METHOD(state_res.ptr(), set_type, state_res->get_type());
			COMMIT_ACTION();
		} break;
		case State::STATE_TYPE_ENTRY: {
			CREATE_ACTION("Change state type");
			auto brother_state_res_list = __get_brother_state_res_list();
			for (size_t i = 0; i < brother_state_res_list.size(); i++) {
				Ref<StateRes> sr = brother_state_res_list[i];
				if (sr.is_valid() && sr != state_res.ptr() &&
						sr->get_type() == State::STATE_TYPE_ENTRY) {
					ADD_DO_METHOD(sr.ptr(), set_type, State::STATE_TYPE_NORMAL);
					ADD_UNDO_METHOD(sr.ptr(), set_type, state_res->get_type());
				}
			}
			ADD_DO_METHOD(state_res.ptr(), set_type, State::STATE_TYPE_ENTRY);
			ADD_UNDO_METHOD(state_res.ptr(), set_type, state_res->get_type());
			COMMIT_ACTION();
		} break;
		case State::STATE_TYPE_EXIT: {
			if (state_res->get_type() == State::STATE_TYPE_ENTRY) {
				UtilityFunctions::printerr(
						"HFSM::", state_res->get_type(),
						str_localize(
								": this state is Entry State, can't set to other type."));
				CREATE_ACTION("Change state type");
				ADD_DO_METHOD(state_res.ptr(), set_type, State::STATE_TYPE_EXIT);
				ADD_UNDO_METHOD(state_res.ptr(), set_type, state_res->get_type());
				COMMIT_ACTION();
			}
		} break;
		default:
			break;
	}
}

void StateNode::__set_has_sub_fsm_check_box(bool p_pressed) {
	if (state_res->get_fsm_res().is_null() && !p_pressed) {
		return;
	}
	if (state_res->get_fsm_res().is_valid() && p_pressed) {
		return;
	}
	CREATE_ACTION("Set Sub-FSM");
	Ref<FsmRes> new_sub_fsm;
	new_sub_fsm.instantiate();
	new_sub_fsm->set_nested_state_res(state_res);
	ADD_DO_METHOD(state_res.ptr(), set_fsm_res, p_pressed ? new_sub_fsm : nullptr);
	ADD_UNDO_METHOD(state_res.ptr(), set_fsm_res, state_res->get_fsm_res());
	COMMIT_ACTION();
}
void StateNode::__request_edit_sub_fsm_res() {
	if (state_res->get_fsm_res().is_valid()) {
		HfsmEditorPlugin::get_singleton()->get_hfsm_editor()->request_edit_fsm_res(state_res->get_fsm_res());
	}
}
void StateNode::__script_selected(const Ref<Script> &p_script, bool p_edit) {
	HfsmEditorPlugin::get_singleton()->get_editor_interface()->edit_resource(
			p_script);
}
void StateNode::__script_changed(const Ref<Script> &p_script) {
	if (Object::cast_to<Script>(get_script())) {
		if (state_res->get_state_script().is_null()) {
			// 新建
			if (Engine::get_singleton()->is_editor_hint() &&
					p_script->get_source_code().is_empty()) {
				// TODO::添加模板
				p_script->set_source_code("");
			}
		}
		if (p_script == state_res->get_state_script()) {
			return;
		}
		CREATE_ACTION("Attach state script");
		ADD_DO_METHOD(state_res.ptr(), set_state_script, p_script);
		ADD_UNDO_METHOD(state_res.ptr(), set_state_script, state_res->get_state_script());
		COMMIT_ACTION();
	}
}
void StateNode::__resize_requested(Vector2 p_new_minsize) {
	auto size = get_size();
	size.x = p_new_minsize.x;
	call_deferred("__resize");
}
void StateNode::__resize() {
	if (Input::get_singleton()->is_mouse_button_pressed(MOUSE_BUTTON_LEFT)) {
		return;
	}
	if (get_size().is_equal_approx(state_res->get_size_in_editor())) {
		return;
	}
	CREATE_ACTION("resized");
	ADD_DO_METHOD(this, set_size, get_size(), false);
	ADD_UNDO_METHOD(this, set_size, state_res->get_size_in_editor(), false);
	ADD_DO_METHOD(state_res.ptr(), set_size_in_editor, get_size());
	ADD_UNDO_METHOD(state_res.ptr(), set_size_in_editor, state_res->get_size_in_editor());
	COMMIT_ACTION();
}
void StateNode::__dragged(Vector2 p_from, Vector2 p_to) {
	auto parent = Object::cast_to<StateNodesEditor>(get_parent());
	if (!parent) {
		return;
	}
	if (parent->is_dealing_move_states()) {
		return;
	}
	parent->set_dealing_move_states(true);
	CREATE_ACTION("move states");
	auto nodes = get_parent()->get_children();
	for (size_t i = 0; i < nodes.size(); i++) {
		auto node = Object::cast_to<StateNode>(nodes[i]);
		if (node) {
			if (!node->state_res->get_editor_offset().is_equal_approx(node->get_position_offset())) {
				ADD_DO_METHOD(node->state_res.ptr(), set_editor_offset, node->get_position_offset());
				ADD_UNDO_METHOD(node->state_res.ptr(), set_editor_offset, node->state_res->get_editor_offset());
			}
		}
	}
	COMMIT_ACTION();
	parent->set_dealing_move_states(false);
}
void StateNode::_ready() {
	if (state_res.is_null()) {
		return;
	}
	auto size = get_size();
	size.y = 0;
	set_size(size);
	state_res->set_size_in_editor(get_size());
	__set_pos_from_res();
}

void StateNode::__setup_structure() {
	// 节点结构====
	{
		auto v_box = memnew(VBoxContainer);
		add_child(v_box);
		// 名称输入行
		name_line_edit = memnew(LineEdit);
		v_box->add_child(name_line_edit);
		name_line_edit->set_auto_translate(false);
		name_line_edit->set_placeholder("state name");
		name_line_edit->set_expand_to_text_length_enabled(true);
		name_line_edit->set_h_size_flags(SIZE_EXPAND_FILL);
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
		// 新特性，动画 TODO 信号
		auto anim_vbox = memnew(VBoxContainer);
		v_box->add_child(anim_vbox);

		auto anim_hbox = memnew(HBoxContainer);
		anim_hbox->set_h_size_flags(Control::SIZE_EXPAND_FILL);
		auto anim_label = memnew(Label);
		anim_label->set_text(str_localize("Animation:"));
		anim_hbox->add_child(anim_label);
		anim_btn = memnew(OptionButton);
		anim_hbox->add_child(anim_btn);
#ifdef FULL_VERSION
		// TODO :: 重新组织结构，与上衣行合并， 信号响应等。
		auto anim_param_hbox = memnew(HBoxContainer);
		anim_param_hbox->set_h_size_flags(Control::SIZE_EXPAND_FILL);

		// anim_blend_time_spin_box = memnew(SpinBox);
		// anim_params_visible_btn = memnew(Button);
		// anim_param_hbox->add_child(anim_blend_time_spin_box);
		// anim_param_hbox->add_child(anim_params_visible_btn);
		// anim_blend_time_spin_box->set_prefix(str_localize("Animation Blend Time:"));

		// anim_params_visible_btn->set_text(">");
		// auto anim_param_hbox = memnew(HBoxContainer);
		// anim_param_hbox->set_h_size_flags(Control::SIZE_EXPAND_FILL);

		// anim_speed_spin_box = memnew(SpinBox);
		// anim_reverse_check_box = memnew(CheckBox);
		// anim_speed_spin_box->set_prefix(str_localize("Animation Speed:"));
		// anim_reverse_check_box->set_text(str_localize("Animation Reverse:"));

		// anim_param_hbox->add_child(anim_speed_spin_box);
		// anim_param_hbox->add_child(anim_reverse_check_box);
#endif
		// 脚本拾取器
		script_picker = memnew(EditorScriptPicker);
		script_picker->set_base_type("Script");
		v_box->add_child(script_picker);
	}
	set_slot(0, true, IN_TYPE, IN_COLOR, true, OUT_TYPE, OUT_COLOR);
	set_resizable(true);

	// 信号功能连接
	{ // 名称输入行
		name_line_edit->connect("focus_exited", TCALLABLE(__cancel_name_changed));
		name_line_edit->connect("text_change_rejected", TCALLABLE(__cancel_name_changed));
		name_line_edit->connect("text_submitted", TCALLABLE(__accept_name_changed));
		// 类型选择框
		type_option_btn->connect("item_selected", TCALLABLE(__type_option_btn_item_selected));
		// 子状态机
		has_sub_fsm_check_box->connect("toggled", TCALLABLE(__set_has_sub_fsm_check_box));
		sub_fsm_btn->connect("pressed", TCALLABLE(__request_edit_sub_fsm_res));
		// 脚本拾取器
		script_picker->connect("resource_selected", TCALLABLE(__script_selected));
		script_picker->connect("resource_changed", TCALLABLE(__script_changed));
		// 自身
		connect("resized", TCALLABLE(__resize));
		connect("resize_request", TCALLABLE(__resize_requested));
		connect("dragged", TCALLABLE(__dragged));
	}
}

void StateNode::_notification(int p_what) {
	if (p_what == NOTIFICATION_PARENTED || p_what == NOTIFICATION_UNPARENTED) {
		auto state_nodes_dite = Object::cast_to<StateNodesEditor>(get_parent());
		if (state_nodes_dite) {
			state_nodes_dite->update_cnnection();
		}
	}
}

StateNode::StateNode() = default;

} // namespace Hfsm
