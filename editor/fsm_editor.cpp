#include "fsm_editor.h"

#ifdef GDEXTENSION_BUILD
#include <godot_cpp/classes/editor_inspector.hpp>
#include <godot_cpp/classes/editor_interface.hpp>
#include <godot_cpp/classes/editor_settings.hpp>
#include <godot_cpp/classes/font.hpp>
#include <godot_cpp/classes/gd_script.hpp>
#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/classes/input_event_mouse_button.hpp>
#include <godot_cpp/classes/input_event_mouse_motion.hpp>
#include <godot_cpp/classes/method_tweener.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/theme.hpp>
#include <godot_cpp/classes/tween.hpp>

#ifdef DEV_ENABLED
#include <godot_cpp/templates/local_vector.hpp>
#endif // DEV_ENABLED

#else // GDEXTENSION_BUILD
#include <editor/editor_interface.h>
#include <editor/editor_settings.h>
#include <modules/gdscript/gdscript.h>
#include <scene/animation/tween.h>
#include <scene/gui/panel.h>
#include <scene/gui/view_panner.h>

#endif // GDEXTENSION_BUILD

#include "../src/fsm_res.h"
#include "../src/transition_res.h"
#include "../src/transitions/transition.h"

#include "../hfsm_global.h"
#include "hfsm_editor.h"
#include "hfsm_editor_plugin.h"
#include "state_node.h"

using namespace godot;
namespace Hfsm {

#define TRANSITION_SELECT_EXTENT 10.0f
#define CONN_POS_OFFSET 50.0f
#define MOVE_ZONE_HIGHT 30.0f

#define set_editor_inspector_signal_connected(p_connected)                                           \
	{                                                                                                \
		static const StringName s_edited_object_changed = "edited_object_changed";                   \
		auto inspector = HfsmEditorPlugin::get_singleton()->get_editor_interface()->get_inspector(); \
		auto cb = TCALLABLE(_disconnect_inspecting_transition_res);                                  \
                                                                                                     \
		bool connected = inspector->is_connected(s_edited_object_changed, cb);                       \
		if constexpr (p_connected) {                                                                 \
			if (connected) {                                                                         \
				inspector->disconnect(s_edited_object_changed, cb);                                  \
			}                                                                                        \
		}                                                                                            \
		if constexpr (!(p_connected)) {                                                              \
			if (!inspector) {                                                                        \
				inspector->connect(s_edited_object_changed, cb);                                     \
			}                                                                                        \
		}                                                                                            \
	}

#define TRIANGLE_TEXTURE_WIDTH 32

#ifdef IDE_TYPE_SAFE
#define ADD_DO_DEFERRED_CALL_METHOD(m_obj_ptr, m_method, ...)        \
	DECLTYPE_METHOD_RETURN_TYPE(m_obj_ptr, m_method, ##__VA_ARGS__); \
	undo_redo->add_do_method(m_obj_ptr, "call_deferred", #m_method, ##__VA_ARGS__)
#define ADD_UNDO_DEFERRED_CALL_METHOD(m_obj_ptr, m_method, ...)      \
	DECLTYPE_METHOD_RETURN_TYPE(m_obj_ptr, m_method, ##__VA_ARGS__); \
	undo_redo->add_undo_method(m_obj_ptr, "call_deferred", #m_method, ##__VA_ARGS__)
#else
#define ADD_DO_DEFERRED_CALL_METHOD(m_obj_ptr, m_method, ...) undo_redo->add_do_method(m_obj_ptr, "call_deferred", #m_method, ##__VA_ARGS__)
#define ADD_UNDO_DEFERRED_CALL_METHOD(m_obj_ptr, m_method, ...) undo_redo->add_undo_method(m_obj_ptr, "call_deferred", #m_method, ##__VA_ARGS__)
#endif // IDE_TYPE_SAFE

#define s_edit_fsm_requested "_edit_fsm_requested"

String FsmEditor::str_localize(const String &en_key) const { return HfsmEditorPlugin::str_localize(en_key); }
void FsmEditor::_bind_methods() {
	GDBIND_BEGIN(FsmEditor);

	GDBIND_METHOD(__queue_refresh);
	// UNDO REDO
	GDBIND_METHOD(__set_current_fsm_res);
	GDBIND_METHOD(__set_selected_transition_res_list);
	GDBIND_METHOD(__set_copied_state_res_list);
	GDBIND_METHOD(__set_copied_transition_list);
	GDBIND_METHOD(__set_selected_state_name_list);
	GDBIND_METHOD(__select_state_nodes);
	GDBIND_METHOD(__select_mamually, "val");
	GDBIND_METHOD(__set_blocking_redraw);
	// CALLBACKS
	GDBIND_CALBACK(_disconnect_inspecting_transition_res);
	GDBIND_CALBACK(_transition_res_updated);
	GDBIND_CALBACK(_popup_menu_id_pressed, "id");
	GDBIND_CALBACK(_delete_nodes_request, "nodes");
	GDBIND_CALBACK(_connection_request, "from", "from_slot", "to", "to_slot");
	GDBIND_CALBACK(_popup_request, "position");
	GDBIND_CALBACK(_node_selected, "node");
	GDBIND_CALBACK(_node_deselected, "node");
	GDBIND_CALBACK(_gui_input_internal, "input");
	GDBIND_CALBACK(_end_node_move);
	GDBIND_CALBACK(_draw_layer_draw);
	GDBIND_CALBACK(_edit_sub_fsm_requested);
	GDBIND_CALBACK(_state_node_reconnected_requested);

	GDBIND_CALBACK(_debug_tween_activity);

	ADD_SIGNAL(MethodInfo(s_edit_fsm_requested, PropertyInfo(Variant::OBJECT, "sub_fsm_res", PROPERTY_HINT_RESOURCE_TYPE, FsmRes::get_class_static())));
}

// ========== SetGet =========
void FsmEditor::__set_current_fsm_res(const Ref<FsmRes> &to_set) {
	current_fsm_res = to_set;
	mask_panel->set_visible(current_fsm_res.is_null());
}

void FsmEditor::__set_selected_state_name_list(const TypedArray<StringName> &p_to_set) {
	if (selected_state_name_list != p_to_set) {
		selected_state_name_list = p_to_set;
	}
	selected_transition_res_list.clear();
	Array conn_list = call("get_connection_list");
	for (auto i = 0; i < conn_list.size(); i++) {
		Dictionary conn = conn_list[i];
		auto from = _get_state_node({ StringName(conn["from"]) });
		auto to = _get_state_node({ StringName(conn["to"]) });
		if (from && to) {
			if (selected_state_name_list.has(from->get_state_res()->get_state_name()) && selected_state_name_list.has(to->get_state_res()->get_state_name())) {
				auto tr = get_transition_res(from, to);
				if (tr.is_valid()) {
					selected_transition_res_list.push_back(tr);
				}
			}
		}
	}
	__set_selected_transition_res_list(selected_transition_res_list);
}

TypedArray<StateNode> FsmEditor::get_selected_state_nodes() {
	TypedArray<StateNode> ret;
	for (auto i = 0; i < get_child_count(); i++) {
		auto node = cast_to<StateNode>(get_child(i));
		if (node && selected_state_name_list.has(node->get_state_res()->get_state_name())) {
			ERR_CONTINUE_EDMSG(!node->is_selected(), "Unbelivalble!");
			ret.push_back(node);
		}
	}
	return ret;
}

void FsmEditor::__set_selected_transition_res_list(const TypedArray<TransitionRes> &p_to_set) {
	_disconnect_inspecting_transition_res();
	set_editor_inspector_signal_connected(false);

	if (selected_transition_res_list != p_to_set) {
		selected_transition_res_list = p_to_set;
	}
	inspecting_transition_res = selected_transition_res_list.size() == 1 ? selected_transition_res_list[0] : Variant(); // 同时只能监视一个 TransitionRes
	if (inspecting_transition_res.is_valid()) {
		inspecting_transition_res->connect(s_changed, TCALLABLE(_transition_res_updated));
	}

	ERR_FAIL_COND(!HfsmEditorPlugin::get_singleton()->get_editor_interface());

	if (selected_state_name_list.size() != 1) {
		HfsmEditorPlugin::get_singleton()->get_editor_interface()->inspect_object(inspecting_transition_res.ptr());
		set_editor_inspector_signal_connected(true);
	}
}

void FsmEditor::__set_copied_transition_list(const TypedArray<TransitionRes> &p_to_set) {
	if (copied_transition_res_list != p_to_set) {
		copied_transition_res_list = p_to_set;
	}
}
void FsmEditor::__set_copied_state_res_list(const TypedArray<StateRes> &p_to_set) {
	if (copied_state_res_list != p_to_set) {
		copied_state_res_list = p_to_set;
	}
}

// ========功能=========

void FsmEditor::try_disconnect(const Vector2 &p_pos1, const Vector2 &p_pos2) {
	if (debug_mode) {
		return;
	}

	Array conn_list = call("get_connection_list");
	const Vector2 scaled_pos1 = p_pos1;
	const Vector2 scaled_pos2 = p_pos2;
	for (auto i = 0; i < conn_list.size(); i++) {
		Dictionary conn = conn_list[i];
		auto from = _get_state_node({ StringName(conn["from"]) });
		auto to = _get_state_node({ StringName(conn["to"]) });
		if (from && to) {
			auto scaled_line = get_connection_line_with_zoom(from, to);
			if (is_judge(scaled_pos1, scaled_pos2, scaled_line[0], scaled_line[1])) {
				delete_transition(conn["from"], conn["from_port"], conn["to"], conn["to_port"]);
			}
		}
	}
}

bool FsmEditor::is_judge(const Vector2 &p_apos1, const Vector2 &p_apos2, const Vector2 &p_bpos1, const Vector2 &p_bpos2) {
	// x 投影重叠
	if ((MAX(p_apos1.x, p_apos2.x) >= MIN(p_bpos1.x, p_bpos2.x)) && (MIN(p_apos1.x, p_apos2.x) <= MAX(p_bpos1.x, p_bpos2.x))) {
		// y 投影重叠
		if ((MAX(p_apos1.y, p_apos2.y) >= MIN(p_bpos1.y, p_bpos2.y)) && (MIN(p_apos1.y, p_apos2.y) <= MAX(p_bpos1.y, p_bpos2.y))) {
			// A 是否跨过 B
			if ((p_bpos1 - p_apos1).cross(p_apos2 - p_apos1) * (p_bpos2 - p_apos1).cross(p_apos2 - p_apos1) <= 0) {
				// B 是否跨过 A
				if ((p_apos1 - p_bpos1).cross(p_bpos2 - p_bpos1) * (p_apos2 - p_bpos1).cross(p_bpos2 - p_bpos1) <= 0) {
					return true;
				}
			}
		}
	}
	return false;
}

void FsmEditor::delete_transition(const StringName &p_from, int32_t p_from_slot, const StringName &p_to, int32_t p_to_slot) {
	if (debug_mode) {
		return;
	}

	auto from_node = cast_to<StateNode>(find_child(p_from, false, false));
	auto to_node = cast_to<StateNode>(find_child(p_to, false, false));
	auto tr = get_transition_res(from_node, to_node);
	if (tr.is_valid()) {
		HFSM_EDITOR_CREATE_ACTION("Delete State Transitions");
		ADD_DO_METHOD(this, disconnect_node, p_from, p_from_slot, p_to, p_to_slot);
		ADD_DO_METHOD(current_fsm_res.ptr(), remove_transition_res, tr);
		ADD_UNDO_METHOD(current_fsm_res.ptr(), add_transition_res, tr);
		ADD_UNDO_METHOD(this, connect_node, p_from, p_from_slot, p_to, p_to_slot);
		COMMIT_ACTION();
	}
}

TypedArray<TransitionRes> FsmEditor::try_select_transitions_at_pos(const Vector2 &p_pos) {
	TypedArray<TransitionRes> ret;
	float graph_zoom = get_zoom();
	Array conn_list = call("get_connection_list");
	for (auto i = 0; i < conn_list.size(); i++) {
		Dictionary conn = conn_list[i];
		const StringName from_name = conn["from"];
		const StringName to_name = conn["to"];
		auto from = _get_state_node({ StringName(conn["from"]) });
		auto to = _get_state_node({ StringName(conn["to"]) });
		auto scaled_line = get_connection_line_with_zoom(from, to);
		Vector2 scaled_from_pos = scaled_line[0];
		Vector2 scaled_to_pos = scaled_line[1];
		// 取 转换线 的垂直方向, 以 鼠标
		// 双击点为基准，向两边延申，取得测试线段的两端点
		auto verti_ab_extent = scaled_from_pos.direction_to(scaled_to_pos).rotated(Math_PI * 0.5f) * TRANSITION_SELECT_EXTENT;
		auto test_segment_p1 = (p_pos / graph_zoom + verti_ab_extent) * graph_zoom;
		auto test_segment_p2 = (p_pos / graph_zoom - verti_ab_extent) * graph_zoom;
		// 测试线段于转换线是否相交
		if (is_judge(test_segment_p1, test_segment_p2, scaled_from_pos, scaled_to_pos)) {
			// 相交， 在识别范围内
			auto tr = get_transition_res(from, to);
			if (tr.is_valid()) {
				ret.push_back(tr);
			}
		}
	}

	return ret;
}

Ref<TransitionRes> FsmEditor::get_transition_res(StateNode *p_from, StateNode *p_to) {
	auto tr_list = current_fsm_res->get_transition_res_list();
	for (auto i = 0; i < tr_list.size(); i++) {
		Ref<TransitionRes> tr = tr_list[i];
		if (tr.is_valid() && tr->get_from_state_res() == p_from->get_state_res() && tr->get_to_state_res() == p_to->get_state_res()) {
			return tr;
		}
	}
	return nullptr;
}

bool FsmEditor::is_node_hotzone(Object *p_in_node, int64_t p_in_port, const Vector2 &p_mouse_position) {
	if (debug_mode) {
		return false;
	}

	if (!Input::get_singleton()->is_key_pressed(KEY(SHIFT))) {
		return false;
	}
	auto graph_zoom = static_cast<float>(get_zoom());
	auto zoomed_pos = p_mouse_position * graph_zoom;
	auto graph_node = cast_to<StateNode>(p_in_node);
	if (!graph_node) {
		return false;
	}
	auto rect = graph_node->get_rect();
	rect.set_size(rect.get_size() * graph_zoom);
	auto end = rect.get_end();
	auto zoomed_size = SCALE_DRAGGER_SIZE * graph_zoom;
	auto pos = end - zoomed_size;
	auto dragger_rect = Rect2(pos, zoomed_size);
	rect = rect.grow_side(SIDE_TOP, -MOVE_ZONE_HIGHT * graph_zoom);
	return rect.has_point(zoomed_pos) && !dragger_rect.has_point(zoomed_pos);
}

void FsmEditor::__select_state_nodes(const TypedArray<StringName> &p_to_select_State_name_list) {
	__set_selected_state_name_list(p_to_select_State_name_list);
	for (auto i = 0; i < get_child_count(); i++) {
		if (auto node = cast_to<StateNode>(get_child(i))) {
			node->set_selected(selected_state_name_list.has(node->get_state_res()->get_state_name()));
		}
	}
}

StateNode *FsmEditor::create_state_node(const Ref<StateRes> &p_state_res, const Ref<FsmRes> &p_fsm_res) {
	auto ret = StateNode::create_state_node(p_state_res, p_fsm_res.is_null() ? current_fsm_res : p_fsm_res, debug_mode);
	ret->connect(SNAME(s_edit_fsm_requested), TCALLABLE(_edit_sub_fsm_requested));
	ret->connect(SNAME("_reconnected_requested"), TCALLABLE(_state_node_reconnected_requested));
	return ret;
}

StateNode *FsmEditor::get_top_state_node_which_hovered() {
	if (debug_mode) {
		return nullptr;
	}

	auto graph_zoom = static_cast<float>(get_zoom());
	for (int i = get_child_count() - 1; i >= 0; i--) {
		if (auto node = cast_to<StateNode>(get_child(i))) {
			auto rect = node->get_rect();
			rect.set_size(rect.get_size() * graph_zoom);
			if (rect.has_point(get_local_mouse_position() * graph_zoom)) {
				return node;
			}
		}
	}
	return nullptr;
}

TypedArray<StateRes> FsmEditor::get_selected_state_res_list() {
	TypedArray<StateRes> ret;
	auto seleted_state_nodes = get_selected_state_nodes();
	for (auto i = 0; i < seleted_state_nodes.size(); i++) {
		if (auto sn = cast_to<StateNode>(seleted_state_nodes[i])) {
			ret.push_back(sn->get_state_res());
		}
	}
	return ret;
}

void FsmEditor::__select_mamually(const TypedArray<StateNode> &p_target_nodes) {
	TypedArray<StringName> to_select_state_name_list;
	for (auto i = 0; i < get_child_count(); i++) {
		if (auto sn = cast_to<StateNode>(get_child(i))) {
			sn->set_selected(p_target_nodes.has(sn));
			if (sn->is_selected()) {
				to_select_state_name_list.push_back(sn->get_state_res()->get_state_name());
			}
		}
	}
	__set_selected_state_name_list(to_select_state_name_list.duplicate());
	bakcup_selected_state_name_list = to_select_state_name_list;
}

// ==================

void FsmEditor::_edit_sub_fsm_requested(const Ref<FsmRes> &p_sub_fsm_res) {
	emit_signal(SNAME(s_edit_fsm_requested), p_sub_fsm_res);
}

void FsmEditor::_state_node_reconnected_requested(const StringName &p_old_name, const StringName &p_new_name) {
	if (!get_node_or_null({ p_new_name })) {
		return;
	}
	TypedArray<Dictionary> conn_list = call(SNAME("get_connection_list"));
	for (auto i = 0; i < conn_list.size(); ++i) {
		Dictionary conn = conn_list[i];
		if (StringName(conn["from"]) == p_old_name) {
			disconnect_node(p_old_name, 0, conn["to"], 0);
			connect_node(p_new_name, 0, conn["to"], 0);
		} else if (StringName(conn["to"]) == p_old_name) {
			disconnect_node(conn["from"], 0, p_old_name, 0);
			connect_node(conn["from"], 0, p_new_name, 0);
		}
	}
}

void FsmEditor::_popup_menu_id_pressed(int32_t p_id) {
	if (debug_mode) {
		return;
	}

	switch (p_id) {
		case ITEM_ADD_STATE: {
			if (__hovered_state_node) {
				return;
			}
			Ref<StateRes> new_sr;
			new_sr.instantiate();
			new_sr->set_editor_offset((get_local_mouse_position() + get_scroll_ofs()) / get_zoom());
			if (current_fsm_res->get_state_res_list().is_empty()) {
				new_sr->set_type(State::STATE_TYPE_ENTRY);
			}
			auto new_sn = create_state_node(new_sr);

			HFSM_EDITOR_CREATE_ACTION("Add State");
			ADD_DO_REFERENCE(new_sn);
			ADD_DO_METHOD(this, add_child, new_sn);
			ADD_DO_METHOD(this, __select_mamually, make_arr<TypedArray<StateNode>>(new_sn));
			ADD_DO_METHOD(current_fsm_res.ptr(), add_state_res, new_sr);
			ADD_UNDO_METHOD(current_fsm_res.ptr(), remove_state_res, new_sr);
			ADD_UNDO_METHOD(this, __select_mamually, TypedArray<StateNode>());
			ADD_UNDO_METHOD(this, remove_child, new_sn);
			COMMIT_ACTION();
		} break;
		case ITEM_CUT_STATE: {
			if (selected_state_name_list.size() <= 0) {
				return;
			}
			HFSM_EDITOR_CREATE_ACTION("Cut State");
			ADD_DO_METHOD(this, __set_blocking_redraw, true);
			ADD_UNDO_METHOD(this, __set_blocking_redraw, true);
			TypedArray<StateRes> to_copied_state_res;
			TypedArray<StateNode> selected_state_nodes = get_selected_state_nodes();
			for (auto i = 0; i < selected_state_nodes.size(); i++) {
				auto node = cast_to<StateNode>(selected_state_nodes[i]);
				if (!node) {
					continue;
				}

				ADD_UNDO_REFERENCE(node);
				ADD_DO_METHOD(this, remove_child, node);
				ADD_DO_METHOD(current_fsm_res.ptr(), remove_state_res, node->get_state_res());
				ADD_UNDO_METHOD(current_fsm_res.ptr(), add_state_res, node->get_state_res());
				ADD_UNDO_METHOD(this, add_child, node);

				to_copied_state_res.push_back(node->get_state_res());
			}

			auto tr_list = current_fsm_res->get_transition_res_list();
			for (auto i = 0; i < tr_list.size(); i++) {
				Ref<TransitionRes> tr = tr_list[i];
				auto from_node = cast_to<StateNode>(tr->get_from_state_res()->get_state_node());
				auto to_node = cast_to<StateNode>(tr->get_from_state_res()->get_state_node());
				if (!from_node || !to_node || selected_state_name_list.has(tr->get_from_state_res()->get_state_name()) || selected_state_name_list.has(tr->get_to_state_res()->get_state_name())) {
					ADD_DO_METHOD(current_fsm_res.ptr(), remove_transition_res, tr);
					ADD_UNDO_METHOD(current_fsm_res.ptr(), add_transition_res, tr);
				}
			}

			ADD_DO_METHOD(this, __set_copied_transition_list, selected_transition_res_list.duplicate());
			ADD_UNDO_METHOD(this, __set_copied_transition_list, copied_transition_res_list);
			ADD_UNDO_METHOD(this, __select_mamually, selected_state_nodes);
			ADD_DO_METHOD(this, __set_copied_state_res_list, to_copied_state_res);
			ADD_UNDO_METHOD(this, __set_copied_state_res_list, copied_state_res_list);

			ADD_DO_METHOD(this, __set_blocking_redraw, false);
			ADD_UNDO_METHOD(this, __set_blocking_redraw, false);
			COMMIT_ACTION();
		} break;
		case ITEM_COPY_STATES: {
			if (selected_state_name_list.size() <= 0) {
				return;
			}
			TypedArray<StateRes> to_copied_state_res_list;
			TypedArray<StateNode> selected_state_node_list = get_selected_state_nodes();
			for (auto i = 0; i < selected_state_node_list.size(); i++) {
				to_copied_state_res_list.push_back(cast_to<StateNode>(selected_state_node_list[i])->get_state_res());
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

			HFSM_EDITOR_CREATE_ACTION("Copy States");
			ADD_DO_METHOD(this, __set_copied_state_res_list, to_copied_state_res_list);
			ADD_UNDO_METHOD(this, __set_copied_state_res_list, copied_state_res_list);
			ADD_DO_METHOD(this, __set_copied_transition_list, selected_transition_res_list);
			ADD_UNDO_METHOD(this, __set_copied_transition_list, copied_transition_res_list);
			COMMIT_ACTION();
		} break;
		case ITEM_PASTE_STATES: {
			if (copied_state_res_list.size() <= 0) {
				return;
			}
			HFSM_EDITOR_CREATE_ACTION("Paste States");
			ADD_DO_METHOD(this, __set_blocking_redraw, true);
			ADD_UNDO_METHOD(this, __set_blocking_redraw, true);
			HashMap<Ref<StateRes>, Ref<StateRes>> osr2csr;
			// 计算中心
			Vector2 center;
			for (auto i = 0; i < copied_state_res_list.size(); i++) {
				Ref<StateRes> state_res = copied_state_res_list[i];
				center += state_res->get_editor_offset();
			}
			center /= static_cast<float>(copied_state_res_list.size());
			auto mouse_offset = (get_local_mouse_position() + get_scroll_ofs()) / static_cast<float>(get_zoom());
			// 计算偏移
			auto offset = center - mouse_offset;
			// 复制
			for (auto i = 0; i < copied_state_res_list.size(); i++) {
				Ref<StateRes> sr = copied_state_res_list[i];
				Ref<StateRes> csr = sr->duplicate(true);
				csr->set_editor_offset(csr->get_editor_offset() - offset);
				auto csn = create_state_node(csr);
				osr2csr.insert(sr, csr);
			}
			// 添加
			TypedArray<StateNode> copied_state_ndoes;
			for (const auto &kv : osr2csr) {
				auto csn = cast_to<StateNode>(kv.value->get_state_node());
				copied_state_ndoes.push_back(csn);

				ADD_DO_REFERENCE(csn);
				ADD_DO_METHOD(this, add_child, csn);
				ADD_DO_METHOD(current_fsm_res.ptr(), add_state_res, csn->get_state_res());
				ADD_UNDO_METHOD(current_fsm_res.ptr(), remove_state_res, csn->get_state_res());
				ADD_UNDO_METHOD(this, remove_child, csn);
			}
			// 拷贝相关转换
			for (auto i = 0; i < copied_transition_res_list.size(); i++) {
				Ref<TransitionRes> tr = copied_transition_res_list[i];
				if (copied_state_res_list.has(tr->get_from_state_res()) && copied_state_res_list.has(tr->get_to_state_res())) {
					Ref<TransitionRes> ctr;
					ctr.instantiate();
					ctr->set_from_state_res(osr2csr[tr->get_from_state_res()]);
					ctr->set_to_state_res(osr2csr[tr->get_to_state_res()]);
					StringName from = ctr->get_from_state_res()->get_state_node()->get_name();
					StringName to = ctr->get_to_state_res()->get_state_node()->get_name();

					ADD_DO_METHOD(current_fsm_res.ptr(), add_transition_res, ctr);
					ADD_DO_METHOD(this, connect_node, from, 0, to, 0);
					ADD_UNDO_METHOD(this, disconnect_node, from, 0, to, 0);
					ADD_UNDO_METHOD(current_fsm_res.ptr(), remove_transition_res, ctr);
				}
			}
			// 选中
			ADD_DO_METHOD(this, __set_blocking_redraw, false);
			ADD_UNDO_METHOD(this, __set_blocking_redraw, false);
			ADD_DO_METHOD(this, __select_mamually, copied_state_ndoes);
			ADD_UNDO_METHOD(this, __select_mamually, get_selected_state_nodes());
			COMMIT_ACTION();
		} break;
		case ITEM_DUPLICATE_STATES: {
			auto selected_state_res_list = get_selected_state_res_list();
			if (selected_state_res_list.size() <= 0) {
				return;
			}
			HFSM_EDITOR_CREATE_ACTION("Duplicate States");
			ADD_DO_METHOD(this, __set_blocking_redraw, true);
			ADD_UNDO_METHOD(this, __set_blocking_redraw, true);
			TypedArray<StateNode> csn_list;
			HashMap<Ref<StateRes>, Ref<StateRes>> osr2csr;
			for (auto i = 0; i < selected_state_res_list.size(); i++) {
				Ref<StateRes> sr = selected_state_res_list[i];
				Ref<StateRes> csr = sr->duplicate(true);
				csr->set_editor_offset(csr->get_editor_offset() + DUPLICATE_OFFSET);
				auto csn = create_state_node(csr);
				csn_list.push_back(csn);
				osr2csr.insert(sr, csr);

				ADD_DO_REFERENCE(csn);
				ADD_DO_METHOD(this, add_child, csn);
				ADD_DO_METHOD(current_fsm_res.ptr(), add_state_res, csr);
				ADD_UNDO_METHOD(current_fsm_res.ptr(), remove_state_res, csr);
				ADD_UNDO_METHOD(this, remove_child, csn);
			}
			// 拷贝相关转换
			for (auto i = 0; i < selected_transition_res_list.size(); i++) {
				Ref<TransitionRes> tr = selected_transition_res_list[i];
				if (selected_state_res_list.has(tr->get_from_state_res()) && selected_state_res_list.has(tr->get_to_state_res())) {
					Ref<TransitionRes> ctr;
					ctr.instantiate();
					ctr->set_from_state_res(osr2csr[tr->get_from_state_res()]);
					ctr->set_to_state_res(osr2csr[tr->get_to_state_res()]);
					StringName from = ctr->get_from_state_res()->get_state_node()->get_name();
					StringName to = ctr->get_to_state_res()->get_state_node()->get_name();

					ADD_DO_METHOD(current_fsm_res.ptr(), add_transition_res, ctr);
					ADD_DO_METHOD(this, connect_node, from, 0, to, 0);
					ADD_UNDO_METHOD(this, disconnect_node, from, 0, to, 0);
					ADD_UNDO_METHOD(current_fsm_res.ptr(), remove_transition_res, ctr);
				}
			}
			// 取消选择

			ADD_DO_METHOD(this, __set_blocking_redraw, false);
			ADD_UNDO_METHOD(this, __set_blocking_redraw, false);

			ADD_DO_METHOD(this, __select_mamually, csn_list);
			ADD_UNDO_METHOD(this, __select_mamually, get_selected_state_nodes());
			COMMIT_ACTION();
		} break;
		case ITEM_DELETE: {
			auto selected_state_res_list = get_selected_state_res_list();
			if (selected_state_res_list.size() > 0) {
				HFSM_EDITOR_CREATE_ACTION("Delete States");
				ADD_DO_METHOD(this, __set_blocking_redraw, true);
				ADD_UNDO_METHOD(this, __set_blocking_redraw, true);
				// 移除相关的转换流
				auto tr_list = current_fsm_res->get_transition_res_list();
				for (auto i = 0; i < tr_list.size(); ++i) {
					Ref<TransitionRes> tr = tr_list[i];
					if (selected_state_res_list.has(tr->get_from_state_res()) ||
							selected_state_res_list.has(tr->get_to_state_res())) {
						StringName from = tr->get_from_state_res()->get_state_node()->get_name();
						StringName to = tr->get_to_state_res()->get_state_node()->get_name();

						ADD_DO_METHOD(this, disconnect_node, from, 0, to, 0);
						ADD_DO_METHOD(current_fsm_res.ptr(), remove_transition_res, tr);
						ADD_UNDO_METHOD(current_fsm_res.ptr(), add_transition_res, tr);
						ADD_UNDO_METHOD(this, connect_node, from, 0, to, 0);
					}
				}
				// 移除状态
				for (auto i = 0; i < selected_state_res_list.size(); ++i) {
					Ref<StateRes> sr = selected_state_res_list[i];
					ADD_DO_REFERENCE(sr->get_state_node());
					ADD_DO_METHOD(this, remove_child, sr->get_state_node());
					ADD_DO_METHOD(current_fsm_res.ptr(), remove_state_res, sr);
					ADD_UNDO_METHOD(current_fsm_res.ptr(), add_state_res, sr);
					ADD_UNDO_METHOD(this, add_child, sr->get_state_node());
				}

				ADD_DO_METHOD(this, __set_blocking_redraw, false);
				ADD_UNDO_METHOD(this, __set_blocking_redraw, false);
				ADD_UNDO_METHOD(this, __select_mamually, get_selected_state_nodes());
				ADD_DO_METHOD(draw_layer, queue_redraw);
				ADD_UNDO_METHOD(draw_layer, queue_redraw);
				COMMIT_ACTION();
			} else if (selected_transition_res_list.size() >= 0) {
				HFSM_EDITOR_CREATE_ACTION("Delete State Transitions");
				for (auto i = 0; i < selected_transition_res_list.size(); i++) {
					Ref<TransitionRes> tr = selected_transition_res_list[i];
					StringName from = tr->get_from_state_res()->get_state_node()->get_name();
					StringName to = tr->get_to_state_res()->get_state_node()->get_name();

					ADD_DO_METHOD(this, disconnect_node, from, 0, to, 0);
					ADD_DO_METHOD(current_fsm_res.ptr(), remove_transition_res, tr);
					ADD_UNDO_METHOD(current_fsm_res.ptr(), add_transition_res, tr);
					ADD_UNDO_METHOD(this, connect_node, from, 0, to, 0);
					COMMIT_ACTION();
				}
			}
		} break;
		case ITEM_CONVERT_TO_FSM: {
			auto selected_state_res_list = get_selected_state_res_list();
			if (!__hovered_state_node || !selected_state_res_list.has(__hovered_state_node->get_state_res())) {
				return;
			}
			HFSM_EDITOR_CREATE_ACTION("Convert To Sub-FSM");
			ADD_DO_METHOD(this, __set_blocking_redraw, true);
			ADD_UNDO_METHOD(this, __set_blocking_redraw, true);
			// 复制状态资源
			Ref<StateRes> duplicated_state_res = __hovered_state_node->get_state_res()->duplicate(true);
			// 新的子状态机
			Ref<FsmRes> new_fsm_res;
			new_fsm_res.instantiate();
			new_fsm_res->set_nested_state_res(duplicated_state_res);
			// 复制的状态节点
			auto duplicated_state_node = create_state_node(duplicated_state_res);
			ADD_DO_REFERENCE(duplicated_state_node);
			//
			auto hovered_state_res = __hovered_state_node->get_state_res();
			auto hovered_state_node_name = __hovered_state_node->get_name();
			auto duplicated_state_node_name = duplicated_state_node->get_name();
			// 处理转换指向
			auto current_tr_list = current_fsm_res->get_transition_res_list();
			for (auto i = 0; i < current_tr_list.size(); i++) {
				Ref<TransitionRes> tr = current_tr_list[i];
				StringName from = tr->get_from_state_res()->get_state_node()->get_name();
				StringName to = tr->get_to_state_res()->get_state_node()->get_name();
				// 一端为指定状态，另一端不在选中的状态中，处理指向
				if (tr->get_from_state_res() == __hovered_state_node->get_state_res() && !selected_state_res_list.has(tr->get_to_state_res())) {
					ADD_DO_METHOD(this, disconnect_node, from, 0, to, 0);
					ADD_DO_METHOD(tr.ptr(), set_from_state_res, duplicated_state_res);
					ADD_DO_DEFERRED_CALL_METHOD(this, connect_node, duplicated_state_node_name, 0, to, 0);

					ADD_DO_METHOD(this, disconnect_node, duplicated_state_node_name, 0, to, 0);
					ADD_DO_METHOD(tr.ptr(), set_from_state_res, hovered_state_res);
					ADD_DO_DEFERRED_CALL_METHOD(this, connect_node, from, 0, to, 0);
				} else if (tr->get_to_state_res() == __hovered_state_node->get_state_res() && !selected_state_res_list.has(tr->get_from_state_res())) {
					ADD_DO_METHOD(this, disconnect_node, from, 0, to, 0);
					ADD_DO_METHOD(tr.ptr(), set_to_state_res, duplicated_state_res);
					ADD_DO_DEFERRED_CALL_METHOD(this, connect_node, from, 0, duplicated_state_node_name, 0);

					ADD_DO_METHOD(this, disconnect_node, from, 0, duplicated_state_node_name, 0);
					ADD_DO_METHOD(tr.ptr(), set_to_state_res, hovered_state_res);
					ADD_DO_DEFERRED_CALL_METHOD(this, connect_node, from, 0, to, 0);
				} else if ((selected_state_res_list.has(tr->get_from_state_res()) && !selected_state_res_list.has(tr->get_to_state_res())) ||
						(selected_state_res_list.has(tr->get_to_state_res()) && !selected_state_res_list.has(tr->get_from_state_res()))) {
					// 一端为选中对象，另一端不在选中状态中，删除（以排除一端为指定状态的情况
					ADD_DO_METHOD(this, disconnect_node, from, 0, to, 0);
					ADD_DO_METHOD(current_fsm_res.ptr(), remove_transition_res, tr);

					ADD_UNDO_METHOD(current_fsm_res.ptr(), add_transition_res, tr);
					ADD_UNDO_DEFERRED_CALL_METHOD(this, connect_node, from, 0, to, 0);
				} else if (selected_state_res_list.has(tr->get_from_state_res()) && selected_state_res_list.has(tr->get_to_state_res())) {
					ADD_DO_METHOD(this, disconnect_node, from, 0, to, 0);
					ADD_DO_METHOD(current_fsm_res.ptr(), remove_transition_res, tr);
					ADD_DO_METHOD(new_fsm_res.ptr(), add_transition_res, tr);

					ADD_UNDO_METHOD(new_fsm_res.ptr(), remove_transition_res, tr);
					ADD_UNDO_METHOD(current_fsm_res.ptr(), add_transition_res, tr);
					ADD_UNDO_DEFERRED_CALL_METHOD(this, connect_node, from, 0, to, 0);
				}
			}
			// 对复制节点的操作（被相关状态节的撤回操作所依赖， 需要提前
			ADD_UNDO_METHOD(this, remove_child, duplicated_state_node);
			ADD_UNDO_METHOD(current_fsm_res.ptr(), remove_state_res, duplicated_state_res);
			ADD_UNDO_METHOD(duplicated_state_res.ptr(), set_fsm_res, Ref<FsmRes>());
			ADD_UNDO_METHOD(duplicated_state_res.ptr(), set_nested, false);
			ADD_UNDO_METHOD(hovered_state_res.ptr(), set_type, hovered_state_res->get_type());
			ADD_UNDO_METHOD(hovered_state_res.ptr(), set_state_script, hovered_state_res->get_state_script());
			// 移动选中节点所处的状态机
			auto selected_state_nodes = get_selected_state_nodes();
			for (auto i = 0; i < selected_state_nodes.size(); i++) {
				auto sn = cast_to<StateNode>(selected_state_nodes[i]);
				ADD_DO_METHOD(this, remove_child, sn);
				ADD_DO_METHOD(current_fsm_res.ptr(), remove_state_res, sn->get_state_res());
				ADD_DO_METHOD(new_fsm_res.ptr(), add_state_res, sn->get_state_res());

				ADD_UNDO_METHOD(new_fsm_res.ptr(), remove_state_res, sn->get_state_res());
				ADD_UNDO_METHOD(current_fsm_res.ptr(), add_state_res, sn->get_state_res());
				ADD_UNDO_METHOD(this, add_child, sn);
				ADD_UNDO_REFERENCE(sn);
			}
			//
			ADD_DO_METHOD(hovered_state_res.ptr(), set_state_script, Ref<Script>());
			ADD_DO_METHOD(hovered_state_res.ptr(), set_type, State::STATE_TYPE_ENTRY);
			ADD_DO_METHOD(duplicated_state_res.ptr(), set_nested, true);
			ADD_DO_METHOD(duplicated_state_res.ptr(), set_fsm_res, new_fsm_res);
			ADD_DO_METHOD(current_fsm_res.ptr(), add_state_res, duplicated_state_res);
			ADD_DO_METHOD(this, add_child, duplicated_state_node);

			ADD_DO_METHOD(this, __select_mamually, make_arr<TypedArray<StateNode>>(duplicated_state_node));
			ADD_UNDO_METHOD(this, __select_mamually, selected_state_nodes);

			ADD_DO_METHOD(this, __set_blocking_redraw, false);
			ADD_UNDO_METHOD(this, __set_blocking_redraw, false);
			ADD_DO_METHOD(draw_layer, queue_redraw);
			ADD_UNDO_METHOD(draw_layer, queue_redraw);
			COMMIT_ACTION();

		} break;
	}
}

void FsmEditor::_delete_nodes_request(const Array &p_nodes) { _popup_menu_id_pressed(ITEM_DELETE); }

void FsmEditor::_connection_request(const StringName &p_from, int p_from_slot, const StringName &p_to, int p_to_slot) {
	if (debug_mode) {
		return;
	}

	auto from_node = cast_to<StateNode>(find_child(p_from, false, false));
	auto to_node = cast_to<StateNode>(find_child(p_to, false, false));
	auto tr = get_transition_res(from_node, to_node);
	if (tr.is_valid()) {
		return;
	}
	Ref<TransitionRes> new_tr;
	new_tr.instantiate();
	new_tr->set_from_state_res(from_node->get_state_res());
	new_tr->set_to_state_res(to_node->get_state_res());
	// undoredo
	HFSM_EDITOR_CREATE_ACTION("Create State Transition");
	ADD_DO_METHOD(this, connect_node, p_from, p_from_slot, p_to, p_to_slot);
	ADD_UNDO_METHOD(this, disconnect_node, p_from, p_from_slot, p_to, p_to_slot);
	ADD_DO_METHOD(current_fsm_res.ptr(), add_transition_res, new_tr);
	ADD_UNDO_METHOD(current_fsm_res.ptr(), remove_transition_res, new_tr);

	auto new_transiion_res_list = make_arr<TypedArray<TransitionRes>>(new_tr);
	ADD_DO_METHOD(this, __set_selected_state_name_list, TypedArray<StringName>());
	ADD_UNDO_METHOD(this, __set_selected_state_name_list, selected_state_name_list);

	ADD_DO_DEFERRED_CALL_METHOD(this, __set_selected_transition_res_list, new_transiion_res_list);
	ADD_UNDO_DEFERRED_CALL_METHOD(this, __set_selected_transition_res_list, selected_transition_res_list);

	ADD_DO_METHOD(draw_layer, queue_redraw);
	ADD_UNDO_METHOD(draw_layer, queue_redraw);
	COMMIT_ACTION();
}

void FsmEditor::_popup_request(const Vector2 &p_position) {
	if (debug_mode) {
		return;
	}

	menu->clear();
	menu->add_item(str_localize("Add State"), ITEM_ADD_STATE);
	menu->add_item(str_localize("Cut States"), ITEM_CUT_STATE);
	menu->add_item(str_localize("Copy States"), ITEM_COPY_STATES, Key(KEYCODE_COPY_STATES));
	menu->add_item(str_localize("Paste States"), ITEM_PASTE_STATES, Key(KEYCODE_PASTE_STATES));
	menu->add_item(str_localize("Duplicate States"), ITEM_DUPLICATE_STATES, Key(KEYCODE_DUPLICATE_STATES));
	menu->add_item(str_localize("Delete"), ITEM_DELETE, Key(KEYCODE_DELETE_STATES));
	menu->add_separator();
	menu->add_item(str_localize("Convert To Sub-FSM"), ITEM_CONVERT_TO_FSM);

	__hovered_state_node = get_top_state_node_which_hovered();
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

	if (!__hovered_state_node || !get_selected_state_nodes().has(__hovered_state_node)) {
		menu->set_item_disabled(menu->get_item_index(ITEM_CONVERT_TO_FSM), true);
		__hovered_state_node = nullptr;
	}

	menu->set_position(get_screen_position() + p_position);
	menu->popup();
}

void FsmEditor::_transition_res_updated() { draw_layer->queue_redraw(); }

void FsmEditor::_node_selected(Object *node) {
	auto sn = cast_to<StateNode>(node);
	if (!sn) {
		return;
	}

	const StringName state_name = sn->get_state_res()->get_state_name();
	if (!selected_state_name_list.has(state_name)) {
		selected_state_name_list.push_back(state_name);

		__set_selected_state_name_list(selected_state_name_list);
		selection_dirty = true;
	}

	if (selected_state_name_list.size() == 1) {
		HfsmEditorPlugin::get_singleton()->get_editor_interface()->inspect_object(sn->get_state_res().ptr());
	}
}

void FsmEditor::_node_deselected(Object *p_node) {
	auto sn = cast_to<StateNode>(p_node);
	if (!sn) {
		return;
	}

	const StringName state_name = sn->get_state_res()->get_state_name();
	if (selected_state_name_list.has(state_name)) {
		selected_state_name_list.erase(state_name);

		__set_selected_state_name_list(selected_state_name_list);
		selection_dirty = true;
	}
}

void FsmEditor::_disconnect_inspecting_transition_res() {
	if (inspecting_transition_res.is_valid()) {
		inspecting_transition_res->TDISCONNECT(s_changed, _transition_res_updated);
		inspecting_transition_res.unref();
	}
}

void FsmEditor::_end_node_move() {
	if (debug_mode) {
		return;
	}

	auto nodes = get_children();

	bool need_move = false;
	for (size_t i = 0; i < nodes.size(); i++) {
		if (auto node = cast_to<StateNode>(nodes[i])) {
			if (!node->get_state_res()->get_editor_offset().is_equal_approx(node->get_position_offset())) {
				need_move = true;
				break;
			}
		}
	}

	if (!need_move) {
		return;
	}

	HFSM_EDITOR_CREATE_ACTION("Move States");
	for (size_t i = 0; i < nodes.size(); i++) {
		if (auto node = cast_to<StateNode>(nodes[i])) {
			ADD_DO_METHOD(node->get_state_res().ptr(), set_editor_offset, node->get_position_offset());
			ADD_UNDO_METHOD(node->get_state_res().ptr(), set_editor_offset, node->get_state_res()->get_editor_offset());
		}
	}
	COMMIT_ACTION();
}

void FsmEditor::_gui_input_internal(const Ref<InputEvent> &p_event) {
	if (debug_mode) {
		return;
	}

	static bool to_disconnect = false;
	static const auto set_input_as_handled = [this]() { this->get_tree()->get_root()->set_input_as_handled(); };

	if (auto mouse_btn_event = Object::cast_to<InputEventMouseButton>(p_event.ptr())) {
		auto mouse_pos = draw_layer->get_local_mouse_position(); // mouse_btn_event->get_position(); //- draw_layer->get_position();
		switch (mouse_btn_event->get_button_index()) {
			case MOUSE_BUTTON(WHEEL_UP):
			case MOUSE_BUTTON(WHEEL_DOWN): {
				if (to_disconnect) {
					// Block zooming if trying disconnect.
					set_input_as_handled();
				}
			} break;
			case MOUSE_BUTTON(MIDDLE): {
				if (!mouse_btn_event->is_alt_pressed()) {
					break;
				}

				// 删除转换操作
				if (mouse_btn_event->is_pressed()) {
					if (!to_disconnect) {
						to_disconnect = true;
						disconnect_line.resize(2);
						disconnect_line.set(0, mouse_pos);
						disconnect_line.set(1, mouse_pos);
						set_input_as_handled();
					}
				} else if (to_disconnect) { // 松开
					to_disconnect = false;
					try_disconnect(mouse_pos, disconnect_line[0]);
					disconnect_line.resize(0);
					draw_layer->queue_redraw();
					set_input_as_handled();
				}
			} break;
			case MOUSE_BUTTON(LEFT): {
				if (mouse_btn_event->is_double_click()) {
					if (mouse_btn_event->is_alt_pressed()) {
						return;
					}
					// 双击选择转换
					auto selected_tr_list = try_select_transitions_at_pos(mouse_pos);

					const static auto undo_redo_selected_transition = [this](const TypedArray<TransitionRes> &P_selected_tr_list) -> void {
						HFSM_EDITOR_CREATE_ACTION("Select State Transitions");
						ADD_DO_METHOD(this, __set_selected_state_name_list, TypedArray<StringName>());
						ADD_DO_METHOD(this, __set_selected_transition_res_list, P_selected_tr_list);
						ADD_DO_METHOD(draw_layer, queue_redraw);
						ADD_UNDO_METHOD(this, __set_selected_transition_res_list, this->selected_transition_res_list);
						ADD_UNDO_METHOD(this, __set_selected_state_name_list, selected_state_name_list);
						ADD_UNDO_METHOD(draw_layer, queue_redraw);
						COMMIT_ACTION();
					};

					if (selected_tr_list.size() != selected_transition_res_list.size()) {
						undo_redo_selected_transition(selected_tr_list);
					} else {
						for (auto i = 0; i < selected_tr_list.size(); i++) {
							Ref<TransitionRes> tr = selected_tr_list[i];
							if (tr.is_valid() && !selected_transition_res_list.has(tr)) {
								undo_redo_selected_transition(selected_tr_list);
								break;
							}
						}
					}
					set_input_as_handled();
				} else if (!mouse_btn_event->is_pressed()) {
					if (selection_dirty) {
						const static auto undo_redo_select_nodes = [this]() {
							HFSM_EDITOR_CREATE_ACTION("Select States");
							ADD_DO_METHOD(this, __set_selected_transition_res_list, TypedArray<TransitionRes>());
							ADD_DO_METHOD(this, __select_state_nodes, this->selected_state_name_list);
							ADD_UNDO_METHOD(this, __select_state_nodes, this->bakcup_selected_state_name_list.duplicate());
							ADD_UNDO_METHOD(this, __set_selected_transition_res_list, this->selected_transition_res_list.duplicate());
							COMMIT_ACTION();
							this->bakcup_selected_state_name_list = this->selected_state_name_list.duplicate();
						};

						if (selected_state_name_list.size() != bakcup_selected_state_name_list.size()) {
							undo_redo_select_nodes();
						} else {
							for (auto i = 0; i < selected_state_name_list.size(); i++) {
								StringName state_name = selected_state_name_list[i];
								if (!bakcup_selected_state_name_list.has(state_name)) {
									undo_redo_select_nodes();
									break;
								}
							}
						}

						selection_dirty = false;
					} else if (!mouse_btn_event->is_alt_pressed()) {
						// 取消选择
						if (selected_transition_res_list.size() > 0 || selected_state_name_list.size() > 0) {
							auto selected_tr_list = try_select_transitions_at_pos(mouse_pos);
							if (selected_tr_list.size() == 0 && !get_top_state_node_which_hovered()) {
								HFSM_EDITOR_CREATE_ACTION("Deselect");
								ADD_DO_METHOD(this, __set_selected_transition_res_list, selected_tr_list);
								ADD_DO_METHOD(this, __set_selected_state_name_list, TypedArray<StringName>());
								ADD_DO_METHOD(draw_layer, queue_redraw);
								ADD_UNDO_METHOD(this, __set_selected_state_name_list, selected_state_name_list);
								ADD_UNDO_METHOD(this, __set_selected_transition_res_list, selected_transition_res_list);
								ADD_UNDO_METHOD(draw_layer, queue_redraw);
								COMMIT_ACTION();
							}
						}
					}
				}

				to_disconnect = false; // 中断断连操作
			} break;
			default: {
				to_disconnect = false; // 中断断连操作
			} break;
		}
	} else if (auto mouse_motion_event = Object::cast_to<InputEventMouseMotion>(p_event.ptr())) {
		if (to_disconnect) {
			// 删除
			disconnect_line.set(1, draw_layer->get_local_mouse_position());
			draw_layer->queue_redraw();
			set_input_as_handled();
		}
	}
}

void FsmEditor::_debug_tween_activity(float p_activity, const StringName &p_from, const StringName &p_to) {
	if (!is_node_connected(p_from, 0, p_to, 0)) {
		debug_activity_from = "";
		debug_activity_to = "";
		debug_activity = 0.0;
		return;
	}
	debug_activity = p_activity;
	queue_redraw();
}

#define get_offset(p_angle) (Vector2(0, -1).rotated(p_angle) * CONN_POS_OFFSET * 0.5f)

PackedVector2Array FsmEditor::get_connection_line_with_zoom(StateNode *p_from, StateNode *p_to) {
	const float graph_zoom = get_zoom();
	const auto from = (p_from->get_connection_output_position(0) + p_from->get_position_offset() * graph_zoom) / graph_zoom;
	const auto to = (p_to->get_connection_input_position(0) + p_to->get_position_offset() * graph_zoom) / graph_zoom;
	const auto angle = from.angle_to_point(to);
	return make_arr<PackedVector2Array>(
			(from + get_offset(angle)) * graph_zoom,
			(to + get_offset(angle)) * graph_zoom);
}

PackedVector2Array FsmEditor::get_connection_line_internal(const Vector2 &p_from, const Vector2 &p_to) const {
	const auto angle = p_from.angle_to_point(p_to);
	return make_arr<PackedVector2Array>(
			p_from + get_offset(angle),
			p_to + get_offset(angle));
}

void FsmEditor::_draw_layer_draw() {
	if (is_blocking_redraw()) {
		return;
	}

	if (connection_dirty) {
		Array conn_list = call("get_connection_list");
		for (auto i = 0; i < conn_list.size(); i++) {
			Dictionary conn = conn_list[i];
			auto from = _get_state_node({ StringName(conn["from"]) });
			auto to = _get_state_node({ StringName(conn["to"]) });
			if (!from || !to) {
				disconnect_node(conn["from"], conn["from_port"], conn["to"], conn["to_port"]);
			}
		}
		connection_dirty = false;
	}

	auto graph_zoom = static_cast<float>(get_zoom());
	for (auto i = 0; i < get_child_count(); i++) {
		if (auto node = cast_to<StateNode>(get_child(i))) {
			auto rect = node->get_rect();
			rect.set_size(rect.get_size() * graph_zoom);
			auto end = rect.get_end();
			auto pos = end - SCALE_DRAGGER_SIZE * graph_zoom;
			auto dragger_rect = Rect2(pos, SCALE_DRAGGER_SIZE * graph_zoom);
			rect = rect.grow_side(SIDE_TOP, -MOVE_ZONE_HIGHT * graph_zoom);
		}
	}

	const Color unactivated_triangle_color = StateNode::OUT_COLOR.lerp(StateNode::IN_COLOR, 0.5f);

	Array conn_list = call("get_connection_list");
	IF_DEV(
			LocalVector<Ref<TransitionRes>> dealed_tr_list;
			dealed_tr_list.reserve(conn_list.size());)
	for (auto i = 0; i < conn_list.size(); i++) {
		Dictionary conn = conn_list[i];
		StringName from_name = conn["from"];
		StringName to_name = conn["to"];

		auto from = _get_state_node({ StringName(conn["from"]) });
		auto to = _get_state_node({ StringName(conn["to"]) });
		if (!from || !to) {
			continue;
		}
		// 正向
		auto tr = get_transition_res(from, to);
		// 异常
		ERR_CONTINUE_MSG(tr.is_null(), "HFSM:: 异常 ，存在连接当不存在对应的转换流。");
		IF_DEV({
			ERR_CONTINUE_MSG(dealed_tr_list.find(tr) >= 0, "不同的链接指向同一个 TransitionRes? 这不可能");
			dealed_tr_list.push_back(tr);
		});
		// 获取反向
		auto revers_tr = get_transition_res(to, from);

		auto selected = selected_transition_res_list.has(tr);

		auto scaled_line = get_connection_line_with_zoom(from, to);
		auto center_pos = (scaled_line[0] + scaled_line[1]) * 0.5f;
		auto angle = scaled_line[0].angle_to_point(scaled_line[1]);
		auto clamped_scale = Vector2(1, 1) * float(CLAMP(get_zoom(), 0.5, 1));

		// 三角形
		Color triangle_color = unactivated_triangle_color;
		if (debug_mode) {
			if (debug_activity_from == from->get_name() && debug_activity_to == to->get_name()) {
				triangle_color = triangle_color.lerp(activity_color, debug_activity);
				set_connection_activity(from_name, 0, to_name, 0, debug_activity);
			} else {
				set_connection_activity(from_name, 0, to_name, 0, 0.0);
			}
		} else {
			triangle_color = selected ? activity_color : triangle_color;
			set_connection_activity(from_name, 0, to_name, 0, selected ? 1.0 : 0.0);
		}
		draw_layer->draw_set_transform(center_pos, angle, clamped_scale);
		draw_layer->draw_colored_polygon(TRIANGLE_POINTS, triangle_color);

		bool valid = false;
		auto texts = get_transition_res_valid_and_texts(tr, valid);
		Color text_color = Color::named("white");
		if (debug_mode) {
			if (debug_activity_from == from->get_name() && debug_activity_to == to->get_name()) {
				text_color = text_color.lerp(activity_color, debug_activity);
			}
		} else {
			if (selected) {
				text_color = triangle_color;
			}
		}
		auto line_count = texts.size();
		Vector2 top;
		auto test_char_size = font->get_string_size("测");
		auto char_high = test_char_size.y;

		// auto char_width = test_char_size.x;
		if (angle <= Math_PI * 0.5f && angle > -Math_PI * 0.5f) {
			// 上方正向显示
			top = (line_count * char_high) * Vector2(0, -1);
			draw_layer->draw_set_transform(center_pos, angle, clamped_scale);
		} else {
			// 上方反向显示
			top = (CLAMP(line_count - 1, 1.2, line_count) * char_high) * Vector2(0, 1);
			draw_layer->draw_set_transform(center_pos, angle + Math_PI, clamped_scale);
		}

		for (auto i = 0; i < line_count; i++) {
			String text = texts[i];
			auto string_size = font->get_string_size(text);
			draw_layer->draw_string(font, top + Vector2(-string_size.x / 2.0f, i * string_size.y), text, HORIZONTAL_ALIGNMENT_LEFT, -1, 16, text_color);
		}
	}

	if (disconnect_line.size() == 2) {
		draw_layer->draw_set_transform(Vector2(0, 0), 0.0, Vector2(1, 1));
		draw_layer->draw_line(disconnect_line[0], disconnect_line[1], Color::named("royal_blue"), 5, true);
	}

	static bool dirty = false;
	if (!dirty) {
		draw_layer->call_deferred(TNAMEOF(queue_redraw));
		dirty = true;
	} else {
		dirty = false;
	}
}

void FsmEditor::initialize() {
	set_name("FsmEditor");
	set_v_size_flags(SIZE_EXPAND_FILL);
	add_valid_connection_type(StateNode::OUT_TYPE, StateNode::IN_TYPE);
	connect("delete_nodes_request", TCALLABLE(_delete_nodes_request));
	connect("copy_nodes_request", TCALLABLE_BIND(_popup_menu_id_pressed, ITEM_COPY_STATES));
	connect("paste_nodes_request", TCALLABLE_BIND(_popup_menu_id_pressed, ITEM_PASTE_STATES));
	connect("duplicate_nodes_request", TCALLABLE_BIND(_popup_menu_id_pressed, ITEM_DUPLICATE_STATES));
	connect("connection_request", TCALLABLE(_connection_request));
	connect("popup_request", TCALLABLE(_popup_request));
	connect("node_selected", TCALLABLE(_node_selected));
	connect("node_deselected", TCALLABLE(_node_deselected));

	// Hack
	for (auto i = 0; i < get_child_count(true); ++i) {
		if (auto ctrl = cast_to<Control>(get_child(i, true))) {
			if (ctrl->get_name() == StringName("CLAYER")) {
				draw_layer = ctrl;
			}
		}
	}
	CRASH_COND(!draw_layer);
	draw_layer->set_mouse_filter(MOUSE_FILTER_PASS);

	menu = memnew(PopupMenu);
	menu->connect("id_pressed", TCALLABLE(_popup_menu_id_pressed));
	add_child(menu);

	mask_panel = memnew(Panel);
	mask_panel->set_anchors_preset(LayoutPreset::PRESET_FULL_RECT);
	add_child(mask_panel);

	mask_hint = memnew(Label);
	mask_hint->set_h_size_flags(SizeFlags::SIZE_EXPAND_FILL);
	mask_hint->set_v_size_flags(SizeFlags::SIZE_EXPAND_FILL);
	mask_hint->set_horizontal_alignment(HORIZONTAL_ALIGNMENT_CENTER);
	mask_hint->set_vertical_alignment(VERTICAL_ALIGNMENT_CENTER);
	mask_hint->set_anchors_preset(LayoutPreset::PRESET_FULL_RECT);
	mask_hint->set_position(Vector2(0, -50));
	mask_hint->set_text(str_localize("Please set up a FsmRes for selected HFSM node to start edit."));
	mask_panel->add_child(mask_hint);
}

FsmEditor *FsmEditor::create_fsm_editor(HBoxContainer *p_path_btn_container, bool p_debug_mode) {
	auto r = memnew(FsmEditor(p_debug_mode));
	r->initialize();
	return r;
}

Ref<FsmRes> FsmEditor::get_nested_fsm_res(const Ref<StateRes> &p_state_res, const Ref<FsmRes> &p_fsm_res) {
	if (p_fsm_res.is_valid()) {
		auto state_res_list = p_fsm_res->get_state_res_list();
		for (size_t i = 0; i < state_res_list.size(); i++) {
			Ref<StateRes> sr = state_res_list[i];
			if (sr == p_state_res) {
				return p_fsm_res;
			} else {
				if (sr->get_fsm_res().is_valid()) {
					auto r = get_nested_fsm_res(p_state_res, sr->get_fsm_res());
					if (r.is_valid()) {
						return r;
					}
				}
			}
		}
	}
	return nullptr;
}

void FsmEditor::edit_fsm_res(const Ref<FsmRes> &p_fsm_res, HBoxContainer *p_path_button_container, const Ref<FsmRes> &p_root_fsm_res) {
	if (debug_mode) {
		__set_blocking_redraw(true);
		// 处理画面显示
		// 断连接
		Array conn_list = call("get_connection_list");
		for (auto i = 0; i < conn_list.size(); i++) {
			Dictionary conn = conn_list[i];
			disconnect_node(conn["from"], conn["from_port"], conn["to"], conn["to_port"]);
		}

		// 移除状态
		for (auto i = 0; i < get_child_count(); i++) {
			if (auto sn = Object::cast_to<StateNode>(get_child(i))) {
				remove_child(sn);
				memdelete(sn);
			}
		}

		// 构建目标的状态机
		__set_current_fsm_res(p_fsm_res);

		//  路径按钮处理
		//  清除路径列表
		auto children = p_path_button_container->get_children(true);
		for (auto i = 0; i < children.size(); i++) {
			if (auto btn = Object::cast_to<Button>(children[i])) {
				p_path_button_container->remove_child(btn);
				memdelete(btn);
			}
		}

		if (p_fsm_res.is_valid()) {
			// 处理路径按钮
			List<Button *> path_btn_list;
			Ref<FsmRes> fr = p_fsm_res;
			Ref<StateRes> nsr = p_fsm_res->get_nested_state_res();
			while (nsr.is_valid()) {
				auto btn = memnew(Button);
				btn->set_text(nsr->get_state_name());
				btn->connect("pressed", TCALLABLE_BIND(edit_fsm_res, fr));

				path_btn_list.push_front(btn);

				fr = get_nested_fsm_res(nsr, p_root_fsm_res);
				nsr = fr->get_nested_state_res();
			}
			auto root_btn = memnew(Button);
			root_btn->set_text("root");
			root_btn->connect("pressed", TCALLABLE_BIND(edit_fsm_res, fr));
			path_btn_list.push_front(root_btn);
			// 末尾按钮不可按
			path_btn_list.back()->get()->set_disabled(true);
			// 按顺序添加
			while (!path_btn_list.is_empty()) {
				Button *front_btn = path_btn_list.front()->get();
				path_btn_list.pop_front();

				p_path_button_container->add_child(front_btn);
			}
			// 新建并添加节点
			auto state_res_list = p_fsm_res->get_state_res_list();
			for (auto i = 0; i < state_res_list.size(); i++) {
				Ref<StateRes> sr = state_res_list[i];
				auto old_state_node = sr->get_state_node();
				auto sn = create_state_node(sr);
				sr->set_state_node(sn);
				add_child(sn);
			}

			// 连接
			auto transition_res_list = p_fsm_res->get_transition_res_list();
			for (auto i = 0; i < transition_res_list.size(); i++) {
				Ref<TransitionRes> tr = transition_res_list[i];
				StringName from = tr->get_from_state_res()->get_state_node()->get_name();
				StringName to = tr->get_to_state_res()->get_state_node()->get_name();

				call_deferred(TNAMEOF(connect_node), from, 0, to, 0);
			}
		}

		__set_blocking_redraw(false);
		propagate_notification(NOTIFICATION_CHILD_ORDER_CHANGED);
	} else {
		if (current_fsm_res.is_null() && p_fsm_res.is_null()) {
			return;
		}

		HFSM_EDITOR_CREATE_ACTION("Edit Sub-FSM");
		ADD_DO_METHOD(this, __set_blocking_redraw, true);
		ADD_UNDO_METHOD(this, __set_blocking_redraw, true);
		// 处理画面显示
		// 断连接
		Array conn_list = call("get_connection_list");
		for (auto i = 0; i < conn_list.size(); i++) {
			Dictionary conn = conn_list[i];
			ADD_DO_METHOD(this, disconnect_node, conn["from"], conn["from_port"], conn["to"], conn["to_port"]);
			ADD_UNDO_DEFERRED_CALL_METHOD(this, connect_node, conn["from"], conn["from_port"], conn["to"], conn["to_port"]);
		}

		// 移除状态
		for (auto i = 0; i < get_child_count(); i++) {
			if (auto sn = Object::cast_to<StateNode>(get_child(i))) {
				ADD_DO_METHOD(this, remove_child, sn);
				ADD_UNDO_METHOD(this, add_child, sn);
				ADD_UNDO_REFERENCE(sn);
			}
		}

		// 构建目标的状态机
		ADD_DO_METHOD(this, __set_current_fsm_res, p_fsm_res);
		ADD_UNDO_METHOD(this, __set_current_fsm_res, current_fsm_res);

		//  路径按钮处理
		//  清除路径列表
		auto children = p_path_button_container->get_children(true);
		for (auto i = 0; i < children.size(); i++) {
			if (auto btn = Object::cast_to<Button>(children[i])) {
				ADD_DO_METHOD(p_path_button_container, remove_child, btn);
				ADD_UNDO_METHOD(p_path_button_container, add_child, btn);
				ADD_UNDO_REFERENCE(btn);
			}
		}

		if (p_fsm_res.is_valid()) {
			// 处理路径按钮
			List<Button *> path_btn_list;
			Ref<FsmRes> fr = p_fsm_res;
			Ref<StateRes> nsr = p_fsm_res->get_nested_state_res();
			while (nsr.is_valid()) {
				auto btn = memnew(Button);
				btn->set_text(nsr->get_state_name());
				btn->connect("pressed", TCALLABLE_BIND(edit_fsm_res, fr));

				path_btn_list.push_front(btn);

				fr = get_nested_fsm_res(nsr, p_root_fsm_res);
				nsr = fr->get_nested_state_res();
			}
			auto root_btn = memnew(Button);
			root_btn->set_text("root");
			root_btn->connect("pressed", TCALLABLE_BIND(edit_fsm_res, fr));
			path_btn_list.push_front(root_btn);
			// 末尾按钮不可按
			path_btn_list.back()->get()->set_disabled(true);
			// 按顺序添加
			while (!path_btn_list.is_empty()) {
				Button *front_btn = path_btn_list.front()->get();
				path_btn_list.pop_front();

				ADD_DO_METHOD(p_path_button_container, add_child, front_btn);
				ADD_UNDO_METHOD(p_path_button_container, remove_child, front_btn);
				ADD_DO_REFERENCE(front_btn);
			}
			// 新建并添加节点
			auto state_res_list = p_fsm_res->get_state_res_list();
			for (auto i = 0; i < state_res_list.size(); i++) {
				Ref<StateRes> sr = state_res_list[i];
				auto old_state_node = sr->get_state_node();
				auto sn = create_state_node(sr, p_fsm_res);
				ADD_DO_REFERENCE(sn);

				ADD_DO_METHOD(sr.ptr(), set_state_node, sn);
				ADD_UNDO_METHOD(sr.ptr(), set_state_node, old_state_node);
				ADD_DO_METHOD(this, add_child, sn);
				ADD_UNDO_METHOD(this, remove_child, sn);
			}

			// 连接
			auto transition_res_list = p_fsm_res->get_transition_res_list();
			for (auto i = 0; i < transition_res_list.size(); i++) {
				Ref<TransitionRes> tr = transition_res_list[i];
				StringName from = tr->get_from_state_res()->get_state_node()->get_name();
				StringName to = tr->get_to_state_res()->get_state_node()->get_name();

				ADD_DO_DEFERRED_CALL_METHOD(this, connect_node, from, 0, to, 0);
				ADD_UNDO_METHOD(this, disconnect_node, from, 0, to, 0);
			}
		}

		ADD_DO_METHOD(this, __set_blocking_redraw, false);
		ADD_UNDO_METHOD(this, __set_blocking_redraw, false);

		ADD_DO_METHOD(this, propagate_notification, NOTIFICATION_CHILD_ORDER_CHANGED);
		ADD_UNDO_METHOD(this, propagate_notification, NOTIFICATION_CHILD_ORDER_CHANGED);
		COMMIT_ACTION();
	}
}

String FsmEditor::get_variable_expression_res_valid_and_text(const Ref<VariableExpressionRes> &p_ver, bool &r_valid) const {
	r_valid = false;
	auto vr = p_ver->get_variable_res();
	if (vr.is_valid()) {
		if (vr->get_type() != Variant::NIL) {
			auto get_op_text = [p_ver]() -> String {
				switch (p_ver->get_comparator()) {
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
			if (p_ver->is_variable_as_value()) {
				if (auto vr = cast_to<HFSMVariableRes>(p_ver->get_value())) {
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
				r_valid = Variant::can_convert(p_ver->get_value().get_type(), Variant::Type(vr->get_type()));
				if (r_valid) {
					String value_text = "";
					switch (vr->get_type()) {
						case Variant::BOOL:
							value_text = p_ver->get_value().booleanize() ? "true" : "false";
							break;
						case Variant::INT:
						case Variant::FLOAT:
							// value_text = itos(int64_t(_value));
							// break;
							value_text = rtos(real_t(p_ver->get_value()));
							break;
						case Variant::STRING:
							value_text = String("'" + String(p_ver->get_value()) + "'");
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
			switch (p_ver->get_trigger_type()) {
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

List<String> FsmEditor::get_transition_res_valid_and_texts(const Ref<TransitionRes> &p_transition_res, bool &r_valid) const {
	List<String> ret;
	r_valid = false;

	switch (p_transition_res->get_type()) {
		IF_FULL_VERSION(
				case TransitionRes::TRANSITION_TYPE_SCRIPT
				: {
					auto tr_script = p_transition_res->get_transition_script();
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
				} break;)
		case TransitionRes::TRANSITION_TYPE_VARIABLE: {
			auto variable_expression_res_list = p_transition_res->get_variable_expression_res_list();
			if (variable_expression_res_list.size() > 0) {
				r_valid = true;
				ret.push_back(str_localize("HFSMVariable Expressions: ") + String(p_transition_res->is_variable_and_mode() ? "AND" : "OR"));
				for (auto i = 0; i < variable_expression_res_list.size(); i++) {
					Ref<VariableExpressionRes> ver = variable_expression_res_list[i];
					if (ver.is_valid()) {
						ret.push_back(get_variable_expression_res_valid_and_text(ver, r_valid));
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
			if (p_transition_res->get_expression_text().is_empty()) {
				ret.push_back(str_localize("Empty expression!"));
			} else {
				r_valid = true;
				ret.push_back(String("Expression: ") + p_transition_res->get_expression_text());
				ret.push_back(String("Comment: ") + p_transition_res->get_expression_comment());
			}
		} break;
		case TransitionRes::TRANSITION_TYPE_AUTO: {
			switch (p_transition_res->get_auto_mode()) {
				case TransitionRes::AUTO_TRANSIT_MODE_DELAY_TIMER: {
					r_valid = true;
					ret.push_back(str_localize("Auto: ") + str_localize("Delay ") + itos(static_cast<int64_t>(p_transition_res->get_auto_delay_msec())) + str_localize(" msec."));
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
					ret.push_back(str_localize("Auto: ") + str_localize("After \"_update()\" being called ") + itos(p_transition_res->get_auto_times()) + str_localize(" times."));

				} break;
				case TransitionRes::AUTO_TRANSIT_MODE_PHYSICS_UPDATE_TIMES: {
					r_valid = true;
					ret.push_back(str_localize("Auto: ") + str_localize("After \"_physics_update()\" being called ") + itos(p_transition_res->get_auto_times()) + str_localize(" times."));
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

void FsmEditor::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_CHILD_ORDER_CHANGED: {
			IF_GDE(if (!is_node_ready()) {
				return;
			})
			IF_GDM(if (!is_ready()) {
				return;
			})
			if (is_blocking_redraw() || connection_dirty) {
				return;
			}
			connection_dirty = true;
			draw_layer->queue_redraw();
		} break;
		case NOTIFICATION_READY: {
			set_process(false);
			draw_layer->connect("draw", TCALLABLE(_draw_layer_draw));

			connect("gui_input", TCALLABLE(_gui_input_internal));
			connect("end_node_move", TCALLABLE(_end_node_move));
			set_editor_inspector_signal_connected(true);

			propagate_notification(NOTIFICATION_THEME_CHANGED);
		} break;
		case NOTIFICATION_DRAW: {
			draw_layer->queue_redraw();
		} break;
		case EditorSettings::NOTIFICATION_EDITOR_SETTINGS_CHANGED:
		case NOTIFICATION_THEME_CHANGED: {
			font = HfsmEditorPlugin::get_singleton()->get_editor_interface()->get_base_control()->get_theme()->get_default_font();
			activity_color = HfsmEditorPlugin::get_singleton()->get_editor_interface()->get_base_control()->get_theme_color("activity", "GraphEdit");
		}
		default:
			break;
	}
}

FsmEditor::FsmEditor(bool p_debug_mode) :
		debug_mode(p_debug_mode),
		TRIANGLE_POINTS(make_arr<PackedVector2Array>(Vector2(20, 0), Vector2(-15, 10), Vector2(-15, -10))){};

void FsmEditor::debug_highlight_active_state(const StringName &p_state_name, bool p_deactive_all) {
	StateNode *prev_activated = nullptr;
	StateNode *next_activated = nullptr;
	for (auto i = 0; i < get_child_count(); ++i) {
		if (auto sn = cast_to<StateNode>(get_child(i))) {
			if (sn->is_debug_actived()) {
				prev_activated = sn;
			}

			if (!p_deactive_all && sn->get_state_res()->get_state_name() == p_state_name) {
				sn->set_debug_actived(true);
				sn->set_self_modulate(Color::named("GREEN"));
				next_activated = sn;
			} else {
				sn->set_debug_actived(false);
				sn->set_self_modulate(Color::named("WHITE"));
			}
		}
	}

	TypedArray<Dictionary> conn_list = call(SNAME("get_connection_list"));
	for (auto i = 0; i < conn_list.size(); ++i) {
		Dictionary conn = conn_list[i];
		set_connection_activity(conn["from"], 0, conn["to"], 0, 0.0);
	}

	if (prev_activated && next_activated && prev_activated != next_activated) {
		auto from = prev_activated->get_name();
		auto to = next_activated->get_name();
		ERR_FAIL_COND(!is_node_connected(from, 0, to, 0));

		debug_activity_from = from;
		debug_activity_to = to;

		auto tween = next_activated->create_tween();
		tween->tween_method(TCALLABLE_BIND(_debug_tween_activity, from, to), 1.0f, 0.0f, 1.0)->set_ease(Tween::EaseType::EASE_IN);
	} else {
		debug_activity_from = "";
		debug_activity_to = "";
		debug_activity = 0.0;
		queue_redraw();
	}
}

StateNode *FsmEditor::_get_state_node(const NodePath &p_path) {
	IF_GDE(return cast_to<StateNode>(call(SNAME("get_node"), p_path));)
	IF_GDM(return cast_to<StateNode>(get_node(p_path));)
}

void FsmEditor::__queue_refresh() {
	for (auto i = 0; i < get_child_count(); ++i) {
		if (auto sn = cast_to<StateNode>(get_child(i))) {
			sn->get_state_res()->emit_signal(SNAME("changed"));
		}
	}
	queue_redraw();
	queuing_refresh = false;
}

void FsmEditor::queue_refresh() {
	if (!queuing_refresh) {
		call_deferred(TNAMEOF(__queue_refresh));
		queuing_refresh = true;
	}
}
}; // namespace Hfsm
