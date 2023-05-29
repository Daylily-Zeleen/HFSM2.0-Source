#include "state_nodes_editor.hpp"

#include <godot_cpp/classes/editor_interface.hpp>
#include <godot_cpp/classes/font.hpp>
#include <godot_cpp/classes/gd_script.hpp>
#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/classes/input_event_mouse_button.hpp>
#include <godot_cpp/classes/input_event_mouse_motion.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/theme.hpp>
#include <godot_cpp/variant/callable.hpp>

#include "core/transitions/transition.hpp"
#include "hfsm_editor_plugin.hpp"
#include "src/core/fsm_res.hpp"
#include "src/core/transition_res.hpp"

#include "state_node.hpp"

using namespace godot;
namespace Hfsm {
String StateNodesEditor::str_localize(const String &en_key) const { return HfsmEditorPlugin::str_localize(en_key); }
void StateNodesEditor::_bind_methods() {
	ClassDB::bind_method(D_METHOD("__set_current_fsm_res", "current_fsm_res"), &StateNodesEditor::__set_current_fsm_res);
	ClassDB::bind_method(D_METHOD("__set_selected_transition_res_list", "val"), &StateNodesEditor::__set_selected_transition_res_list);
	ClassDB::bind_method(D_METHOD("__set_copied_state_res_list", "val"), &StateNodesEditor::__set_copied_state_res_list);
	ClassDB::bind_method(D_METHOD("__set_copied_transition_list", "val"), &StateNodesEditor::__set_copied_transition_list);
	ClassDB::bind_method(D_METHOD("__set_selected_state_name_list", "val"), &StateNodesEditor::__set_selected_state_name_list);

	ClassDB::bind_method(D_METHOD("__on_current_fsm_res_changed"), &StateNodesEditor::__on_current_fsm_res_changed);
	ClassDB::bind_method(D_METHOD("__on_transition_res_updated"), &StateNodesEditor::__on_transition_res_updated);
	ClassDB::bind_method(D_METHOD("__update_conntion"), &StateNodesEditor::__update_conntion);
	ClassDB::bind_method(D_METHOD("__set_updating", "val"), &StateNodesEditor::__set_updating);
	ClassDB::bind_method(D_METHOD("__select_state_nodes", "val"), &StateNodesEditor::__select_state_nodes);
	ClassDB::bind_method(D_METHOD("___select_mamually", "val"), &StateNodesEditor::___select_mamually);
	ClassDB::bind_method(D_METHOD("__on_popup_menu_id_pressed", "id"), &StateNodesEditor::__on_popup_menu_id_pressed);
	ClassDB::bind_method(D_METHOD("__on_delete_nodes_request", "nodes"), &StateNodesEditor::__on_delete_nodes_request);
	ClassDB::bind_method(D_METHOD("__on_connection_request", "from", "from_slot", "to", "to_slot"), &StateNodesEditor::__on_connection_request);
	ClassDB::bind_method(D_METHOD("__on_popup_request", "position"), &StateNodesEditor::__on_popup_request);
	ClassDB::bind_method(D_METHOD("__on_node_selected", "node"), &StateNodesEditor::__on_node_selected);
	ClassDB::bind_method(D_METHOD("__on_node_deselected", "node"), &StateNodesEditor::__on_node_deselected);
	ClassDB::bind_method(D_METHOD("__on_create_btn_pressed"), &StateNodesEditor::__on_create_btn_pressed);
	ClassDB::bind_method(D_METHOD("__check_empty_fsm_res_or_not", "val"), &StateNodesEditor::__check_empty_fsm_res_or_not);
	// // =======HACK=======
	// ClassDB::bind_method(D_METHOD("__on_copy_requested"), &StateNodesEditor::__on_copy_requested);
	// ClassDB::bind_method(D_METHOD("__on_paste_requested"), &StateNodesEditor::__on_paste_requested);
	// ClassDB::bind_method(D_METHOD("__on_duplicate_requested"), &StateNodesEditor::__on_duplicate_requested);
	// ClassDB::bind_method(D_METHOD("__on_edit_fsm_res_requeted"), &StateNodesEditor::__on_edit_fsm_res_requeted);
	// // =======HACK=======
}
// void StateNodesEditor::__on_edit_fsm_res_requeted() {
// 	for (size_t i = 0; i < path_button_container->get_child_count(); i++) {
// 		auto btn = Object::cast_to<Button>(path_button_container->get_child(1));
// 		if (btn && btn->is_pressed()) {
// 			Ref<FsmRes> fsm_res = btn->get_meta("fsm_res");
// 			if (fsm_res.is_valid()) {
// 				edit_fsm_res(fsm_res);
// 				return;
// 			}
// 		}
// 	}
// }

// ========== SetGet =========
bool StateNodesEditor::is_dealing_move_states() { return dealing_move; }
void StateNodesEditor::set_dealing_move_states(bool dealing) { dealing_move = dealing; };

void StateNodesEditor::__set_current_fsm_res(const Ref<FsmRes> &to_set) {
	if (current_fsm_res.is_valid() && current_fsm_res->is_connected("changed", Callable(this, "__on_current_fsm_res_changed"))) {
		current_fsm_res->disconnect("changed", Callable(this, "__on_current_fsm_res_changed"));
	}
	current_fsm_res = to_set;
	if (current_fsm_res.is_valid() && !current_fsm_res->is_connected("changed", Callable(this, "__on_current_fsm_res_changed"))) {
		current_fsm_res->connect("changed", Callable(this, "__on_current_fsm_res_changed"));
	}
}
void StateNodesEditor::__set_selected_state_name_list(const TypedArray<StringName> &to_set) {
	selected_state_name_list = to_set;
	selected_transition_res_list.clear();
	auto connection_list = get_connection_list();
	for (auto i = 0; i < connection_list.size(); i++) {
		Dictionary conn = connection_list[i];
		auto from = get_node<StateNode>({ conn["from"].operator godot::StringName() });
		auto to = get_node<StateNode>({ conn["to"].operator godot::StringName() });
		if (from && to) {
			if (selected_state_name_list.has(from->state_res->get_state_name()) && selected_state_name_list.has(to->state_res->get_state_name())) {
				auto tr = __get_transition_res(from, to);
				if (tr.is_valid()) {
					selected_transition_res_list.push_back(tr);
				}
			}
		}
	}
	__set_selected_transition_res_list(selected_transition_res_list);
}
TypedArray<StateNode> StateNodesEditor::get_selected_state_nodes() {
	TypedArray<StateNode> ret;
	for (auto i = 0; i < get_child_count(); i++) {
		auto c = Object::cast_to<StateNode>(get_child(i));
		if (c && selected_state_name_list.has(c->state_res->get_state_name())) {
			ret.push_back(c);
		}
	}
	return ret;
}
void StateNodesEditor::__set_selected_transition_res_list(const TypedArray<TransitionRes> &to_set) {
	selected_transition_res_list = to_set;
	Ref<TransitionRes> to_inspect = selected_transition_res_list.size() == 1 ? selected_transition_res_list.front() : nullptr;
	HfsmEditorPlugin::get_singleton()->get_editor_interface()->inspect_object(to_inspect.ptr());
	auto conn_list = get_incoming_connections();
	static Ref<TransitionRes> inspecting;
	if (inspecting.is_valid()) {
		inspecting->disconnect("changed", Callable(this, "__on_transition_res_updated"));
		inspecting = Variant(); // 同时只能监视一个 TransitionRes
	}
	if (to_inspect.is_valid()) {
		to_inspect->connect("changed", Callable(this, "__on_transition_res_updated"));
		inspecting = to_inspect;
	}
	queue_redraw();
}
void StateNodesEditor::__set_copied_transition_list(const TypedArray<TransitionRes> &to_set) { copied_transition_res_list = to_set; }
void StateNodesEditor::__set_copied_state_res_list(const TypedArray<StateRes> &to_set) { copied_state_res_list = to_set; }
// ========功能=========
void StateNodesEditor::update_cnnection() {
	if (updating) {
		return;
	}
	updating = true;
	call_deferred("__update_conntion");
}
void StateNodesEditor::__update_conntion() {
	auto conn_list = get_connection_list();
	for (auto i = 0; i < conn_list.size(); i++) {
		Dictionary conn = conn_list[i];
		auto from = get_node<StateNode>({ conn["from"].operator godot::StringName() });
		auto to = get_node<StateNode>({ conn["to"].operator godot::StringName() });
		if (!from || !to) {
			disconnect_node(conn["from"], conn["from_port"], conn["to"], conn["to_port"]);
		}
	}
	queue_redraw();
	call_deferred("__set_updating", false);
}
void StateNodesEditor::__set_updating(bool to_set) { updating = to_set; }
void StateNodesEditor::__undo_redo_select_nodes() {
	auto undo_redo = HfsmEditorPlugin::create_action("select states");
	undo_redo->add_do_method(this, StringName("__set_selected_transition_res_list"), TypedArray<TransitionRes>());
	undo_redo->add_do_method(this, StringName("__select_state_nodes"), selected_state_name_list);
	undo_redo->add_undo_method(this, StringName("__select_state_nodes"), __bakcup_selected_state_name_list.duplicate());
	undo_redo->add_undo_method(this, StringName("__set_selected_transition_res_list"), selected_transition_res_list.duplicate());
	undo_redo->commit_action();
	__bakcup_selected_state_name_list = selected_state_name_list.duplicate();
};
void StateNodesEditor::___deal_selection_action() {
	__set_selected_state_name_list(selected_state_name_list);
	// 已处理
	if (_left_pressing) {
		return;
	}
	// 设置处理中
	_left_pressing = true;
	// 等待左键释放
	set_process(true);
}
void StateNodesEditor::__try_disconnect(Vector2 pos1, Vector2 pos2) {
	auto conn_list = get_connection_list();
	for (auto i = 0; i < conn_list.size(); i++) {
		Dictionary conn = conn_list[i];
		auto from = get_node<StateNode>({ conn["from"].operator godot::StringName() });
		auto to = get_node<StateNode>({ conn["to"].operator godot::StringName() });
		if (from && to) {
			auto line = __get_connection_line_with_zoom(from, to);
			Vector2 from_pos = line[0];
			Vector2 to_pos = line[1];
			if (__is_judge(pos1, pos2, from_pos, to_pos)) {
				__delete_transition(conn["from"], conn["from_port"], conn["to"], conn["to_port"]);
			}
		}
	}
}

bool StateNodesEditor::__is_judge(Vector2 apos1, Vector2 apos2, Vector2 bpos1, Vector2 bpos2) {
	// x 投影重叠
	if ((MAX(apos1.x, apos2.x) >= MIN(bpos1.x, bpos2.x)) && (MIN(apos1.x, apos2.x) <= MAX(bpos1.x, bpos2.x))) {
		// y 投影重叠
		if ((MAX(apos1.y, apos2.y) >= MIN(bpos1.y, bpos2.y)) && (MIN(apos1.y, apos2.y) <= MAX(bpos1.y, bpos2.y))) {
			// A 是否跨过 B
			if ((bpos1 - apos1).cross(apos2 - apos1) * (bpos2 - apos1).cross(apos2 - apos1) <= 0) {
				// B 是否跨过 A
				if ((apos1 - bpos1).cross(bpos2 - bpos1) * (apos2 - bpos1).cross(bpos2 - bpos1) <= 0) {
					return true;
				}
			}
		}
	}
	return false;
}
void StateNodesEditor::__delete_transition(const StringName &from, int32_t from_slot, const StringName &to, int32_t to_slot) {
	auto from_node = Object::cast_to<StateNode>(find_child(from, false, false));
	auto to_node = Object::cast_to<StateNode>(find_child(to, false, false));
	auto tr = __get_transition_res(from_node, to_node);
	if (tr.is_valid()) {
		auto undo_redo = HfsmEditorPlugin::create_action("Delete State Transitions");
		undo_redo->add_do_method(this, StringName("disconnect_node"), from, from_slot, to, to_slot);
		undo_redo->add_do_method(current_fsm_res.ptr(), StringName("remove_transition_res"), tr);
		undo_redo->add_undo_method(current_fsm_res.ptr(), StringName("add_transition_res"), tr);
		undo_redo->add_do_method(this, StringName("connect_node"), from, from_slot, to, to_slot);
		undo_redo->commit_action();
	}
}
TypedArray<TransitionRes> StateNodesEditor::__try_select_transitions_at_pos(Vector2 pos) {
	TypedArray<TransitionRes> ret;
	auto conn_list = get_connection_list();
	for (auto i = 0; i < conn_list.size(); i++) {
		Dictionary conn = conn_list[i];
		auto childrens = get_children();
		auto from = get_node<StateNode>({ conn["from"].operator godot::StringName() });
		auto to = get_node<StateNode>({ conn["to"].operator godot::StringName() });
		auto origin_line = __get_connection_line_with_zoom(from, to);
		Vector2 from_pos = origin_line[0];
		Vector2 to_pos = origin_line[1];
		// 取 转换线 的垂直方向, 以 鼠标
		// 双击点为基准，向两边延申，取得测试线段的两端点
		auto ab = to_pos - from_pos;
		auto verti_ab_extent = ab.rotated(Math_PI * 0.5f).normalized() * TRANSITION_SELECT_EXTENT;
		auto test_segment_p1 = pos + verti_ab_extent;
		auto test_segment_p2 = pos - verti_ab_extent;
		// 测试线段于转换线是否相交
		if (__is_judge(test_segment_p1, test_segment_p2, from_pos, to_pos)) {
			// 相交， 在识别范围内
			auto tr = __get_transition_res(from, to);
			if (tr.is_valid()) {
				ret.push_back(tr);
			}
		}
	}

	return ret;
}

TypedArray<Vector2> StateNodesEditor::__get_connection_line_with_zoom(StateNode *from, StateNode *to) {
	auto from_pos = from->get_connection_output_position(0) + from->get_position();
	auto to_pos = from->get_connection_input_position(0) + to->get_position();
	auto zoom = static_cast<float>(get_zoom());
	auto origin_line = get_connection_line(from_pos / zoom, to_pos / zoom);
	from_pos = origin_line[0] * zoom;
	to_pos = origin_line[1] * zoom;
	return TypedArray<Vector2>::make(from_pos, to_pos);
}
Ref<TransitionRes> StateNodesEditor::__get_transition_res(StateNode *from, StateNode *to) {
	auto tr_list = current_fsm_res->get_transition_res_list();
	for (auto i = 0; i < tr_list.size(); i++) {
		Ref<TransitionRes> tr = tr_list[i];
		if (tr.is_valid() && tr->get_from_state_res() == from->state_res && tr->get_to_state_res() == to->state_res) {
			return tr;
		}
	}
	return nullptr;
}
bool StateNodesEditor::__is_node_hotzone(Object *in_node, int64_t in_port, const Vector2 &mouse_position) {
	if (!Input::get_singleton()->is_key_pressed(KEY_SHIFT)) {
		return false;
	}
	auto zoom = static_cast<float>(get_zoom());
	auto zoomed_pos = mouse_position * zoom;
	auto graph_node = Object::cast_to<StateNode>(in_node);
	if (!graph_node) {
		return false;
	}
	auto rect = graph_node->get_rect();
	rect.set_size(rect.get_size() * zoom);
	auto end = rect.get_end();
	auto zoomed_size = SCALE_DRAGGER_SIZE * zoom;
	auto pos = end - zoomed_size;
	auto dragger_rect = Rect2(pos, zoomed_size);
	rect = rect.grow_side(SIDE_TOP, -MOVE_ZONE_HIGHT * zoom);
	return rect.has_point(zoomed_pos) && !dragger_rect.has_point(zoomed_pos);
}
TypedArray<StateNode> StateNodesEditor::__get_selected_state_nodes() {
	TypedArray<StateNode> ret;
	for (auto i = 0; i < get_child_count(); i++) {
		auto node = Object::cast_to<StateNode>(get_child(i));
		if (node && node->is_selected()) {
			ret.push_back(node);
		}
	}
	return ret;
}

void StateNodesEditor::__select_state_nodes(const TypedArray<StringName> &to_select_State_name_list) {
	__set_selected_state_name_list(to_select_State_name_list);
	for (auto i = 0; i < get_child_count(); i++) {
		auto node = Object::cast_to<StateNode>(get_child(i));
		if (node) {
			node->set_selected(selected_state_name_list.has(node->state_res->get_state_name()));
		}
	}
}
StateNode *StateNodesEditor::____create_state_node(const Ref<StateRes> &state_res) { return StateNode::create_state_node(state_res); }

StateNode *StateNodesEditor::___get_top_state_node_which_hovered() {
	auto zoom = static_cast<float>(get_zoom());
	for (int i = get_child_count() - 1; i >= 0; i--) {
		auto node = Object::cast_to<StateNode>(get_child(i));
		if (node) {
			auto rect = node->get_rect();
			rect.set_size(rect.get_size() * zoom);
			if (rect.has_point(get_local_mouse_position() * zoom)) {
				return node;
			}
		}
	}
	return nullptr;
}
TypedArray<StateRes> StateNodesEditor::___get_selected_state_res_list() {
	TypedArray<StateRes> ret;
	auto seleted_state_nodes = get_selected_state_nodes();
	for (auto i = 0; i < seleted_state_nodes.size(); i++) {
		auto sn = Object::cast_to<StateNode>(seleted_state_nodes[i]);
		ret.push_back(sn->state_res);
	}
	return ret;
}
void StateNodesEditor::___select_mamually(const TypedArray<StateNode> &target_nodes) {
	TypedArray<StringName> to_select_state_name_list;
	for (auto i = 0; i < get_child_count(); i++) {
		auto sn = Object::cast_to<StateNode>(get_child(i));
		if (sn) {
			sn->set_selected(target_nodes.has(sn));
			if (sn->is_selected()) {
				to_select_state_name_list.push_back(sn->state_res->get_state_name());
			}
		}
	}
	__set_selected_state_name_list(to_select_state_name_list.duplicate());
	__bakcup_selected_state_name_list = to_select_state_name_list;
}
// ==================
void StateNodesEditor::__on_current_fsm_res_changed() { __check_empty_fsm_res_or_not(current_fsm_res); }
void StateNodesEditor::__check_empty_fsm_res_or_not(const Ref<FsmRes> &fsm_res) {
	if (fsm_res.is_null()) {
		UtilityFunctions::printerr(str_localize("HFSM::Invalid FsmRes"));
		return;
	}
	if (fsm_res->get_state_res_list().size() <= 0) {
		mask_panel->show();
		not_state_alert->set_text(str_localize("The current FSM has not contain a State.\n\n "));
		create_btn->set_text(str_localize("Click here to create a Entry State"));
	} else {
		mask_panel->hide();
	}
}

void StateNodesEditor::__on_popup_menu_id_pressed(int32_t id) {
	switch (id) {
		case ITEM_ADD_STATE: {
			if (__hovered_state_node) {
				return;
			}
			Ref<StateRes> new_sr;
			new_sr.instantiate();
			new_sr->set_editor_offset((get_local_mouse_position() + get_scroll_ofs()) / static_cast<float>(get_zoom()));
			auto new_sn = ____create_state_node(new_sr);
			auto undo_redo = HfsmEditorPlugin::create_action("Add State");
			undo_redo->add_do_method(this, StringName("add_child"), new_sn);
			undo_redo->add_do_reference(new_sn);
			undo_redo->add_undo_method(this, StringName("remove_child"), new_sn);
			undo_redo->add_do_method(this, StringName("___select_mamually"), TypedArray<StateNode>::make(new_sn));
			undo_redo->add_undo_method(this, StringName("___select_mamually"), TypedArray<StateNode>());
			undo_redo->add_do_method(current_fsm_res.ptr(), StringName("add_state_res"), new_sr);
			undo_redo->add_undo_method(current_fsm_res.ptr(), StringName("remove_state_res"), new_sr);
			undo_redo->commit_action();
			break;
		}
		case ITEM_CUT_STATE: {
			if (selected_state_name_list.size() <= 0) {
				return;
			}
			auto undo_redo = HfsmEditorPlugin::create_action("Cut States");
			TypedArray<StateRes> to_copied_state_res;
			TypedArray<StateNode> selected_state_nodes = get_selected_state_nodes();
			for (auto i = 0; i < selected_state_nodes.size(); i++) {
				auto node = Object::cast_to<StateNode>(selected_state_nodes[i]);
				if (!node) {
					continue;
				}
				undo_redo->add_do_method(this, StringName("remove_child"), node);
				undo_redo->add_undo_method(this, StringName("add_child"), node);
				undo_redo->add_undo_reference(node);
				undo_redo->add_do_method(current_fsm_res.ptr(), StringName("remove_state_res"), node->state_res);
				undo_redo->add_undo_method(current_fsm_res.ptr(), StringName("add_state_res"), node->state_res);
				to_copied_state_res.push_back(node->state_res);
			}

			auto tr_list = current_fsm_res->get_transition_res_list();
			for (auto i = 0; i < tr_list.size(); i++) {
				Ref<TransitionRes> tr = tr_list[i];
				auto from_node = Object::cast_to<StateNode>(tr->get_from_state_res()->get("state_node"));
				auto to_node = Object::cast_to<StateNode>(tr->get_from_state_res()->get("state_node"));
				if (!from_node || !to_node || selected_state_name_list.has(tr->get_from_state_res()->get_state_name()) || selected_state_name_list.has(tr->get_to_state_res()->get_state_name())) {
					undo_redo->add_do_method(current_fsm_res.ptr(), StringName("remove_transition_res"), tr);
					undo_redo->add_undo_method(current_fsm_res.ptr(), StringName("add_transition_res"), tr);
				}
			}
			undo_redo->add_do_method(this, StringName("__set_copied_transition_list"), selected_transition_res_list.duplicate());
			undo_redo->add_undo_method(this, StringName("__set_copied_transition_list"), copied_transition_res_list);
			undo_redo->add_undo_method(this, StringName("___select_mamually"), selected_state_nodes);
			undo_redo->add_do_method(this, StringName("__set_copied_state_res_list"), to_copied_state_res);
			undo_redo->add_undo_method(this, StringName("__set_copied_state_res_list"), copied_state_res_list);
			undo_redo->commit_action();
		} break;
		case ITEM_COPY_STATES: {
			if (selected_state_name_list.size() <= 0) {
				return;
			}
			TypedArray<StateRes> to_copied_state_res_list;
			TypedArray<StateNode> selected_state_node_list = get_selected_state_nodes();
			for (auto i = 0; i < selected_state_node_list.size(); i++) {
				to_copied_state_res_list.push_back(selected_state_node_list[i].get("state_res"));
			}
			if (to_copied_state_res_list.size() == copied_state_res_list.size()) {
				bool difference = false;
				for (auto i = 0; i < to_copied_state_res_list.size(); i++) {
					Ref<StateRes> sr = to_copied_state_res_list[i];
					if (!selected_state_name_list.has(sr)) {
						difference = true;
						break;
					}
				}
				if (!difference) {
					return; // 相同，不执行拷贝， 直接返回
				}
			}
			auto undo_redo = HfsmEditorPlugin::create_action("Copy States");
			undo_redo->add_do_method(this, StringName("__set_copied_state_res_list"), to_copied_state_res_list);
			undo_redo->add_undo_method(this, StringName("__set_copied_state_res_list"), copied_state_res_list);
			undo_redo->add_do_method(this, StringName("__set_copied_transition_list"), selected_transition_res_list);
			undo_redo->add_undo_method(this, StringName("__set_copied_transition_list"), copied_transition_res_list);
			undo_redo->commit_action();
		} break;
		case ITEM_PASTE_STATES: {
			if (copied_state_res_list.size() <= 0) {
				return;
			}
			auto undo_redo = HfsmEditorPlugin::create_action("Paste States");
			HashMap<Ref<StateRes>, Ref<StateRes>> osr2csr;
			// 计算中心
			Vector2 center;
			for (auto i = 0; i < copied_state_res_list.size(); i++) {
				Ref<StateRes> state_res = copied_state_res_list[i];
				center += state_res->get_editor_offet();
			}
			center /= static_cast<float>(copied_state_res_list.size());
			auto mouse_offset = (get_local_mouse_position() + get_scroll_ofs()) / static_cast<float>(get_zoom());
			// 计算偏移
			auto offset = center - mouse_offset;
			// 复制
			for (auto i = 0; i < copied_state_res_list.size(); i++) {
				Ref<StateRes> sr = copied_state_res_list[i];
				Ref<StateRes> csr = sr->duplicate(true);
				csr->set_editor_offset(csr->get_editor_offet() - offset);
				auto csn = ____create_state_node(csr);
				osr2csr.insert(sr, csr);
			}
			// 添加
			TypedArray<StateNode> copied_state_ndoes;
			for (const auto &kv : osr2csr) {
				auto csn = Object::cast_to<StateNode>(kv.value->get("state_node"));
				copied_state_ndoes.push_back(csn);
				undo_redo->add_do_method(this, StringName("add_child"), csn);
				undo_redo->add_do_reference(csn);
				undo_redo->add_undo_method(this, StringName("remove_child"), csn);
				undo_redo->add_do_method(current_fsm_res.ptr(), StringName("add_state_res"), csn->state_res);
				undo_redo->add_undo_method(current_fsm_res.ptr(), StringName("remove_state_res"), csn->state_res);
			}
			// 拷贝相关转换
			for (auto i = 0; i < copied_transition_res_list.size(); i++) {
				Ref<TransitionRes> tr = copied_transition_res_list[i];
				if (copied_state_res_list.has(tr->get_from_state_res()) && copied_state_res_list.has(tr->get_to_state_res())) {
					Ref<TransitionRes> ctr;
					ctr.instantiate();
					ctr->set_from_state_res(osr2csr[tr->get_from_state_res()]);
					ctr->set_to_state_res(osr2csr[tr->get_to_state_res()]);
					StringName from = ctr->get_from_state_res()->get("state_node").get("name");
					StringName to = ctr->get_to_state_res()->get("state_node").get("name");
					undo_redo->add_do_method(current_fsm_res.ptr(), StringName("add_transition_res"), ctr);
					undo_redo->add_do_method(this, StringName("connect_node"), from, 0, to, 0);
					undo_redo->add_undo_method(this, StringName("disconnect_node"), from, 0, to, 0);
					undo_redo->add_undo_method(current_fsm_res.ptr(), StringName("remove_transition_res"), ctr);
				}
			}
			// 选中
			undo_redo->add_do_method(this, StringName("___select_mamually"), copied_state_ndoes);
			undo_redo->add_undo_method(this, StringName("___select_mamually"), get_selected_state_nodes());
			// 提交
			undo_redo->commit_action();
			break;
		}
		case ITEM_DUPLICATE_STATES: {
			auto selected_state_res_list = ___get_selected_state_res_list();
			if (selected_state_res_list.size() <= 0) {
				return;
			}
			auto undo_redo = HfsmEditorPlugin::create_action("Duplicate States");
			TypedArray<StateNode> csn_list;
			HashMap<Ref<StateRes>, Ref<StateRes>> osr2csr;
			for (auto i = 0; i < selected_state_res_list.size(); i++) {
				Ref<StateRes> sr = selected_state_res_list[i];
				Ref<StateRes> csr = sr->duplicate(true);
				csr->set_editor_offset(csr->get_editor_offet() + DUPLICATE_OFFSET);
				auto csn = ____create_state_node(csr);
				csn_list.push_back(csn);
				osr2csr.insert(sr, csr);
				undo_redo->add_do_reference(csn);
				undo_redo->add_do_method(this, StringName("add_child"), csn);
				undo_redo->add_undo_method(this, StringName("remove_child"), csn);
				undo_redo->add_do_method(current_fsm_res.ptr(), StringName("add_state_res"), csr);
				undo_redo->add_undo_method(this, StringName("remove_state_res"), csr);
			}
			// 拷贝相关转换
			for (auto i = 0; i < selected_transition_res_list.size(); i++) {
				Ref<TransitionRes> tr = selected_transition_res_list[i];
				if (selected_state_res_list.has(tr->get_from_state_res()) && selected_state_res_list.has(tr->get_to_state_res())) {
					Ref<TransitionRes> ctr;
					ctr.instantiate();
					ctr->set_from_state_res(osr2csr[tr->get_from_state_res()]);
					ctr->set_to_state_res(osr2csr[tr->get_to_state_res()]);
					StringName from = ctr->get_from_state_res()->get("state_node").get("name");
					StringName to = ctr->get_to_state_res()->get("state_node").get("name");
					undo_redo->add_do_method(current_fsm_res.ptr(), StringName("add_transition_res"), ctr);
					undo_redo->add_do_method(this, StringName("connect_node"), from, 0, to, 0);
					undo_redo->add_undo_method(this, StringName("disconnect_node"), from, 0, to, 0);
					undo_redo->add_undo_method(current_fsm_res.ptr(), StringName("remove_transition_res"), ctr);
				}
			}
			// 取消选择
			undo_redo->add_do_method(this, StringName("___select_mamually"), csn_list);
			undo_redo->add_undo_method(this, StringName("___select_mamually"), get_selected_state_nodes());
			undo_redo->commit_action();
		} break;
		case ITEM_DELETE: {
			auto selected_state_res_list = ___get_selected_state_res_list();
			if (selected_state_res_list.size() > 0) {
				auto undo_redo = HfsmEditorPlugin::create_action("Delete States");
				// 移除状态
				auto selected_state_nodes = get_selected_state_nodes();
				for (auto i = 0; i < selected_state_nodes.size(); i++) {
					Ref<TransitionRes> tr = selected_state_nodes[i];
					if (selected_state_res_list.has(tr->get_from_state_res()) || selected_state_res_list.has(tr->get_to_state_res())) {
						StringName from = tr->get_from_state_res()->get("state_node").get("name");
						StringName to = tr->get_to_state_res()->get("state_node").get("name");
						undo_redo->add_do_method(this, StringName("disconnect_node"), from, 0, to, 0);
						undo_redo->add_do_method(current_fsm_res.ptr(), StringName("remove_transition_res"), tr);
						undo_redo->add_undo_method(current_fsm_res.ptr(), StringName("add_transition_res"), tr);
						undo_redo->add_undo_method(this, StringName("connect_node"), from, 0, to, 0);
					}
				}
				undo_redo->add_undo_method(this, StringName("___select_mamually"), selected_state_nodes);
				undo_redo->add_do_method(this, StringName("queue_redraw"));
				undo_redo->add_undo_method(this, StringName("queue_redraw"));
				undo_redo->commit_action();
			} else if (selected_transition_res_list.size() >= 0) {
				auto undo_redo = HfsmEditorPlugin::create_action("Delete State Transitions");
				for (auto i = 0; i < selected_transition_res_list.size(); i++) {
					Ref<TransitionRes> tr = selected_transition_res_list[i];
					StringName from = tr->get_from_state_res()->get("state_node").get("name");
					StringName to = tr->get_to_state_res()->get("state_node").get("name");
					undo_redo->add_do_method(this, StringName("disconnect_node"), from, 0, to, 0);
					undo_redo->add_do_method(current_fsm_res.ptr(), StringName("remove_transition_res"), tr);
					undo_redo->add_undo_method(current_fsm_res.ptr(), StringName("add_transition_res"), tr);
					undo_redo->add_undo_method(this, StringName("connect_node"), from, 0, to, 0);
					undo_redo->commit_action();
				}
			}
			break;
		}
		case ITEM_CONVERT_TO_FSM: {
			auto selected_state_res_list = ___get_selected_state_res_list();
			if (!__hovered_state_node || !selected_state_res_list.has(__hovered_state_node->state_res)) {
				return;
			}
			auto undo_redo = HfsmEditorPlugin::create_action("Convert To Sub-FSM");
			// 复制状态资源
			Ref<StateRes> duplicated_state_res = __hovered_state_node->state_res->duplicate(true);
			// 新的子状态机
			Ref<FsmRes> new_fsm_res;
			new_fsm_res.instantiate();
			new_fsm_res->set_nested_state_res(duplicated_state_res);
			// 复制的状态节点
			auto duplicated_state_node = ____create_state_node(duplicated_state_res);
			//
			auto hovered_state_res = __hovered_state_node->state_res;
			auto hovered_state_node_name = __hovered_state_node->get_name();
			auto duplicated_state_node_name = duplicated_state_node->get_name();
			// 处理转换指向
			auto current_tr_list = current_fsm_res->get_transition_res_list();
			for (auto i = 0; i < current_tr_list.size(); i++) {
				Ref<TransitionRes> tr = current_tr_list[i];
				StringName from = tr->get_from_state_res()->get("state_node").get("name");
				StringName to = tr->get_to_state_res()->get("state_node").get("name");
				// 一端为指定状态，另一端不在选中的状态中，处理指向
				if (tr->get_from_state_res() == __hovered_state_node->state_res && !selected_state_res_list.has(tr->get_to_state_res())) {
					undo_redo->add_do_method(this, StringName("disconnect_node"), from, 0, to, 0);
					undo_redo->add_do_property(tr.ptr(), "from_state_res", duplicated_state_res);
					undo_redo->add_do_method(this, StringName("call_deferred"), StringName("connect_node"), duplicated_state_node_name, 0, to, 0);

					undo_redo->add_do_method(this, StringName("disconnect_node"), duplicated_state_node_name, 0, to, 0);
					undo_redo->add_do_property(tr.ptr(), "from_state_res", hovered_state_res);
					undo_redo->add_do_method(this, StringName("call_deferred"), StringName("connect_node"), from, 0, to, 0);
				} else if (tr->get_to_state_res() == __hovered_state_node->state_res && !selected_state_res_list.has(tr->get_from_state_res())) {
					undo_redo->add_do_method(this, StringName("disconnect_node"), from, 0, to, 0);
					undo_redo->add_do_property(tr.ptr(), StringName("to_state_res"), duplicated_state_res);
					undo_redo->add_do_method(this, StringName("call_deferred"), StringName("connect_node"), from, 0, duplicated_state_node_name, 0);

					undo_redo->add_do_method(this, StringName("disconnect_node"), from, 0, duplicated_state_node_name, 0);
					undo_redo->add_do_property(tr.ptr(), "to_state_res", hovered_state_res);
					undo_redo->add_do_method(this, StringName("call_deferred"), StringName("connect_node"), from, 0, to, 0);
				} else if ((selected_state_res_list.has(tr->get_from_state_res()) && !selected_state_res_list.has(tr->get_to_state_res())) ||
						(selected_state_res_list.has(tr->get_to_state_res()) && !selected_state_res_list.has(tr->get_from_state_res()))) {
					// 一端为选中对象，另一端不在选中状态中，删除（以排除一端为指定状态的情况
					undo_redo->add_do_method(this, StringName("disconnect_node"), from, 0, to, 0);
					undo_redo->add_do_method(current_fsm_res.ptr(), StringName("remove_transition_res"), tr);

					undo_redo->add_undo_method(current_fsm_res.ptr(), StringName("add_transition_res"), tr);
					undo_redo->add_undo_method(this, StringName("call_deferred"), StringName("connect_node"), from, 0, to, 0);
				} else if (selected_state_res_list.has(tr->get_from_state_res()) && selected_state_res_list.has(tr->get_to_state_res())) {
					undo_redo->add_do_method(this, StringName("disconnect_node"), from, 0, to, 0);
					undo_redo->add_do_method(current_fsm_res.ptr(), StringName("remove_transition_res"), tr);
					undo_redo->add_do_method(new_fsm_res.ptr(), StringName("add_transition_res"), tr);

					undo_redo->add_undo_method(new_fsm_res.ptr(), StringName("remove_transition_res"), tr);
					undo_redo->add_undo_method(current_fsm_res.ptr(), StringName("add_transition_res"), tr);
					undo_redo->add_undo_method(this, StringName("call_deferred"), StringName("connect_node"), from, 0, to, 0);
				}
			}
			// 对复制节点的操作（被相关状态节的撤回操作所依赖， 需要提前
			undo_redo->add_undo_method(this, StringName("remove_child"), duplicated_state_node);
			undo_redo->add_undo_method(current_fsm_res.ptr(), StringName("remove_state_res"), duplicated_state_res);
			undo_redo->add_undo_property(duplicated_state_res.ptr(), "fsm_res", nullptr);
			undo_redo->add_undo_property(duplicated_state_res.ptr(), "nested", false);
			undo_redo->add_undo_property(hovered_state_res.ptr(), "type", hovered_state_res->get_type());
			undo_redo->add_undo_property(hovered_state_res.ptr(), "state_script", hovered_state_res->get_state_script());
			// 移动选中节点所处的状态机
			auto selected_state_nodes = get_selected_state_nodes();
			for (auto i = 0; i < selected_state_nodes.size(); i++) {
				auto sn = Object::cast_to<StateNode>(selected_state_nodes[i]);
				undo_redo->add_do_method(this, "remove_child", sn);
				undo_redo->add_do_method(current_fsm_res.ptr(), "remove_state_res", sn->state_res);
				undo_redo->add_do_method(new_fsm_res.ptr(), "add_state_res", sn->state_res);

				undo_redo->add_undo_method(new_fsm_res.ptr(), "remove_state_res", sn->state_res);
				undo_redo->add_undo_method(current_fsm_res.ptr(), "add_state_res", sn->state_res);
				undo_redo->add_undo_method(this, "add_child", sn);
				undo_redo->add_undo_reference(sn);
			}
			//
			undo_redo->add_do_property(hovered_state_res.ptr(), "state_script", nullptr);
			undo_redo->add_do_property(hovered_state_res.ptr(), "type", State::STATE_TYPE_ENTRY);
			undo_redo->add_do_property(duplicated_state_res.ptr(), "nested", true);
			undo_redo->add_do_property(duplicated_state_res.ptr(), "fsm_res", new_fsm_res);
			undo_redo->add_do_method(current_fsm_res.ptr(), "add_state_res", duplicated_state_res);
			undo_redo->add_do_method(this, "add_child", duplicated_state_node);
			undo_redo->add_do_reference(duplicated_state_node);

			undo_redo->add_do_method(this, "___select_mamually", TypedArray<StateNode>::make(duplicated_state_node));
			undo_redo->add_undo_method(this, "___select_mamually", selected_state_nodes);

			undo_redo->add_do_method(this, "queue_redraw");
			undo_redo->add_undo_method(this, "queue_redraw");
			undo_redo->commit_action();

		} break;
	}
}
void StateNodesEditor::__on_delete_nodes_request(const Array &nodes) { __on_popup_menu_id_pressed(ITEM_DELETE); }
void StateNodesEditor::__on_connection_request(const StringName &from, int from_slot, const StringName &to, int to_slot) {
	auto from_node = Object::cast_to<StateNode>(find_child(from, false, false));
	auto to_node = Object::cast_to<StateNode>(find_child(to, false, false));
	auto tr = __get_transition_res(from_node, to_node);
	if (tr.is_valid()) {
		return;
	}
	Ref<TransitionRes> new_tr;
	new_tr.instantiate();
	new_tr->set_from_state_res(from_node->state_res);
	new_tr->set_to_state_res(to_node->state_res);
	// undoredo
	auto undo_redo = HfsmEditorPlugin::create_action("Create State Transition");
	undo_redo->add_do_method(this, "connect_node", from, from_slot, to, to_slot);
	undo_redo->add_undo_method(this, "disconnect_node", from, from_slot, to, to_slot);
	undo_redo->add_do_method(current_fsm_res.ptr(), StringName("add_transition_res"), new_tr);
	undo_redo->add_undo_method(current_fsm_res.ptr(), StringName("remove_transition_res"), new_tr);

	auto new_transiion_res_list = TypedArray<TransitionRes>::make(new_tr);
	undo_redo->add_do_method(this, "__set_selected_state_name_list", TypedArray<StringName>());
	undo_redo->add_undo_method(this, "__set_selected_state_name_list", selected_state_name_list);
	undo_redo->add_do_method(this, "call_deferred", "__set_selected_transition_res_list", new_transiion_res_list);
	undo_redo->add_undo_method(this, "call_deferred", "__set_selected_transition_res_list", selected_transition_res_list);

	undo_redo->add_do_method(this, StringName("queue_redraw"));
	undo_redo->add_undo_method(this, StringName("queue_redraw"));
	undo_redo->commit_action();
}

void StateNodesEditor::__on_popup_request(Vector2 position) {
	menu->clear();
	menu->add_item(str_localize("Add State"), ITEM_ADD_STATE);
	menu->add_item(str_localize("Cut States"), ITEM_CUT_STATE);
	menu->add_item(str_localize("Copy States"), ITEM_COPY_STATES, Key(KEYCODE_COPY_STATES));
	menu->add_item(str_localize("Paste States"), ITEM_PASTE_STATES, Key(KEYCODE_PASTE_STATES));
	menu->add_item(str_localize("Duplicate States"), ITEM_DUPLICATE_STATES, Key(KEYCODE_DUPLICATE_STATES));
	menu->add_item(str_localize("Delete"), ITEM_DELETE, Key(KEYCODE_DELETE_STATES));
	menu->add_separator();
	menu->add_item(str_localize("Convert To Sub-FSM"), ITEM_CONVERT_TO_FSM);

	__hovered_state_node = ___get_top_state_node_which_hovered();
	if (__hovered_state_node) {
		menu->set_item_disabled(ITEM_ADD_STATE, true);
	}
	if (selected_state_name_list.size() <= 0) {
		menu->set_item_disabled(ITEM_CUT_STATE, true);
		menu->set_item_disabled(ITEM_COPY_STATES, true);
		menu->set_item_disabled(ITEM_DUPLICATE_STATES, true);
		if (selected_transition_res_list.size() <= 0) {
			menu->set_item_disabled(ITEM_DELETE, true);
		}
	}
	if (copied_state_res_list.size() <= 0) {
		menu->set_item_disabled(ITEM_PASTE_STATES, true);
	}
	menu->set_position(get_tree()->get_root()->get_mouse_position());
	menu->popup();

	if (__hovered_state_node && get_selected_state_nodes().has(__hovered_state_node)) {
		return;
	}
	menu->set_item_disabled(menu->get_item_index(ITEM_CONVERT_TO_FSM), true);
	__hovered_state_node = nullptr;
}

void StateNodesEditor::__on_create_btn_pressed() {
	Ref<StateRes> new_sr;
	new_sr.instantiate();
	new_sr->set_type(State::STATE_TYPE_ENTRY);
	auto new_sn = ____create_state_node(new_sr);
	auto undo_redo = HfsmEditorPlugin::create_action("Add State");
	undo_redo->add_do_method(this, "add_child", new_sn);
	undo_redo->add_do_reference(new_sn);
	undo_redo->add_undo_method(this, "remove_child", new_sn);
	undo_redo->add_do_method(this, "___select_mamually", TypedArray<StateNode>::make(new_sn));
	undo_redo->add_undo_method(this, "___select_mamually", TypedArray<StateNode>());
	undo_redo->add_do_method(current_fsm_res.ptr(), "add_state_res", new_sr);
	undo_redo->add_undo_method(current_fsm_res.ptr(), "remove_state_res", new_sr);
	undo_redo->add_do_method(mask_panel, "hide");
	undo_redo->add_undo_method(mask_panel, "show");
	undo_redo->commit_action();
}
void StateNodesEditor::__on_transition_res_updated() { queue_redraw(); }

void StateNodesEditor::__on_node_selected(Object *node) {
	auto sn = Object::cast_to<StateNode>(node);
	if (!sn) {
		return;
	}
	if (!selected_state_name_list.has(sn->state_res->get_state_name())) {
		selected_state_name_list.push_back(sn->state_res->get_state_name());
	}
	if (selected_state_name_list.size() == 1) {
		UtilityFunctions::print(sn->state_res);
		HfsmEditorPlugin::get_singleton()->get_editor_interface()->inspect_object(sn->state_res.ptr());
	}
	___deal_selection_action();
}
void StateNodesEditor::__on_node_deselected(Object *node) {
	auto sn = Object::cast_to<StateNode>(node);
	if (!sn) {
		return;
	}
	if (selected_state_name_list.has(sn->state_res->get_state_name())) {
		selected_state_name_list.push_back(sn->state_res->get_state_name());
	}
	___deal_selection_action();
}

StateNodesEditor::StateNodesEditor() = default;

StateNodesEditor *StateNodesEditor::create_state_nodes_edit(HBoxContainer *path_btn_container) {
	auto r = memnew(StateNodesEditor);
	r->path_button_container = path_btn_container;
	// TODO?
	r->font = HfsmEditorPlugin::get_singleton()->get_editor_interface()->get_base_control()->get_theme()->get_default_font();
	// 结构
	{
		r->menu = memnew(PopupMenu);
		r->add_child(r->menu);
		r->mask_panel = memnew(Panel);
		r->mask_panel->set_anchors_preset(LayoutPreset::PRESET_FULL_RECT);
		r->add_child(r->mask_panel);
		r->not_state_alert = memnew(Label);
		r->not_state_alert->set_horizontal_alignment(HORIZONTAL_ALIGNMENT_CENTER);
		r->not_state_alert->set_vertical_alignment(VERTICAL_ALIGNMENT_CENTER);
		r->not_state_alert->set_anchors_preset(LayoutPreset::PRESET_CENTER);
		r->mask_panel->add_child(r->not_state_alert);
		r->create_btn = memnew(Button);
		r->create_btn->set_anchors_preset(LayoutPreset::PRESET_CENTER);
		r->mask_panel->add_child(r->create_btn);
	}
	// 信号
	{
		r->menu->connect("id_pressed", Callable(r, "__on_popup_menu_id_pressed"));
		r->create_btn->connect("pressed", Callable(r, "__on_create_btn_pressed"));
		r->connect("delete_nodes_request", Callable(r, "__on_delete_nodes_request"));
		// // ======== HACK =========
		// r->connect("copy_nodes_request", Callable(r, "__on_copy_requested"));
		// r->connect("paste_nodes_request", Callable(r, "__on_paste_requested"));
		// r->connect("duplicate_nodes_request", Callable(r, "__on_duplicate_requested"));
		// // ======== HACK =========
		r->connect("copy_nodes_request",
				Callable(r, "__on_popup_menu_id_pressed").bindv(Array::make(ITEM_COPY_STATES)));
		r->connect("paste_nodes_request",
				Callable(r, "__on_popup_menu_id_pressed").bindv(Array::make(ITEM_PASTE_STATES)));
		r->connect(
				"duplicate_nodes_request",
				Callable(r, "__on_popup_menu_id_pressed").bindv(Array::make(ITEM_DUPLICATE_STATES)));

		r->connect("connection_request", Callable(r, "__on_connection_request"));
		r->connect("popup_request", Callable(r, "__on_popup_request"));

		r->connect("node_selected", Callable(r, "__on_node_selected"));
		r->connect("node_deselected", Callable(r, "__on_node_deselected"));
	}
	// 参数
	r->set_v_size_flags(SIZE_EXPAND_FILL);
	r->add_valid_connection_type(StateNode::OUT_TYPE, StateNode::IN_TYPE);
	// r->_undo_redo = HfsmEditorPlugin::get_singleton()->get_undo_redo();
	// TODO?
	r->activity_color = HfsmEditorPlugin::get_singleton()->get_editor_interface()->get_base_control()->get_theme_color("activity", "GraphEdit");
	return r;
}

void StateNodesEditor::_ready() { set_process(false); }
void StateNodesEditor::edit_fsm_res(const Ref<FsmRes> &fsm_res) {
	auto undo_redo = HfsmEditorPlugin::create_action("Edit Sub-FSM");
	// 处理画面显示
	// 断连接
	auto conn_list = get_connection_list();
	for (auto i = 0; i < conn_list.size(); i++) {
		Dictionary conn = conn_list[i];
		undo_redo->add_do_method(this, "disconnect_node", conn["from"], conn["from_port"], conn["to"], conn["to_port"]);
		undo_redo->add_undo_method(this, "call_deferred", "connect_node", conn["from"], conn["from_port"], conn["to"], conn["to_port"]);
	}

	// 移除状态
	for (auto i = 0; i < get_child_count(); i++) {
		auto sn = Object::cast_to<StateNode>(get_child(i));
		if (sn) {
			undo_redo->add_do_method(this, "remove_child", sn);
			undo_redo->add_undo_method(this, "add_child", sn);
			undo_redo->add_undo_reference(sn);
		}
	}

	// 构建目标的状态机
	undo_redo->add_do_method(this, "__set_current_fsm_res", fsm_res);
	undo_redo->add_undo_method(this, "__set_current_fsm_res", current_fsm_res);
	//  路径按钮处理
	//  清除路径列表
	auto children = path_button_container->get_children(true);
	UtilityFunctions::print(children);
	for (auto i = 0; i < children.size(); i++) {
		auto btn = Object::cast_to<Button>(children[i]);
		if (btn) {
			undo_redo->add_do_method(path_button_container, "remove_child", btn);
			undo_redo->add_undo_method(path_button_container, "add_child", btn);
			undo_redo->add_undo_reference(btn);
		}
	}
	if (fsm_res.is_valid()) {
		// 处理路径按钮
		TypedArray<Button> path_btn_list;
		Ref<FsmRes> to_fsm_res = fsm_res->get_nested_state_res();
		Ref<StateRes> nsr = fsm_res->get_nested_state_res();
		while (nsr.is_valid()) {
			auto btn = memnew(Button);
			btn->set_text(nsr->get_state_name());
			// 暂时不能使用 Callable::bind()
			btn->set_meta("fsm_res", to_fsm_res);
			// btn->connect("pressed", Callable(this, "__on_edit_fsm_res_requeted"));
			// 暂时不能使用 Callable::bind()
			btn->connect("pressed",
					Callable(this, "edit_fsm_res").bindv(Array::make(to_fsm_res)));
			path_btn_list.push_back(btn);

			to_fsm_res = HfsmEditorPlugin::get_singleton()->get_hfsm_editor()->get_nested_fsm_res(nsr);
			nsr = to_fsm_res->get_nested_state_res();
		}
		auto root_btn = memnew(Button);
		root_btn->set_text("root");
		// 暂时不能使用 Callable::bind()
		root_btn->set_meta("fsm_res", to_fsm_res);
		// root_btn->connect("pressed", Callable(this, "__on_edit_fsm_res_requeted"));
		// 暂时不能使用 Callable::bind()
		root_btn->connect("pressed",
				Callable(this, "edit_fsm_res").bindv(Array::make(to_fsm_res)));
		path_btn_list.push_back(root_btn);
		// 末尾按钮不可按
		path_btn_list.front().set("disabled", true);
		// 按顺序添加
		while (!path_btn_list.is_empty()) {
			auto end = Object::cast_to<Button>(path_btn_list.pop_back());
			undo_redo->add_do_method(path_button_container, "add_child", end);
			undo_redo->add_undo_method(path_button_container, "remove_child", end);
			undo_redo->add_do_reference(end);
		}
		// 没有状态
		undo_redo->add_do_method(this, "__check_empty_fsm_res_or_not", fsm_res);
		undo_redo->add_undo_method(this, "__check_empty_fsm_res_or_not", current_fsm_res);
		// 新建并添加节点
		auto state_res_list = fsm_res->get_state_res_list();
		for (auto i = 0; i < state_res_list.size(); i++) {
			Ref<StateRes> sr = state_res_list[i];
			auto old_state_node = sr->get("state_node");
			auto sn = ____create_state_node(sr);
			undo_redo->add_do_method(sr.ptr(), "set", "state_node", sn);
			undo_redo->add_undo_method(sr.ptr(), "set", "state_node", old_state_node);
			undo_redo->add_do_method(this, "add_child", sn);
			undo_redo->add_undo_method(this, "remove_child", sn);
			undo_redo->add_do_reference(sn);
		}

		// 连接
		auto transition_res_list = fsm_res->get_transition_res_list();
		for (auto i = 0; i < transition_res_list.size(); i++) {
			Ref<TransitionRes> tr = transition_res_list[i];
			StringName from = tr->get_from_state_res()->get("state_node").get("name");
			StringName to = tr->get_to_state_res()->get("state_node").get("name");
			undo_redo->add_do_method(this, "call_deferred", "connect_node", from, 0, to, 0);
			undo_redo->add_undo_method(this, "disconnect_node", from, 0, to, 0);
		}
	}

	undo_redo->commit_action();
}

void StateNodesEditor::_process(real_t delta) {
	// 只处理左键松开时的选择
	if (!Input::get_singleton()->is_mouse_button_pressed(MOUSE_BUTTON_LEFT)) {
		if (selected_state_name_list.size() != __bakcup_selected_state_name_list.size()) {
			__undo_redo_select_nodes();
		} else {
			for (auto i = 0; i < selected_state_name_list.size(); i++) {
				StringName state_name = selected_state_name_list[i];
				if (!__bakcup_selected_state_name_list.has(state_name)) {
					__undo_redo_select_nodes();
					break;
				}
			}
		}
		// 释放
		_left_pressing = false;
		set_process(false);
	}
}

void StateNodesEditor::_gui_input(const Ref<InputEvent> &event) {
	static bool to_disconnect;
	auto mouse_btn_event = Object::cast_to<InputEventMouseButton>(event.ptr());
	if (mouse_btn_event) {
		// 删除转换操作
		if (mouse_btn_event->get_button_index() == MOUSE_BUTTON_MIDDLE) {
			if (mouse_btn_event->is_pressed()) {
				if (mouse_btn_event->is_alt_pressed() && !to_disconnect) {
					to_disconnect = true;
					_disconnect_line.resize(2);
					_disconnect_line[0] = mouse_btn_event->get_position();
					_disconnect_line[1] = mouse_btn_event->get_position();
				}
			} else if (mouse_btn_event->is_alt_pressed() && to_disconnect) {
				to_disconnect = false;
				__try_disconnect(mouse_btn_event->get_position(), _disconnect_line[0]);
				_disconnect_line.resize(0);
				queue_redraw();
			}
		} else {
			to_disconnect = false;
		}
		// 左键
		if (mouse_btn_event->get_button_index() == MOUSE_BUTTON_LEFT) {
			if (mouse_btn_event->is_double_click()) {
				if (mouse_btn_event->is_alt_pressed()) {
					return;
				}
				// 双击选择转换
				auto selected_tr_list = __try_select_transitions_at_pos(mouse_btn_event->get_position());
				// undoredo
				auto deal_selected_transition = [this, selected_tr_list]() -> void {
					auto undo_redo = HfsmEditorPlugin::create_action("Select State Transitions");
					undo_redo->add_do_method(this, StringName("__set_selected_state_name_list"), TypedArray<StringName>());
					undo_redo->add_do_method(this, StringName("__set_selected_transition_res_list"), selected_tr_list);
					undo_redo->add_undo_method(this, StringName("__set_selected_transition_res_list"), selected_transition_res_list);
					undo_redo->add_undo_method(this, StringName("__set_selected_state_name_list"), selected_state_name_list);
					undo_redo->commit_action();
				};

				if (selected_tr_list.size() != selected_transition_res_list.size()) {
					deal_selected_transition();
				} else {
					for (auto i = 0; i < selected_tr_list.size(); i++) {
						Ref<TransitionRes> tr = selected_tr_list[i];
						if (tr.is_valid() && !selected_transition_res_list.has(tr)) {
							deal_selected_transition();
							return;
						}
					}
				}
			} else {
				if (mouse_btn_event->is_alt_pressed()) {
					return;
				}
				// 取消选择
				if (!mouse_btn_event->is_pressed()) {
					if (selected_transition_res_list.size() > 0 || selected_state_name_list.size() > 0) {
						auto selected_tr_list = __try_select_transitions_at_pos(mouse_btn_event->get_position());
						if (selected_tr_list.size() == 0 && !___get_top_state_node_which_hovered()) {
							auto undo_redo = HfsmEditorPlugin::create_action("Deselect");
							undo_redo->add_do_method(this, StringName("__set_selected_transition_res_list"), selected_tr_list);
							undo_redo->add_do_method(this, StringName("__set_selected_state_name_list"), TypedArray<StringName>());
							undo_redo->add_undo_method(this, StringName("__set_selected_transition_res_list"), selected_transition_res_list);
							undo_redo->add_undo_method(this, StringName("__set_selected_state_name_list"), selected_state_name_list);
							undo_redo->commit_action();
						}
					}
				}
			}
		}
	}
	auto mouse_motion_event = Object::cast_to<InputEventMouseMotion>(event.ptr());
	if (mouse_motion_event && to_disconnect) {
		// 删除线
		_disconnect_line[1] = mouse_motion_event->get_position();
		queue_redraw();
	}
}

bool StateNodesEditor::_is_in_input_hotzone(Object *in_node, int64_t in_port, const Vector2 &mouse_position) { return __is_node_hotzone(in_node, in_port, mouse_position); }
bool StateNodesEditor::_is_in_output_hotzone(Object *in_node, int64_t in_port, const Vector2 &mouse_position) { return __is_node_hotzone(in_node, in_port, mouse_position); }
PackedVector2Array StateNodesEditor::_get_connection_line(const Vector2 &from, const Vector2 &to) const {
	StateNode *from_node = nullptr;
	StateNode *to_node = nullptr;
	auto zoom = static_cast<float>(get_zoom());
	for (auto i = 0; i < get_child_count(); i++) {
		auto node = Object::cast_to<StateNode>(get_child(i));
		if (!node) {
			continue;
		}
		auto from_slot_pos = node->get_connection_output_position(0) + node->get_position_offset() * zoom;
		auto to_slot_pos = node->get_connection_input_position(0) + node->get_position_offset() * zoom;
		from_slot_pos /= zoom;
		to_slot_pos /= zoom;
		// from
		if (!from_node) {
			if (from_slot_pos.is_equal_approx(from)) {
				from_node = node;
			} else {
				from_slot_pos = node->get_connection_output_position(0) + node->get_position();
				from_slot_pos /= zoom;
				if (from_slot_pos.is_equal_approx(from)) {
					from_node = node;
				}
			}
		}
		// to
		if (node != from_node && !to_node) {
			if (to_slot_pos.is_equal_approx(to)) {
				to_node = node;
			} else {
				to_slot_pos = node->get_connection_input_position(0) + node->get_position();
				to_slot_pos /= zoom;
				if (to_slot_pos.is_equal_approx(to)) {
					to_node = node;
				}
			}
		}
		if (from_node && to_node) {
			break;
		}
	}
	// 计算
	auto angle = from.angle_to_point(to);
	PackedVector2Array ret;
	ret.resize(2);
	if (from_node) {
		ret[0] = from + (Vector2(-1, 0) * from_node->get_size().x * 0.5f + Vector2(0, -1).rotated(angle) * CONN_POS_OFFSET * 0.5f);
	} else {
		ret[0] = from;
	}
	if (to_node) {
		ret[1] = to + (Vector2(1, 0) * to_node->get_size().x * 0.5f + Vector2(0, -1).rotated(angle) * CONN_POS_OFFSET * 0.5f);
	} else {
		ret[1] = to;
	}
	return ret;
}

void StateNodesEditor::_draw() {
	static bool deferring = false;
	auto zoom = static_cast<float>(get_zoom());
	for (auto i = 0; i < get_child_count(); i++) {
		auto node = Object::cast_to<StateNode>(get_child(i));
		if (!node) {
			continue;
		}
		auto rect = node->get_rect();
		rect.set_size(rect.get_size() * zoom);
		auto end = rect.get_end();
		auto pos = end - SCALE_DRAGGER_SIZE * zoom;
		auto dragger_rect = Rect2(pos, SCALE_DRAGGER_SIZE * zoom);
		rect = rect.grow_side(SIDE_TOP, -MOVE_ZONE_HIGHT * zoom);
		// ==测试显示==
		// draw_rect(rect, Color::named("red"), false, 5);
		// draw_rect(dragger_rect, Color::named("green"), false, 5);
		// ==测试显示==
	}
	Array dealed_tr_list;
	auto conn_list = get_connection_list();
	for (auto i = 0; i < conn_list.size(); i++) {
		Dictionary conn = conn_list[i];
		auto from = get_node<StateNode>({ conn["from"].operator godot::StringName() });
		auto to = get_node<StateNode>({ conn["to"].operator godot::StringName() });
		if (!from || !to) {
			continue;
		}
		// 正向
		auto tr = __get_transition_res(from, to);
		// 异常
		if (!tr.is_valid()) {
			UtilityFunctions::printerr("HFSM:: 异常 ，存在连接当不存在对应的转换流。");
			continue;
		}
		// 已处理过
		if (dealed_tr_list.has(tr)) {
			continue;
		}
		// 获取反向
		auto revers_tr = __get_transition_res(to, from);

		auto selected = selected_transition_res_list.has(tr);

		auto origin_line = __get_connection_line_with_zoom(from, to);
		Vector2 from_pos = origin_line[0];
		Vector2 to_pos = origin_line[1];
		auto angle = from_pos.angle_to_point(to_pos);
		auto clamped_zoom = static_cast<float>(CLAMP(get_zoom(), 0.5, 1));
		Color triangle_color = UtilityFunctions::lerp(from->OUT_COLOR, from->IN_COLOR, 0.5f);
		triangle_color = UtilityFunctions::lerp(triangle_color, activity_color, selected ? activity_amount : 0.0f);
		draw_set_transform((from_pos + to_pos) * 0.5f, angle, Vector2(1, 1) * clamped_zoom);
		draw_colored_polygon(TRIANGLE_POINTS, triangle_color);
		set_connection_activity(conn["from"], conn["from_port"], conn["to"], conn["to_port"], selected ? activity_amount : 0.0);
		bool valid = false;
		auto texts = __get_transition_res_valid_and_texts(tr, valid);
		auto text_color = selected ? triangle_color : Color::named("white");
		auto line_count = texts.size();
		Vector2 top;
		auto test_char_size = font->get_string_size("测");
		auto char_high = test_char_size.y;
		// auto char_width = test_char_size.x;
		if (angle <= Math_PI / 2.0 && angle > -Math_PI / 2.0) {
			// 上方正向显示
			top = (static_cast<float>(line_count) * char_high) * Vector2(0, -1);
			draw_set_transform((from_pos + to_pos) / 2.0f, angle, Vector2(1, 1) * clamped_zoom);
		} else {
			// 上方反向显示
			top = (CLAMP(line_count - 1, 1.2, line_count) * char_high) * Vector2(0, 1);
			draw_set_transform((from_pos + to_pos) / 2.0f, angle + Math_PI, Vector2(1, 1) * clamped_zoom);
		}
		for (auto i = 0; i < line_count; i++) {
			String text = texts[i];
			auto string_size = font->get_string_size(text);
			draw_string(font, top + Vector2(-string_size.x / 2.0f, static_cast<float>(i) * string_size.y), text, HORIZONTAL_ALIGNMENT_LEFT, -1, 16, text_color);
		}
	}

	if (_disconnect_line.size() == 2) {
		draw_set_transform(Vector2(0, 0), 0.0, Vector2(1, 1));
		draw_line(_disconnect_line[0], _disconnect_line[1], Color::named("royal_blue"), 5, true);
	}
	if (!deferring) {
		deferring = true;
		call_deferred("queue_redraw");
	} else {
		deferring = false;
	}
}

String StateNodesEditor::__get_variable_expression_res_valid_and_text(const Ref<VariableExpressionRes> &ver, bool &r_valid) const {
	r_valid = false;
	auto vr = ver->get_variable_res();
	if (vr.is_valid()) {
		if (vr->get_type() != Variant::NIL) {
			auto get_op_text = [ver]() -> String {
				switch (ver->get_operator()) {
					case VariableExpressionRes::OP_EQUAL:
						return " == ";
					case VariableExpressionRes::OP_NOT_EQUAL:
						return " != ";
					case VariableExpressionRes::OP_GREATER:
						return " > ";
					case VariableExpressionRes::OP_GREATER_EQUAL:
						return " >= ";
					case VariableExpressionRes::OP_LESS:
						return " < ";
					case VariableExpressionRes::OP_LESS_EQUAL:
						return " <= ";
					default:
						return " invalid operator";
				}
			};
			if (ver->is_variable_as_value()) {
				if (auto vr = Object::cast_to<HFSMVariableRes>(ver->get_value())) {
					r_valid = Variant::can_convert(Variant::Type(vr->get_type()), Variant::Type(vr->get_type()));
					if (r_valid) {
						return String(vr->get_variable_name()) + get_op_text() + String(vr->get_variable_name());
					} else {
						return str_localize(R"XXX("value" can't convert to the type of "HFSMVariableRes".)XXX");
					}
				} else {
					return str_localize(R"XXX("value" is not a valid "HFSMVariableRes".)XXX");
				}
			} else {
				r_valid = Variant::can_convert(ver->get_value().get_type(), Variant::Type(vr->get_type()));
				if (r_valid) {
					String value_text = "";
					switch (vr->get_type()) {
						case Variant::BOOL:
							value_text = ver->get_value().operator bool() ? "true" : "false";
							break;
						case Variant::INT:
						case Variant::FLOAT:
							// value_text = itos(int64_t(_value));
							// break;
							value_text = rtos(real_t(ver->get_value()));
							break;
						case Variant::STRING:
							value_text = String("'" + String(ver->get_value()) + "'");
							break;
						default:
							break;
					}
					return String(vr->get_variable_name()) + get_op_text() + value_text;
				} else {
					return str_localize("\"value\" can't convert to the type "
										"of \"variable_res\".");
				}
			}
		} else {
			r_valid = true;
			String vrn = { vr->get_variable_name() };
			switch (ver->get_trigger_type()) {
				case VariableExpressionRes::TRIGGER_TYPE_NORMAL:
					return str_localize("Trigger: ") + vrn;
					break;
				case VariableExpressionRes::TRIGGER_TYPE_SOLO:
					return str_localize("Solo Trigger: ") + vrn;
					break;
				case VariableExpressionRes::TRIGGER_TYPE_UNION:
					return str_localize("Union Trigger: ") + vrn;
					break;
				default:
					r_valid = false;
					return str_localize("Invalid Trigger Type:") + vrn;
					break;
			}
		}
	} else {
		return str_localize("Has not valid 'variable_res'");
	}
}
List<String> StateNodesEditor::__get_transition_res_valid_and_texts(const Ref<TransitionRes> &transition_res, bool &r_valid) const {
	List<String> ret;
	r_valid = false;

	switch (transition_res->get_type()) {
		case TransitionRes::TRANSITION_TYPE_SCRIPT: {
			auto tr_script = transition_res->get_transition_script();
			if (tr_script.is_valid()) {
				r_valid = true;
				if (tr_script->is_class(GDScript::get_class_static()) &&
						tr_script->get_instance_base_type() != Transition::get_class_static()) {
					r_valid = false;
				}

				if (r_valid) {
					ret.push_back(
							// 第一行名称
							str_localize("Script: ") + (tr_script->get_path().is_empty() ? (String("Build in ") + tr_script->get_class()) : tr_script->get_path()));
					ret.push_back(
							// 第二行路径
							tr_script->get_path());
				} else {
					ret.push_back(str_localize("Script isn't extends from \"Transition\"."));
					ret.push_back(str_localize("You can use other type of script if this is intended."));
				}
			} else {
				ret.push_back(str_localize("Script is invalid!"));
			}
		} break;
		case TransitionRes::TRANSITION_TYPE_VARIABLE: {
			auto variable_expression_res_list = transition_res->get_variable_expression_res_list();
			if (variable_expression_res_list.size() > 0) {
				r_valid = true;
				ret.push_back(str_localize("HFSMVariable Expressions: ") + String(transition_res->is_and_mode() ? "AND" : "OR"));
				for (auto i = 0; i < variable_expression_res_list.size(); i++) {
					Ref<VariableExpressionRes> ver = variable_expression_res_list[i];
					if (ver.is_valid()) {
						ret.push_back(__get_variable_expression_res_valid_and_text(ver, r_valid));
						if (!r_valid) {
							break;
						}
					} else {
						r_valid = false;
						ret.push_back(str_localize("Invalid \"VariableExpressionRes\"."));

						break;
					}
				}
			} else {
				ret.push_back(str_localize("HFSMVariable Expressions: ") + str_localize("Have not valid HFSMVariable Expression."));
			}
		} break;
		case TransitionRes::TRANSITION_TYPE_EXPRESSION: {
			// 非运行时无法检测表达式合法性， 故只要表达式不为空就认为合法
			if (transition_res->get_expression_text().is_empty()) {
				ret.push_back(str_localize("Empty expression!"));
			} else {
				r_valid = true;
				ret.push_back(String("Expression: ") + transition_res->get_expression_text());
				ret.push_back(String("Comment: ") + transition_res->get_expression_comment());
			}
		} break;
		case TransitionRes::TRANSITION_TYPE_AUTO: {
			switch (transition_res->get_auto_mode()) {
				case TransitionRes::AUTO_TRANSIT_MODE_DELAY_TIMER: {
					r_valid = true;
					ret.push_back(str_localize("Auto: ") + str_localize("Delay ") + itos(static_cast<int64_t>(transition_res->get_auto_delay_msec())) + str_localize(" msec."));
				} break;
				case TransitionRes::AUTO_TRANSIT_MODE_FSM_EXIT: {
					r_valid = true;
					ret.push_back(str_localize("Auto: ") + str_localize("When sub Fsm exit."));
				} break;
				case TransitionRes::AUTO_TRANSIT_MODE_MANUAL: {
					r_valid = true;
					ret.push_back(str_localize("Auto: ") + str_localize("After calling \"manual_exit()\"."));
				} break;
				case TransitionRes::AUTO_TRANSIT_MODE_UPDATE_TIMES: {
					r_valid = true;
					ret.push_back(str_localize("Auto: ") + str_localize("After \"_update()\" being called ") + itos(transition_res->get_auto_times()) + str_localize(" times."));

				} break;
				case TransitionRes::AUTO_TRANSIT_MODE_PHYSICS_UPDATE_TIMES: {
					r_valid = true;
					ret.push_back(str_localize("Auto: ") + str_localize("After \"_physics_update()\" being called ") + itos(transition_res->get_auto_times()) + str_localize(" times."));
				}
				default:
					ret.push_back(String::utf8("不应发生: 非法自动转换类型。"));
					break;
			}
		} break;
		default:
			break;
	}
	return ret;
}

}; // namespace Hfsm
