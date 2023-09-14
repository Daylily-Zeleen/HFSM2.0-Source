/**************************************************************************/
/*  fsm_editor.cpp                                                        */
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

#include "fsm_editor.h"

#ifdef GDEXTENSION_BUILD
#include <godot_cpp/classes/editor_inspector.hpp>
#include <godot_cpp/classes/editor_interface.hpp>
#include <godot_cpp/classes/editor_settings.hpp>
#include <godot_cpp/classes/file_access.hpp>
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

#include "../src/fsm_config.h"
#include "../src/transition_config.h"
#include "../src/transitions/transition.h"

#include "../hfsm_global.h"
#include "hfsm_editor.h"
#include "hfsm_editor_plugin.h"
#include "state_node.h"

using namespace godot;
namespace HFSM2 {

#define TRANSITION_SELECT_EXTENT 10.0f
#define CONN_POS_OFFSET 50.0f
#define MOVE_ZONE_HIGHT 30.0f

#define set_editor_inspector_signal_connected(p_connected)                     \
	{                                                                          \
		const auto s_edited_object_changed = SNAME("edited_object_changed");   \
		auto inspector = EditorInterface::get_singleton()->get_inspector();    \
		auto cb = TCALLABLE(_disconnect_inspecting_transition_config);         \
                                                                               \
		bool connected = inspector->is_connected(s_edited_object_changed, cb); \
		if constexpr (p_connected) {                                           \
			if (connected) {                                                   \
				inspector->disconnect(s_edited_object_changed, cb);            \
			}                                                                  \
		}                                                                      \
		if constexpr (!(p_connected)) {                                        \
			if (!inspector) {                                                  \
				inspector->connect(s_edited_object_changed, cb);               \
			}                                                                  \
		}                                                                      \
	}

#define TRIANGLE_TEXTURE_WIDTH 32

#ifdef IDE_TYPE_SAFE
#define ADD_DO_DEFERRED_CALL_METHOD(m_obj_ptr, m_method, ...)        \
	DECLTYPE_METHOD_RETURN_TYPE(m_obj_ptr, m_method, ##__VA_ARGS__); \
	undo_redo->add_do_method(m_obj_ptr, "call_deferred", #m_method, ##__VA_ARGS__)
#define ADD_UNDO_DEFERRED_CALL_METHOD(m_obj_ptr, m_method, ...)       \
	DECLTYPE_METHOD_RETURN_TYPE(m_obj_ptr, m_method, ##__VA_ARGS__);  \
	undo_redo->add_undo_method(m_obj_ptr, "call_deferred", #m_method, \
			##__VA_ARGS__)
#else
#define ADD_DO_DEFERRED_CALL_METHOD(m_obj_ptr, m_method, ...) \
	undo_redo->add_do_method(m_obj_ptr, "call_deferred", #m_method, ##__VA_ARGS__)
#define ADD_UNDO_DEFERRED_CALL_METHOD(m_obj_ptr, m_method, ...) \
	undo_redo->add_undo_method(m_obj_ptr, "call_deferred", #m_method, ##__VA_ARGS__)
#endif // IDE_TYPE_SAFE

#define s_edit_fsm_requested "_edit_fsm_requested"

String FsmEditor::str_localize(const String &en_key) const {
	return HFSMEditorPlugin::str_localize(en_key);
}

void FsmEditor::_bind_methods() {
	GDBIND_BEGIN(FsmEditor);

	GDBIND_METHOD(edit_fsm_config, "fsm_config", "path_button_container", "root_fsm_config", "as_action");

	GDBIND_METHOD(__queue_refresh);
	GDBIND_METHOD(__queue_redraw_request);
	GDBIND_METHOD(__queue_redraw);
	// UNDO REDO
	GDBIND_METHOD(__set_current_fsm_config);
	GDBIND_METHOD(__set_selected_transition_config_list);
	GDBIND_METHOD(__set_copied_state_config_list);
	GDBIND_METHOD(__set_copied_transition_list);
	GDBIND_METHOD(__set_selected_state_name_list);
	GDBIND_METHOD(__select_state_nodes);
	GDBIND_METHOD(__select_mamually, "val");
	GDBIND_METHOD(__set_blocking_redraw);
	// CALLBACKS
	GDBIND_CALBACK(_disconnect_inspecting_transition_config);
	GDBIND_CALBACK(_transition_config_updated);
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

	ADD_SIGNAL(MethodInfo(s_edit_fsm_requested, PropertyInfo(Variant::OBJECT, "sub_fsm_config", PROPERTY_HINT_RESOURCE_TYPE, FSMConfig::get_class_static())));
}

// ========== SetGet =========
void FsmEditor::__set_current_fsm_config(const Ref<FSMConfig> &to_set, const Ref<FSMConfig> &p_root) {
	auto cb = TCALLABLE(__queue_redraw_request);
	if (current_fsm_config.is_valid() && current_fsm_config->is_connected(s_changed, cb)) {
		current_fsm_config->disconnect(s_changed, cb);
	}
	current_fsm_config = to_set;
	current_root_fsm_config = p_root;
	if (current_fsm_config.is_valid() && !current_fsm_config->is_connected(s_changed, cb)) {
		current_fsm_config->connect(s_changed, cb);
	}
	mask_panel->set_visible(current_fsm_config.is_null());
}

void FsmEditor::__set_selected_state_name_list(const TypedArray<StringName> &p_to_set) {
	if (selected_state_name_list != p_to_set) {
		selected_state_name_list = p_to_set;
	}

	selected_transition_config_list.clear();
	foreach_connection_by_nodes([this](StateNode *p_from, StateNode *p_to) -> bool {
		if (!p_from || !p_to) {
			return false;
		}

		auto from_name = p_from->get_state_config()->get_state_name();
		auto to_name = p_to->get_state_config()->get_state_name();
		if (selected_state_name_list.has(from_name) && selected_state_name_list.has(to_name)) {
			auto tc = get_transition_config(p_from, p_to);
			if (tc.is_valid()) {
				selected_transition_config_list.push_back(tc);
			}
		}

		return false;
	});

	__set_selected_transition_config_list(selected_transition_config_list);
}

TypedArray<StateNode> FsmEditor::get_selected_state_nodes() {
	TypedArray<StateNode> ret;
	for (auto i = 0; i < get_child_count(); i++) {
		if (auto node = cast_to<StateNode>(get_child(i))) {
			auto state_name = node->get_state_config()->get_state_name();
			if (selected_state_name_list.has(state_name)) {
				ERR_CONTINUE_EDMSG(!node->is_selected(), "Unbelievable!");
				ret.push_back(node);
			}
		}
	}
	return ret;
}

void FsmEditor::__set_selected_transition_config_list(const TypedArray<TransitionConfig> &p_to_set) {
	_disconnect_inspecting_transition_config();
	set_editor_inspector_signal_connected(false);

	if (selected_transition_config_list != p_to_set) {
		selected_transition_config_list = p_to_set;
	}
	inspecting_transition_config = selected_transition_config_list.size() == 1 ? selected_transition_config_list[0] : Variant(); // 同时只能监视一个 TransitionConfig
	if (inspecting_transition_config.is_valid()) {
		inspecting_transition_config->connect(s_changed, TCALLABLE(_transition_config_updated));
	}

	ERR_FAIL_COND(!EditorInterface::get_singleton());

	if (selected_state_name_list.size() != 1) {
		EditorInterface::get_singleton()->inspect_object(inspecting_transition_config.ptr());
		set_editor_inspector_signal_connected(true);
	}
}

void FsmEditor::__set_copied_transition_list(const TypedArray<TransitionConfig> &p_to_set) {
	if (copied_transition_config_list != p_to_set) {
		copied_transition_config_list = p_to_set;
	}
}

void FsmEditor::__set_copied_state_config_list(const TypedArray<StateConfig> &p_to_set) {
	if (copied_state_config_list != p_to_set) {
		copied_state_config_list = p_to_set;
	}
}

// ========功能=========

void FsmEditor::try_disconnect(const Vector2 &p_pos1, const Vector2 &p_pos2) {
	if (debug_mode) {
		return;
	}

	// const auto graph_zoom = static_cast<float>(get_zoom());
	const Vector2 scaled_pos1 = p_pos1; // * graph_zoom;
	const Vector2 scaled_pos2 = p_pos2; // * graph_zoom;

	LocalVector<Pair<StateNode *, StateNode *>> to_delete;
	foreach_connection_by_nodes([this, &to_delete, scaled_pos1, scaled_pos2](StateNode *p_from, StateNode *p_to) -> bool {
		if (p_from && p_to) {
			auto scaled_line = get_connection_line_with_zoom(p_from, p_to);
			if (is_judge(scaled_pos1, scaled_pos2, scaled_line[0], scaled_line[1])) {
				to_delete.push_back({ p_from, p_to });
			}
		}
		return false;
	});

	if (to_delete.size() > 0) {
		HFSM_EDITOR_CREATE_ACTION("Delete State Transitions");
		for (const auto &E : to_delete) {
			auto tc = get_transition_config(E.first, E.second);
			ERR_CONTINUE(tc.is_null());
			ADD_DO_METHOD(this, disconnect_node, E.first->get_name(), 0, E.second->get_name(), 0);
			ADD_DO_METHOD(current_fsm_config.ptr(), remove_transition_config, tc);
			ADD_UNDO_METHOD(current_fsm_config.ptr(), add_transition_config, tc);
			ADD_UNDO_METHOD(this, connect_node, E.first->get_name(), 0, E.second->get_name(), 0);
		}
		COMMIT_ACTION();
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

TypedArray<TransitionConfig> FsmEditor::try_select_transitions_at_pos(const Vector2 &p_pos) {
	TypedArray<TransitionConfig> ret;
	float graph_zoom = get_zoom();

	foreach_connection_by_nodes([this, graph_zoom, p_pos, &ret](StateNode *from, StateNode *to) {
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
			auto tc = get_transition_config(from, to);
			if (tc.is_valid()) {
				ret.push_back(tc);
			}
		}
		return false;
	});

	return ret;
}

Ref<TransitionConfig> FsmEditor::get_transition_config(StateNode *p_from, StateNode *p_to) {
	auto tc_list = current_fsm_config->get_transition_config_list();
	for (auto i = 0; i < tc_list.size(); i++) {
		Ref<TransitionConfig> tc = tc_list[i];
		if (tc.is_valid() && tc->get_from_state_config() == p_from->get_state_config() && tc->get_to_state_config() == p_to->get_state_config()) {
			return tc;
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
			node->set_selected(selected_state_name_list.has(node->get_state_config()->get_state_name()));
		}
	}
}

StateNode *FsmEditor::create_state_node(const Ref<StateConfig> &p_state_config, const Ref<FSMConfig> &p_fsm_config) {
	auto ret = StateNode::create_state_node(p_state_config, p_fsm_config.is_null() ? current_fsm_config : p_fsm_config, debug_mode);
	ret->connect(SNAME(s_edit_fsm_requested), TCALLABLE(_edit_sub_fsm_requested));
	ret->connect(SNAME("_reconnected_requested"), TCALLABLE(_state_node_reconnected_requested));
	return ret;
}

StateNode *FsmEditor::get_top_state_node_which_hovering() {
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

TypedArray<StateConfig> FsmEditor::get_selected_state_config_list() {
	TypedArray<StateConfig> ret;
	auto seleted_state_nodes = get_selected_state_nodes();
	for (auto i = 0; i < seleted_state_nodes.size(); i++) {
		if (auto sn = cast_to<StateNode>(seleted_state_nodes[i])) {
			ret.push_back(sn->get_state_config());
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
				to_select_state_name_list.push_back(sn->get_state_config()->get_state_name());
			}
		}
	}
	__set_selected_state_name_list(to_select_state_name_list.duplicate());
	bakcup_selected_state_name_list = to_select_state_name_list;
	selection_dirty = false;
}

// ==================

void FsmEditor::_edit_sub_fsm_requested(const Ref<FSMConfig> &p_sub_fsm_config) {
	emit_signal(SNAME(s_edit_fsm_requested), p_sub_fsm_config);
}

void FsmEditor::_state_node_reconnected_requested(const StringName &p_old_name, const StringName &p_new_name) {
	if (!get_node_or_null({ p_new_name })) {
		return;
	}

	foreach_connection_by_names([this, p_old_name, p_new_name](const StringName &p_from, const StringName &p_to) {
		if (p_from == p_old_name) {
			disconnect_node(p_old_name, 0, p_to, 0);
			connect_node(p_new_name, 0, p_to, 0);
		} else if (p_to == p_old_name) {
			disconnect_node(p_from, 0, p_old_name, 0);
			connect_node(p_from, 0, p_new_name, 0);
		}
		return false;
	});
}

void FsmEditor::_popup_menu_id_pressed(int32_t p_id) {
	if (debug_mode) {
		return;
	}

	switch (p_id) {
		case ITEM_ADD_STATE: {
			if (__hovering_state_node) {
				return;
			}
			Ref<StateConfig> new_sc;
			new_sc.instantiate();
			new_sc->set_editor_offset((get_local_mouse_position() + _get_scroll_offset()) / get_zoom());
			if (current_fsm_config->get_state_config_list().is_empty()) {
				new_sc->set_type(State::STATE_TYPE_ENTRY);
			}
			auto new_sn = create_state_node(new_sc);

			HFSM_EDITOR_CREATE_ACTION("Add State");
			ADD_DO_REFERENCE(new_sn);
			ADD_DO_METHOD(this, add_child, new_sn);
			ADD_DO_METHOD(current_fsm_config.ptr(), add_state_config, new_sc);
			ADD_DO_METHOD(this, __select_mamually, make_arr<TypedArray<StateNode>>(new_sn));
			ADD_UNDO_METHOD(this, __select_mamually, TypedArray<StateNode>());
			ADD_UNDO_METHOD(current_fsm_config.ptr(), remove_state_config, new_sc);
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
			TypedArray<StateConfig> to_copied_state_config;
			TypedArray<StateNode> selected_state_nodes = get_selected_state_nodes();
			for (auto i = 0; i < selected_state_nodes.size(); i++) {
				auto node = cast_to<StateNode>(selected_state_nodes[i]);
				if (!node) {
					continue;
				}

				ADD_UNDO_REFERENCE(node);
				ADD_DO_METHOD(this, remove_child, node);
				ADD_DO_METHOD(current_fsm_config.ptr(), remove_state_config, node->get_state_config());
				ADD_UNDO_METHOD(current_fsm_config.ptr(), add_state_config, node->get_state_config());
				ADD_UNDO_METHOD(this, add_child, node);

				to_copied_state_config.push_back(node->get_state_config());
			}

			auto tc_list = current_fsm_config->get_transition_config_list();
			for (auto i = 0; i < tc_list.size(); i++) {
				Ref<TransitionConfig> tc = tc_list[i];
				auto from_node = cast_to<StateNode>(tc->get_from_state_config()->get_state_node());
				auto to_node = cast_to<StateNode>(tc->get_from_state_config()->get_state_node());
				if (!from_node || !to_node || selected_state_name_list.has(tc->get_from_state_config()->get_state_name()) || selected_state_name_list.has(tc->get_to_state_config()->get_state_name())) {
					ADD_DO_METHOD(current_fsm_config.ptr(), remove_transition_config, tc);
					ADD_UNDO_METHOD(current_fsm_config.ptr(), add_transition_config, tc);
				}
			}

			ADD_DO_METHOD(this, __set_copied_transition_list, selected_transition_config_list.duplicate());
			ADD_UNDO_METHOD(this, __set_copied_transition_list, copied_transition_config_list);
			ADD_UNDO_METHOD(this, __select_mamually, selected_state_nodes);
			ADD_DO_METHOD(this, __set_copied_state_config_list, to_copied_state_config);
			ADD_UNDO_METHOD(this, __set_copied_state_config_list, copied_state_config_list);

			ADD_DO_METHOD(this, __set_blocking_redraw, false);
			ADD_UNDO_METHOD(this, __set_blocking_redraw, false);
			COMMIT_ACTION();
		} break;
		case ITEM_COPY_STATES: {
			if (selected_state_name_list.size() <= 0) {
				return;
			}
			TypedArray<StateConfig> to_copied_state_config_list;
			TypedArray<StateNode> selected_state_node_list = get_selected_state_nodes();
			for (auto i = 0; i < selected_state_node_list.size(); i++) {
				to_copied_state_config_list.push_back(cast_to<StateNode>(selected_state_node_list[i])->get_state_config());
			}
			if (to_copied_state_config_list.size() == copied_state_config_list.size()) {
				bool difference = false;
				for (auto i = 0; i < to_copied_state_config_list.size(); i++) {
					Ref<StateConfig> sc = to_copied_state_config_list[i];
					if (!selected_state_name_list.has(sc)) {
						difference = true;
						break;
					}
				}
				if (!difference) {
					return; // 相同，不执行拷贝， 直接返回
				}
			}

			HFSM_EDITOR_CREATE_ACTION("Copy States");
			ADD_DO_METHOD(this, __set_copied_state_config_list, to_copied_state_config_list);
			ADD_UNDO_METHOD(this, __set_copied_state_config_list, copied_state_config_list);
			ADD_DO_METHOD(this, __set_copied_transition_list, selected_transition_config_list);
			ADD_UNDO_METHOD(this, __set_copied_transition_list, copied_transition_config_list);
			COMMIT_ACTION();
		} break;
		case ITEM_PASTE_STATES: {
			if (copied_state_config_list.size() <= 0) {
				return;
			}
			HFSM_EDITOR_CREATE_ACTION("Paste States");
			ADD_DO_METHOD(this, __set_blocking_redraw, true);
			ADD_UNDO_METHOD(this, __set_blocking_redraw, true);
			HashMap<Ref<StateConfig>, Ref<StateConfig>> osc2csc;
			// 计算中心
			Vector2 center;
			for (auto i = 0; i < copied_state_config_list.size(); i++) {
				Ref<StateConfig> state_config = copied_state_config_list[i];
				center += state_config->get_editor_offset();
			}
			center /= static_cast<float>(copied_state_config_list.size());
			auto mouse_offset = (get_local_mouse_position() + _get_scroll_offset()) / static_cast<float>(get_zoom());
			// 计算偏移
			auto offset = center - mouse_offset;
			// 复制
			for (auto i = 0; i < copied_state_config_list.size(); i++) {
				Ref<StateConfig> sc = copied_state_config_list[i];
				Ref<StateConfig> csc = sc->duplicate(true);
				csc->set_editor_offset(csc->get_editor_offset() - offset);
				auto csn = create_state_node(csc);
				osc2csc.insert(sc, csc);
			}
			// 添加
			TypedArray<StateNode> copied_state_ndoes;
			for (const auto &kv : osc2csc) {
				auto csn = cast_to<StateNode>(kv.value->get_state_node());
				copied_state_ndoes.push_back(csn);

				ADD_DO_REFERENCE(csn);
				ADD_DO_METHOD(this, add_child, csn);
				ADD_DO_METHOD(current_fsm_config.ptr(), add_state_config, csn->get_state_config());
				ADD_UNDO_METHOD(current_fsm_config.ptr(), remove_state_config, csn->get_state_config());
				ADD_UNDO_METHOD(this, remove_child, csn);
			}
			// 拷贝相关转换
			for (auto i = 0; i < copied_transition_config_list.size(); i++) {
				Ref<TransitionConfig> tc = copied_transition_config_list[i];
				if (copied_state_config_list.has(tc->get_from_state_config()) && copied_state_config_list.has(tc->get_to_state_config())) {
					Ref<TransitionConfig> ctc;
					ctc.instantiate();
					ctc->set_from_state_config(osc2csc[tc->get_from_state_config()]);
					ctc->set_to_state_config(osc2csc[tc->get_to_state_config()]);
					StringName from = ctc->get_from_state_config()->get_state_node()->get_name();
					StringName to = ctc->get_to_state_config()->get_state_node()->get_name();

					ADD_DO_METHOD(current_fsm_config.ptr(), add_transition_config, ctc);
					ADD_DO_METHOD(this, connect_node, from, 0, to, 0);
					ADD_UNDO_METHOD(this, disconnect_node, from, 0, to, 0);
					ADD_UNDO_METHOD(current_fsm_config.ptr(), remove_transition_config, ctc);
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
			auto selected_state_config_list = get_selected_state_config_list();
			if (selected_state_config_list.size() <= 0) {
				return;
			}
			HFSM_EDITOR_CREATE_ACTION("Duplicate States");
			ADD_DO_METHOD(this, __set_blocking_redraw, true);
			ADD_UNDO_METHOD(this, __set_blocking_redraw, true);
			TypedArray<StateNode> csn_list;
			HashMap<Ref<StateConfig>, Ref<StateConfig>> osc2csc;
			for (auto i = 0; i < selected_state_config_list.size(); i++) {
				Ref<StateConfig> sc = selected_state_config_list[i];
				Ref<StateConfig> csc = sc->duplicate(true);
				csc->set_editor_offset(csc->get_editor_offset() + DUPLICATE_OFFSET);
				auto csn = create_state_node(csc);
				csn_list.push_back(csn);
				osc2csc.insert(sc, csc);

				ADD_DO_REFERENCE(csn);
				ADD_DO_METHOD(this, add_child, csn);
				ADD_DO_METHOD(current_fsm_config.ptr(), add_state_config, csc);
				ADD_UNDO_METHOD(current_fsm_config.ptr(), remove_state_config, csc);
				ADD_UNDO_METHOD(this, remove_child, csn);
			}
			// 拷贝相关转换
			for (auto i = 0; i < selected_transition_config_list.size(); i++) {
				Ref<TransitionConfig> tc = selected_transition_config_list[i];
				if (selected_state_config_list.has(tc->get_from_state_config()) && selected_state_config_list.has(tc->get_to_state_config())) {
					Ref<TransitionConfig> ctc;
					ctc.instantiate();
					ctc->set_from_state_config(osc2csc[tc->get_from_state_config()]);
					ctc->set_to_state_config(osc2csc[tc->get_to_state_config()]);
					StringName from = ctc->get_from_state_config()->get_state_node()->get_name();
					StringName to = ctc->get_to_state_config()->get_state_node()->get_name();

					ADD_DO_METHOD(current_fsm_config.ptr(), add_transition_config, ctc);
					ADD_DO_METHOD(this, connect_node, from, 0, to, 0);
					ADD_UNDO_METHOD(this, disconnect_node, from, 0, to, 0);
					ADD_UNDO_METHOD(current_fsm_config.ptr(), remove_transition_config, ctc);
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
			auto selected_state_config_list = get_selected_state_config_list();
			if (selected_state_config_list.size() > 0) {
				HFSM_EDITOR_CREATE_ACTION("Delete States");
				ADD_DO_METHOD(this, __set_blocking_redraw, true);
				ADD_UNDO_METHOD(this, __set_blocking_redraw, true);
				// 移除相关的转换流
				auto tc_list = current_fsm_config->get_transition_config_list();
				for (auto i = 0; i < tc_list.size(); ++i) {
					Ref<TransitionConfig> tc = tc_list[i];
					if (selected_state_config_list.has(tc->get_from_state_config()) ||
							selected_state_config_list.has(tc->get_to_state_config())) {
						StringName from = tc->get_from_state_config()->get_state_node()->get_name();
						StringName to = tc->get_to_state_config()->get_state_node()->get_name();

						ADD_DO_METHOD(this, disconnect_node, from, 0, to, 0);
						ADD_DO_METHOD(current_fsm_config.ptr(), remove_transition_config, tc);
						ADD_UNDO_METHOD(current_fsm_config.ptr(), add_transition_config, tc);
						ADD_UNDO_METHOD(this, connect_node, from, 0, to, 0);
					}
				}
				// 移除状态
				for (auto i = 0; i < selected_state_config_list.size(); ++i) {
					Ref<StateConfig> sc = selected_state_config_list[i];
					ADD_DO_REFERENCE(sc->get_state_node());
					ADD_DO_METHOD(this, remove_child, sc->get_state_node());
					ADD_DO_METHOD(current_fsm_config.ptr(), remove_state_config, sc);
					ADD_UNDO_METHOD(current_fsm_config.ptr(), add_state_config, sc);
					ADD_UNDO_METHOD(this, add_child, sc->get_state_node());
				}

				ADD_DO_METHOD(this, __set_blocking_redraw, false);
				ADD_UNDO_METHOD(this, __set_blocking_redraw, false);
				ADD_UNDO_METHOD(this, __select_mamually, get_selected_state_nodes());
				ADD_DO_METHOD(connection_layer, queue_redraw);
				ADD_UNDO_METHOD(connection_layer, queue_redraw);
				COMMIT_ACTION();
			} else if (selected_transition_config_list.size() >= 0) {
				HFSM_EDITOR_CREATE_ACTION("Delete State Transitions");
				for (auto i = 0; i < selected_transition_config_list.size(); i++) {
					Ref<TransitionConfig> tc = selected_transition_config_list[i];
					StringName from = tc->get_from_state_config()->get_state_node()->get_name();
					StringName to = tc->get_to_state_config()->get_state_node()->get_name();

					ADD_DO_METHOD(this, disconnect_node, from, 0, to, 0);
					ADD_DO_METHOD(current_fsm_config.ptr(), remove_transition_config, tc);
					ADD_UNDO_METHOD(current_fsm_config.ptr(), add_transition_config, tc);
					ADD_UNDO_METHOD(this, connect_node, from, 0, to, 0);
					COMMIT_ACTION();
				}
			}
		} break;
		case ITEM_CONVERT_TO_FSM: {
			auto selected_state_config_list = get_selected_state_config_list();
			if (!__hovering_state_node || !selected_state_config_list.has(__hovering_state_node->get_state_config())) {
				return;
			}
			HFSM_EDITOR_CREATE_ACTION("Convert To Sub-FSM");
			ADD_DO_METHOD(this, __set_blocking_redraw, true);
			ADD_UNDO_METHOD(this, __set_blocking_redraw, true);
			// 复制状态资源
			Ref<StateConfig> duplicated_state_config = __hovering_state_node->get_state_config()->duplicate(true);
			// 新的子状态机
			Ref<FSMConfig> new_fsm_config;
			new_fsm_config.instantiate();
			new_fsm_config->set_nested_state_config(duplicated_state_config);
			// 复制的状态节点
			auto duplicated_state_node = create_state_node(duplicated_state_config);
			ADD_DO_REFERENCE(duplicated_state_node);
			//
			auto hovering_state_config = __hovering_state_node->get_state_config();
			auto hovering_state_node_name = __hovering_state_node->get_name();
			auto duplicated_state_node_name = duplicated_state_node->get_name();
			// 处理转换指向
			auto current_tr_list = current_fsm_config->get_transition_config_list();
			for (auto i = 0; i < current_tr_list.size(); i++) {
				Ref<TransitionConfig> tc = current_tr_list[i];
				StringName from = tc->get_from_state_config()->get_state_node()->get_name();
				StringName to = tc->get_to_state_config()->get_state_node()->get_name();
				// 一端为指定状态，另一端不在选中的状态中，处理指向
				if (tc->get_from_state_config() == __hovering_state_node->get_state_config() && !selected_state_config_list.has(tc->get_to_state_config())) {
					ADD_DO_METHOD(this, disconnect_node, from, 0, to, 0);
					ADD_DO_METHOD(tc.ptr(), set_from_state_config, duplicated_state_config);
					ADD_DO_DEFERRED_CALL_METHOD(this, connect_node, duplicated_state_node_name, 0, to, 0);

					ADD_DO_METHOD(this, disconnect_node, duplicated_state_node_name, 0, to, 0);
					ADD_DO_METHOD(tc.ptr(), set_from_state_config, hovering_state_config);
					ADD_DO_DEFERRED_CALL_METHOD(this, connect_node, from, 0, to, 0);
				} else if (tc->get_to_state_config() == __hovering_state_node->get_state_config() && !selected_state_config_list.has(tc->get_from_state_config())) {
					ADD_DO_METHOD(this, disconnect_node, from, 0, to, 0);
					ADD_DO_METHOD(tc.ptr(), set_to_state_config, duplicated_state_config);
					ADD_DO_DEFERRED_CALL_METHOD(this, connect_node, from, 0, duplicated_state_node_name, 0);

					ADD_DO_METHOD(this, disconnect_node, from, 0, duplicated_state_node_name, 0);
					ADD_DO_METHOD(tc.ptr(), set_to_state_config, hovering_state_config);
					ADD_DO_DEFERRED_CALL_METHOD(this, connect_node, from, 0, to, 0);
				} else if ((selected_state_config_list.has(tc->get_from_state_config()) && !selected_state_config_list.has(tc->get_to_state_config())) ||
						(selected_state_config_list.has(tc->get_to_state_config()) && !selected_state_config_list.has(tc->get_from_state_config()))) {
					// 一端为选中对象，另一端不在选中状态中，删除（以排除一端为指定状态的情况
					ADD_DO_METHOD(this, disconnect_node, from, 0, to, 0);
					ADD_DO_METHOD(current_fsm_config.ptr(), remove_transition_config, tc);

					ADD_UNDO_METHOD(current_fsm_config.ptr(), add_transition_config, tc);
					ADD_UNDO_DEFERRED_CALL_METHOD(this, connect_node, from, 0, to, 0);
				} else if (selected_state_config_list.has(tc->get_from_state_config()) && selected_state_config_list.has(tc->get_to_state_config())) {
					ADD_DO_METHOD(this, disconnect_node, from, 0, to, 0);
					ADD_DO_METHOD(current_fsm_config.ptr(), remove_transition_config, tc);
					ADD_DO_METHOD(new_fsm_config.ptr(), add_transition_config, tc);

					ADD_UNDO_METHOD(new_fsm_config.ptr(), remove_transition_config, tc);
					ADD_UNDO_METHOD(current_fsm_config.ptr(), add_transition_config, tc);
					ADD_UNDO_DEFERRED_CALL_METHOD(this, connect_node, from, 0, to, 0);
				}
			}
			// 对复制节点的操作（被相关状态节的撤回操作所依赖， 需要提前
			ADD_UNDO_METHOD(this, remove_child, duplicated_state_node);
			ADD_UNDO_METHOD(current_fsm_config.ptr(), remove_state_config, duplicated_state_config);
			ADD_UNDO_METHOD(duplicated_state_config.ptr(), set_sub_fsm_config, Ref<FSMConfig>());
			ADD_UNDO_METHOD(duplicated_state_config.ptr(), set_nested, false);
			ADD_UNDO_METHOD(hovering_state_config.ptr(), set_type, hovering_state_config->get_type());
			ADD_UNDO_METHOD(hovering_state_config.ptr(), set_state_script, hovering_state_config->get_state_script());
			// 移动选中节点所处的状态机
			auto selected_state_nodes = get_selected_state_nodes();
			for (auto i = 0; i < selected_state_nodes.size(); i++) {
				auto sn = cast_to<StateNode>(selected_state_nodes[i]);
				ADD_DO_METHOD(this, remove_child, sn);
				ADD_DO_METHOD(current_fsm_config.ptr(), remove_state_config, sn->get_state_config());
				ADD_DO_METHOD(new_fsm_config.ptr(), add_state_config, sn->get_state_config());

				ADD_UNDO_METHOD(new_fsm_config.ptr(), remove_state_config, sn->get_state_config());
				ADD_UNDO_METHOD(current_fsm_config.ptr(), add_state_config, sn->get_state_config());
				ADD_UNDO_METHOD(this, add_child, sn);
				ADD_UNDO_REFERENCE(sn);
			}
			//
			ADD_DO_METHOD(hovering_state_config.ptr(), set_state_script, Ref<Script>());
			ADD_DO_METHOD(hovering_state_config.ptr(), set_type, State::STATE_TYPE_ENTRY);
			ADD_DO_METHOD(duplicated_state_config.ptr(), set_nested, true);
			ADD_DO_METHOD(duplicated_state_config.ptr(), set_sub_fsm_config, new_fsm_config);
			ADD_DO_METHOD(current_fsm_config.ptr(), add_state_config, duplicated_state_config);
			ADD_DO_METHOD(this, add_child, duplicated_state_node);

			ADD_DO_METHOD(this, __select_mamually, make_arr<TypedArray<StateNode>>(duplicated_state_node));
			ADD_UNDO_METHOD(this, __select_mamually, selected_state_nodes);

			ADD_DO_METHOD(this, __set_blocking_redraw, false);
			ADD_UNDO_METHOD(this, __set_blocking_redraw, false);
			ADD_DO_METHOD(connection_layer, queue_redraw);
			ADD_UNDO_METHOD(connection_layer, queue_redraw);
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
	auto tc = get_transition_config(from_node, to_node);
	if (tc.is_valid()) {
		return;
	}
	Ref<TransitionConfig> new_tr;
	new_tr.instantiate();
	new_tr->set_from_state_config(from_node->get_state_config());
	new_tr->set_to_state_config(to_node->get_state_config());
	// undoredo
	HFSM_EDITOR_CREATE_ACTION("Create State Transition");
	ADD_DO_METHOD(this, connect_node, p_from, p_from_slot, p_to, p_to_slot);
	ADD_UNDO_METHOD(this, disconnect_node, p_from, p_from_slot, p_to, p_to_slot);
	ADD_DO_METHOD(current_fsm_config.ptr(), add_transition_config, new_tr);
	ADD_UNDO_METHOD(current_fsm_config.ptr(), remove_transition_config, new_tr);

	auto new_transiion_config_list = make_arr<TypedArray<TransitionConfig>>(new_tr);
	ADD_DO_METHOD(this, __set_selected_state_name_list, TypedArray<StringName>());
	ADD_UNDO_METHOD(this, __set_selected_state_name_list, selected_state_name_list);

	ADD_DO_DEFERRED_CALL_METHOD(this, __set_selected_transition_config_list, new_transiion_config_list);
	ADD_UNDO_DEFERRED_CALL_METHOD(this, __set_selected_transition_config_list, selected_transition_config_list);

	ADD_DO_METHOD(connection_layer, queue_redraw);
	ADD_UNDO_METHOD(connection_layer, queue_redraw);
	COMMIT_ACTION();
}

void FsmEditor::_popup_request(const Vector2 &p_position) {
	if (debug_mode) {
		return;
	}

	menu->clear();
	menu->add_item(str_localize("Add State"), ITEM_ADD_STATE);
	menu->add_item(str_localize("Cut States"), ITEM_CUT_STATE, Key(KEYCODE_CUT_STATE));
	menu->add_item(str_localize("Copy States"), ITEM_COPY_STATES, Key(KEYCODE_COPY_STATES));
	menu->add_item(str_localize("Paste States"), ITEM_PASTE_STATES, Key(KEYCODE_PASTE_STATES));
	menu->add_item(str_localize("Duplicate States"), ITEM_DUPLICATE_STATES, Key(KEYCODE_DUPLICATE_STATES));
	menu->add_item(str_localize("Delete"), ITEM_DELETE, Key(KEYCODE_DELETE_STATES));
	menu->add_separator();
	menu->add_item(str_localize("Convert To Sub-FSM"), ITEM_CONVERT_TO_FSM);

	__hovering_state_node = get_top_state_node_which_hovering();
	if (__hovering_state_node) {
		menu->set_item_disabled(ITEM_ADD_STATE, true);
	}
	if (selected_state_name_list.size() <= 0) {
		menu->set_item_disabled(ITEM_CUT_STATE, true);
		menu->set_item_disabled(ITEM_COPY_STATES, true);
		menu->set_item_disabled(ITEM_DUPLICATE_STATES, true);
		if (selected_transition_config_list.size() <= 0) {
			menu->set_item_disabled(ITEM_DELETE, true);
		}
	}
	if (copied_state_config_list.size() <= 0) {
		menu->set_item_disabled(ITEM_PASTE_STATES, true);
	}

	if (!__hovering_state_node || !get_selected_state_nodes().has(__hovering_state_node)) {
		menu->set_item_disabled(menu->get_item_index(ITEM_CONVERT_TO_FSM), true);
		__hovering_state_node = nullptr;
	}

	menu->set_position(get_screen_position() + p_position);
	menu->popup();
}

void FsmEditor::_transition_config_updated() { connection_layer->queue_redraw(); }

void FsmEditor::_node_selected(Object *node) {
	auto sn = cast_to<StateNode>(node);
	if (!sn) {
		return;
	}

	const StringName state_name = sn->get_state_config()->get_state_name();
	if (!selected_state_name_list.has(state_name)) {
		selected_state_name_list.push_back(state_name);

		__set_selected_state_name_list(selected_state_name_list);
		selection_dirty = true;
	}

	if (selected_state_name_list.size() == 1) {
		EditorInterface::get_singleton()->inspect_object(sn->get_state_config().ptr());
	}
}

void FsmEditor::_node_deselected(Object *p_node) {
	auto sn = cast_to<StateNode>(p_node);
	if (!sn) {
		return;
	}

	const StringName state_name = sn->get_state_config()->get_state_name();
	if (selected_state_name_list.has(state_name)) {
		selected_state_name_list.erase(state_name);

		__set_selected_state_name_list(selected_state_name_list);
		selection_dirty = true;
	}
}

void FsmEditor::_disconnect_inspecting_transition_config() {
	if (inspecting_transition_config.is_valid()) {
		inspecting_transition_config->TDISCONNECT(s_changed, _transition_config_updated);
		inspecting_transition_config.unref();
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
			if (!node->get_state_config()->get_editor_offset().is_equal_approx(node->get_position_offset())) {
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
			ADD_DO_METHOD(node->get_state_config().ptr(), set_editor_offset, node->get_position_offset());
			ADD_UNDO_METHOD(node->get_state_config().ptr(), set_editor_offset, node->get_state_config()->get_editor_offset());
		}
	}
	COMMIT_ACTION();
}

void FsmEditor::_gui_input_internal(const Ref<InputEvent> &p_event) {
	if (debug_mode) {
		return;
	}

	static bool to_disconnect = false;
	const auto set_input_as_handled = [this]() { this->get_tree()->get_root()->set_input_as_handled(); };

	if (auto mouse_btn_event = Object::cast_to<InputEventMouseButton>(p_event.ptr())) {
		auto mouse_pos = connection_layer->get_local_mouse_position(); // mouse_btn_event->get_position(); //- connection_layer->get_position();
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
					connection_layer->queue_redraw();
					set_input_as_handled();
				}
			} break;
			case MOUSE_BUTTON(LEFT): {
				if (mouse_btn_event->is_double_click()) {
					if (mouse_btn_event->is_alt_pressed()) {
						return;
					}
					// 双击选择转换
					auto selected_tc_list = try_select_transitions_at_pos(mouse_pos);

					const auto undo_redo_selected_transition = [this](const TypedArray<TransitionConfig> &p_selected_tc_list) -> void {
						HFSM_EDITOR_CREATE_ACTION("Select State Transitions");
						ADD_DO_METHOD(this, __set_selected_state_name_list, TypedArray<StringName>());
						ADD_DO_METHOD(this, __set_selected_transition_config_list, p_selected_tc_list);
						ADD_DO_METHOD(connection_layer, queue_redraw);
						ADD_UNDO_METHOD(this, __set_selected_transition_config_list, this->selected_transition_config_list);
						ADD_UNDO_METHOD(this, __set_selected_state_name_list, selected_state_name_list);
						ADD_UNDO_METHOD(connection_layer, queue_redraw);
						COMMIT_ACTION();
					};

					if (selected_tc_list.size() != selected_transition_config_list.size()) {
						undo_redo_selected_transition(selected_tc_list);
					} else {
						for (auto i = 0; i < selected_tc_list.size(); i++) {
							Ref<TransitionConfig> tc = selected_tc_list[i];
							if (tc.is_valid() && !selected_transition_config_list.has(tc)) {
								undo_redo_selected_transition(selected_tc_list);
								break;
							}
						}
					}
					set_input_as_handled();
				} else if (!mouse_btn_event->is_pressed()) {
					if (selection_dirty) {
						const auto undo_redo_select_nodes = [this]() {
							HFSM_EDITOR_CREATE_ACTION(selected_state_name_list.size() == 0 ? "Deselect" : "Select States");
							ADD_DO_METHOD(this, __set_selected_transition_config_list, TypedArray<TransitionConfig>());
							ADD_DO_METHOD(this, __select_state_nodes, this->selected_state_name_list);
							ADD_UNDO_METHOD(this, __select_state_nodes, this->bakcup_selected_state_name_list.duplicate());
							ADD_UNDO_METHOD(this, __set_selected_transition_config_list, this->selected_transition_config_list.duplicate());
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
						if (selected_transition_config_list.size() > 0 || selected_state_name_list.size() > 0) {
							auto selected_tc_list = try_select_transitions_at_pos(mouse_pos);
							if (selected_tc_list.size() == 0 && !get_top_state_node_which_hovering()) {
								HFSM_EDITOR_CREATE_ACTION("Deselect");
								ADD_DO_METHOD(this, __set_selected_transition_config_list, selected_tc_list);
								ADD_DO_METHOD(this, __set_selected_state_name_list, TypedArray<StringName>());
								ADD_DO_METHOD(connection_layer, queue_redraw);
								ADD_UNDO_METHOD(this, __set_selected_state_name_list, selected_state_name_list);
								ADD_UNDO_METHOD(this, __set_selected_transition_config_list, selected_transition_config_list);
								ADD_UNDO_METHOD(connection_layer, queue_redraw);
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
			disconnect_line.set(1, connection_layer->get_local_mouse_position());
			connection_layer->queue_redraw();
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

Vector2 FsmEditor::_state_node_get_output_port_position(StateNode *p_state_node, int p_port_idx) const {
	IF_GDM(return p_state_node->get_output_port_position(p_port_idx);)
	IF_NOT_GDE_COMPATIBLE(return p_state_node->get_output_port_position(p_port_idx);)
	IF_GDE_COMPATIBLE({
		Vector2 ret = p_state_node->call(incompatible_apis.state_node_get_output_port_position, p_port_idx);
		if (likely(!HFSMGlobal::is_4_point_2_or_later())) {
			ret /= get_zoom();
		}
		return ret;
	})
}

Vector2 FsmEditor::_state_node_get_input_port_position(StateNode *p_state_node, int p_port_idx) const {
	IF_GDM(return p_state_node->get_input_port_position(p_port_idx);)
	IF_NOT_GDE_COMPATIBLE(return p_state_node->get_input_port_position(p_port_idx);)
	IF_GDE_COMPATIBLE({
		Vector2 ret = p_state_node->call(incompatible_apis.state_node_get_input_port_position, p_port_idx);
		if (likely(!HFSMGlobal::is_4_point_2_or_later())) {
			ret /= get_zoom();
		}
		return ret;
	})
}

PackedVector2Array FsmEditor::get_connection_line_with_zoom(StateNode *p_from, StateNode *p_to) {
	const float graph_zoom = get_zoom();

	const auto get_port_position = [this, graph_zoom](StateNode *p_node, bool p_from) {
		IF_GDM(auto port_position = p_from ? p_node->get_output_port_position(0) : p_node->get_input_port_position(0);)
		IF_NOT_GDE_COMPATIBLE(auto port_position = p_from ? p_node->get_output_port_position(0) : p_node->get_input_port_position(0);)
		IF_GDE_COMPATIBLE(Vector2 port_position = p_from ? _state_node_get_output_port_position(p_node, 0) : _state_node_get_input_port_position(p_node, 0);)
		return port_position + p_node->get_position_offset();
	};

	const auto from = get_port_position(p_from, true);
	const auto to = get_port_position(p_to, false);
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
		foreach_connection_by_names([this](const StringName &p_from, const StringName &p_to) {
			auto from = _get_state_node({ p_from });
			auto to = _get_state_node({ p_to });
			if (!from || !to) {
				disconnect_node(p_from, 0, p_to, 0);
			}
			return false;
		});
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

	const Color unactivated_triangle_color = StateNode::OUT_COLOR().lerp(StateNode::IN_COLOR(), 0.5f);

#ifdef DEV_ENABLED
	LocalVector<Ref<TransitionConfig>> dealed_tc_list;
	IF_GDE(dealed_tc_list.reserve(get_connection_list().size()));
	IF_GDM(List<Connection> conn_list;
			get_connection_list(&conn_list);
			dealed_tc_list.reserve(conn_list.size()));
	const auto action = [this, unactivated_triangle_color, &dealed_tc_list](const StringName &from_name, const StringName &to_name)
#else // DEV_ENABLED
	const auto action = [this, unactivated_triangle_color](const StringName &from_name, const StringName &to_name)
#endif // DEV_ENABLED
	{
		auto from = _get_state_node({ from_name });
		auto to = _get_state_node({ to_name });
		if (!from || !to) {
			return false;
		}
		// 正向
		auto tc = get_transition_config(from, to);
		// 异常
		ERR_FAIL_COND_V_MSG(tc.is_null(), false, "HFSM:: 异常 ，存在连接当不存在对应的转换流。");
		IF_DEV({
			ERR_FAIL_COND_V_MSG(dealed_tc_list.find(tc) >= 0, false, "不同的链接指向同一个 TransitionConfig? 这不可能");
			dealed_tc_list.push_back(tc);
		});

		auto selected = selected_transition_config_list.has(tc);

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
		connection_layer->draw_set_transform(center_pos, angle, clamped_scale);
		connection_layer->draw_colored_polygon(TRIANGLE_POINTS, triangle_color);

		TransitionConfigValidLevel valid = TRANSITION_CONFIG_VALID_LEVEL_ERROR;
		auto texts = get_transition_config_valid_and_texts(tc, valid);

		Color text_color;
		switch (valid) {
			case TRANSITION_CONFIG_VALID_LEVEL_ERROR: {
				text_color = Color::named("red");
			} break;
			case TRANSITION_CONFIG_VALID_LEVEL_WARNING: {
				text_color = Color::named("yellow");
			} break;
			default: {
				text_color = Color::named("white");
			} break;
		}

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
			connection_layer->draw_set_transform(center_pos, angle, clamped_scale);
		} else {
			// 上方反向显示
			top = (CLAMP(line_count - 1, 1.2, line_count) * char_high) * Vector2(0, 1);
			connection_layer->draw_set_transform(center_pos, angle + Math_PI, clamped_scale);
		}

		for (auto i = 0; i < line_count; i++) {
			String text = texts[i];
			auto string_size = font->get_string_size(text);
			connection_layer->draw_string(font, top + Vector2(-string_size.x / 2.0f, i * string_size.y), text, HORIZONTAL_ALIGNMENT_LEFT, -1, 16, text_color);
		}

		return false;
	};

	foreach_connection_by_names(action);

	if (disconnect_line.size() == 2) {
		connection_layer->draw_set_transform(Vector2(0, 0), 0.0, Vector2(1, 1));
		connection_layer->draw_line(disconnect_line[0], disconnect_line[1], Color::named("royal_blue"), 5, true);
	}

	static bool dirty = false;
	if (!dirty) {
		connection_layer->call_deferred(TNAMEOF(queue_redraw));
		dirty = true;
	} else {
		dirty = false;
	}
}

void FsmEditor::initialize() {
#ifdef GDE_COMPATIBILITY_ENABLED
	if (!HFSMGlobal::is_4_point_2_or_later()) {
		incompatible_apis = {
			"get_scroll_ofs",
			"set_scroll_ofs",
			"set_snap",
			"get_snap",
			"set_use_snap",
			"is_using_snap",
			"get_zoom_hbox",

			"get_connection_input_position",
			"get_connection_output_position",

		};
	} else {
		incompatible_apis = {
			"get_scroll_offset",
			"set_scroll_offset",
			"set_snapping_distance",
			"get_snapping_distance",
			"set_snapping_enabled",
			"is_snapping_enabled",
			"get_menu_hbox",

			"get_input_port_position",
			"get_output_port_position",
		};
	}
#endif //GDE_COMPATIBILITY_ENABLED

	set_name("FsmEditor");
	set_v_size_flags(SIZE_EXPAND_FILL);
	add_valid_connection_type(StateNode::OUT_TYPE, StateNode::IN_TYPE);
	IF_GDM(connect("close_nodes_request", TCALLABLE(_delete_nodes_request));)
	IF_NOT_GDE_COMPATIBLE(connect("close_nodes_request", TCALLABLE(_delete_nodes_request));)
	IF_GDE_COMPATIBLE(connect(HFSMGlobal::is_4_point_2_or_later() ? "close_nodes_request" : "delete_nodes_request", TCALLABLE(_delete_nodes_request));)
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
			if (ctrl->get_class() == decltype(ctrl->get_class())(Control::get_class_static())) {
				TypedArray<Dictionary> signal_conn_list = ctrl->call(TNAMEOF(get_signal_connection_list), "draw");
				if (signal_conn_list.size() != 1) {
					continue;
				}
				if (ctrl->get_mouse_filter() != MOUSE_FILTER_IGNORE) {
					continue;
				}
				connection_layer = ctrl;
				break;
			}
		}
	}
	CRASH_COND_MSG(!connection_layer, "Gets connection_layer of GraphEdit is faild on your Godot version. Please open a issue on \"https://github.com/Daylily-Zeleen/HFSM2/issues\".");

	connection_layer->set_mouse_filter(MOUSE_FILTER_PASS);

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
	mask_hint->set_text(str_localize("Please set up a FSMConfig for selected HFSM node to start edit."));
	mask_panel->add_child(mask_hint);
}

FsmEditor *FsmEditor::create_fsm_editor(HBoxContainer *p_path_btn_container, bool p_debug_mode) {
	auto ret = memnew(FsmEditor(p_debug_mode));
	ret->initialize();
	return ret;
}

Ref<FSMConfig> FsmEditor::get_nested_fsm_config(const Ref<StateConfig> &p_state_config, const Ref<FSMConfig> &p_fsm_config) {
	if (p_fsm_config.is_valid()) {
		auto state_config_list = p_fsm_config->get_state_config_list();
		for (size_t i = 0; i < state_config_list.size(); i++) {
			Ref<StateConfig> sc = state_config_list[i];
			if (sc == p_state_config) {
				return p_fsm_config;
			} else {
				if (sc->get_sub_fsm_config().is_valid()) {
					auto fsm_config = get_nested_fsm_config(p_state_config, sc->get_sub_fsm_config());
					if (fsm_config.is_valid()) {
						return fsm_config;
					}
				}
			}
		}
	}
	return nullptr;
}

void FsmEditor::edit_fsm_config(const Ref<FSMConfig> &p_fsm_config, HBoxContainer *p_path_button_container, const Ref<FSMConfig> &p_root_fsm_config, bool p_as_action) {
#define get_btn_callback(m_fsm_config) TCALLABLE_BIND(edit_fsm_config, m_fsm_config, p_path_button_container, p_root_fsm_config, p_as_action)

	const auto remove_and_free_children = [](Node *p_node, bool (*p_filter)(Node * p_child)) {
		List<Node *> to_remove;
		for (auto i = 0; i < p_node->get_child_count(); ++i) {
			auto child = p_node->get_child(i);
			if (p_filter(child)) {
				to_remove.push_back(child);
			}
		}
		while (!to_remove.is_empty()) {
			auto bck = to_remove.back()->get();
			to_remove.pop_back();
			p_node->remove_child(bck);
			bck->queue_free();
		}
	};

	if (debug_mode) {
		__set_blocking_redraw(true);
		// 处理画面显示
		// 断连接
		foreach_connection_by_names([this](const StringName &p_from, const StringName &p_to) {
			disconnect_node(p_from, 0, p_to, 0);
			return false;
		});

		// 移除状态
		remove_and_free_children(this, [](Node *p_child) { return cast_to<StateNode>(p_child) != nullptr; });

		// 构建目标的状态机
		__set_current_fsm_config(p_fsm_config, p_root_fsm_config);

		//  路径按钮处理
		//  清除路径列表
		remove_and_free_children(p_path_button_container, [](Node *p_child) { return cast_to<Button>(p_child) != nullptr; });

		if (p_fsm_config.is_valid()) {
			// 处理路径按钮
			List<Button *> path_btn_list;
			Ref<FSMConfig> fc = p_fsm_config;
			Ref<StateConfig> nsc = p_fsm_config->get_nested_state_config();
			while (nsc.is_valid()) {
				auto btn = memnew(Button);
				btn->set_text(nsc->get_state_name());
				btn->connect("pressed", get_btn_callback(fc), CONNECT_DEFERRED);

				path_btn_list.push_front(btn);

				fc = get_nested_fsm_config(nsc, p_root_fsm_config);
				nsc = fc->get_nested_state_config();
			}
			auto root_btn = memnew(Button);
			root_btn->set_text("root");
			root_btn->connect("pressed", get_btn_callback(fc), CONNECT_DEFERRED);
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
			auto state_config_list = p_fsm_config->get_state_config_list();
			for (auto i = 0; i < state_config_list.size(); i++) {
				Ref<StateConfig> sc = state_config_list[i];
				auto sn = create_state_node(sc);
				sc->set_state_node(sn);
				add_child(sn);
			}

			// 连接
			auto transition_config_list = p_fsm_config->get_transition_config_list();
			for (auto i = 0; i < transition_config_list.size(); i++) {
				Ref<TransitionConfig> tc = transition_config_list[i];
				StringName from = tc->get_from_state_config()->get_state_node()->get_name();
				StringName to = tc->get_to_state_config()->get_state_node()->get_name();

				call_deferred(TNAMEOF(connect_node), from, 0, to, 0);
			}
		}

		__set_blocking_redraw(false);
		propagate_notification(NOTIFICATION_CHILD_ORDER_CHANGED);
	} else if (!p_as_action) {
		if (current_fsm_config.is_null() && p_fsm_config.is_null()) {
			return;
		}

		__set_blocking_redraw(true);
		// 处理画面显示
		// 断连接
		foreach_connection_by_names([this](const StringName &p_from, const StringName &p_to) {
			disconnect_node(p_from, 0, p_to, 0);
			return false;
		});

		// 移除状态
		remove_and_free_children(this, [](Node *p_child) { return cast_to<StateNode>(p_child) != nullptr; });

		// 构建目标的状态机
		__set_current_fsm_config(p_fsm_config, p_root_fsm_config);

		//  路径按钮处理
		//  清除路径列表
		remove_and_free_children(p_path_button_container, [](Node *p_child) { return cast_to<Button>(p_child) != nullptr; });

		if (p_fsm_config.is_valid()) {
			// 处理路径按钮
			List<Button *> path_btn_list;
			Ref<FSMConfig> fc = p_fsm_config;
			Ref<StateConfig> nsc = p_fsm_config->get_nested_state_config();
			while (nsc.is_valid()) {
				auto btn = memnew(Button);
				btn->set_text(nsc->get_state_name());
				btn->connect("pressed", get_btn_callback(fc), CONNECT_DEFERRED);

				path_btn_list.push_front(btn);

				fc = get_nested_fsm_config(nsc, p_root_fsm_config);
				nsc = fc->get_nested_state_config();
			}
			auto root_btn = memnew(Button);
			root_btn->set_text("root");
			root_btn->connect("pressed", get_btn_callback(fc), CONNECT_DEFERRED);
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
			auto state_config_list = p_fsm_config->get_state_config_list();
			for (auto i = 0; i < state_config_list.size(); i++) {
				Ref<StateConfig> sc = state_config_list[i];
				auto sn = create_state_node(sc, p_fsm_config);
				sc->_set_state_node(sn);
				add_child(sn);
			}

			// 连接
			auto transition_config_list = p_fsm_config->get_transition_config_list();
			for (auto i = 0; i < transition_config_list.size(); i++) {
				Ref<TransitionConfig> tc = transition_config_list[i];
				StringName from = tc->get_from_state_config()->get_state_node()->get_name();
				StringName to = tc->get_to_state_config()->get_state_node()->get_name();

				call_deferred(TNAMEOF(connect_node), from, 0, to, 0);
			}
		}

		__set_blocking_redraw(false);

		propagate_notification(NOTIFICATION_CHILD_ORDER_CHANGED);
	} else {
		if (current_fsm_config.is_null() && p_fsm_config.is_null()) {
			return;
		}

		HFSM_EDITOR_CREATE_ACTION("Edit Sub-FSM");
		ADD_DO_METHOD(this, __set_blocking_redraw, true);
		ADD_UNDO_METHOD(this, __set_blocking_redraw, true);
		// 处理画面显示
		// 断连接
		foreach_connection_by_names([this, undo_redo](const StringName &p_from, const StringName &p_to) {
			ADD_DO_METHOD(this, disconnect_node, p_from, 0, p_to, 0);
			ADD_UNDO_DEFERRED_CALL_METHOD(this, connect_node, p_from, 0, p_to, 0);
			return false;
		});

		// 移除状态
		for (auto i = 0; i < get_child_count(); i++) {
			if (auto sn = Object::cast_to<StateNode>(get_child(i))) {
				ADD_DO_METHOD(this, remove_child, sn);
				ADD_UNDO_METHOD(this, add_child, sn);
				ADD_UNDO_REFERENCE(sn);
			}
		}

		// 构建目标的状态机
		ADD_DO_METHOD(this, __set_current_fsm_config, p_fsm_config, p_root_fsm_config);
		ADD_UNDO_METHOD(this, __set_current_fsm_config, current_fsm_config, current_root_fsm_config);

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

		if (p_fsm_config.is_valid()) {
			// 处理路径按钮
			List<Button *> path_btn_list;
			Ref<FSMConfig> fc = p_fsm_config;
			Ref<StateConfig> nsc = p_fsm_config->get_nested_state_config();
			while (nsc.is_valid()) {
				auto btn = memnew(Button);
				btn->set_text(nsc->get_state_name());
				btn->connect("pressed", get_btn_callback(fc), CONNECT_DEFERRED);

				path_btn_list.push_front(btn);

				fc = get_nested_fsm_config(nsc, p_root_fsm_config);
				nsc = fc->get_nested_state_config();
			}
			auto root_btn = memnew(Button);
			root_btn->set_text("root");
			root_btn->connect("pressed", get_btn_callback(fc), CONNECT_DEFERRED);
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
			auto state_config_list = p_fsm_config->get_state_config_list();
			for (auto i = 0; i < state_config_list.size(); i++) {
				Ref<StateConfig> sc = state_config_list[i];
				auto old_state_node = sc->get_state_node();
				auto sn = create_state_node(sc, p_fsm_config);
				ADD_DO_REFERENCE(sn);

				ADD_DO_METHOD(sc.ptr(), _set_state_node, sn);
				ADD_UNDO_METHOD(sc.ptr(), _set_state_node, old_state_node);
				ADD_DO_METHOD(this, add_child, sn);
				ADD_UNDO_METHOD(this, remove_child, sn);
			}

			// 连接
			auto transition_config_list = p_fsm_config->get_transition_config_list();
			for (auto i = 0; i < transition_config_list.size(); i++) {
				Ref<TransitionConfig> tc = transition_config_list[i];
				StringName from = tc->get_from_state_config()->get_state_node()->get_name();
				StringName to = tc->get_to_state_config()->get_state_node()->get_name();

				ADD_DO_DEFERRED_CALL_METHOD(this, connect_node, from, 0, to, 0);
				ADD_UNDO_METHOD(this, disconnect_node, from, 0, to, 0);
			}
		}

		ADD_DO_METHOD(this, __set_blocking_redraw, false);
		ADD_UNDO_METHOD(this, __set_blocking_redraw, false);

		ADD_DO_METHOD(this, propagate_notification, NOTIFICATION_CHILD_ORDER_CHANGED);
		ADD_UNDO_METHOD(this, propagate_notification, NOTIFICATION_CHILD_ORDER_CHANGED);

		ADD_DO_METHOD(this, queue_redraw);
		ADD_UNDO_METHOD(this, queue_redraw);

		COMMIT_ACTION();
	}
}

String FsmEditor::get_variable_expression_config_valid_and_text(const Ref<VariableExpressionConfig> &p_ver, TransitionConfigValidLevel &r_valid) const {
	r_valid = TRANSITION_CONFIG_VALID_LEVEL_ERROR;
	auto vc = p_ver->get_variable_config();
	if (vc.is_valid()) {
		if (!current_root_fsm_config->get_variable_config_list().has(vc)) {
			r_valid = TRANSITION_CONFIG_VALID_LEVEL_ERROR;
			return vformat(str_localize(R"("VariableConfig" %s is not contained in editing HFSM.)"), vc->get_variable_name());
		}

		if (vc->get_type() != Variant::NIL) {
			auto get_op_text = [p_ver]() -> String {
				switch (p_ver->get_comparator()) {
					case VariableExpressionConfig::Comparator::COMPARATOR_EQUAL:
						return " == ";
					case VariableExpressionConfig::Comparator::COMPARATOR_NOT_EQUAL:
						return " != ";
					case VariableExpressionConfig::Comparator::COMPARATOR_GREATER:
						return " > ";
					case VariableExpressionConfig::Comparator::COMPARATOR_GREATER_EQUAL:
						return " >= ";
					case VariableExpressionConfig::Comparator::COMPARATOR_LESS:
						return " < ";
					case VariableExpressionConfig::Comparator::COMPARATOR_LESS_EQUAL:
						return " <= ";
					default:
						return " invalid operator";
				}
			};
			if (p_ver->is_variable_as_value()) {
				if (auto value = cast_to<VariableConfig>(p_ver->get_value())) {
					if (!current_root_fsm_config->get_variable_config_list().has(value)) {
						r_valid = TRANSITION_CONFIG_VALID_LEVEL_ERROR;
						return vformat(str_localize(R"("VariableConfig" %s is not contained in editing HFSM.)"), vc->get_variable_name());
					}

					r_valid = Variant::can_convert(Variant::Type(vc->get_type()), Variant::Type(value->get_type())) ? TRANSITION_CONFIG_VALID_LEVEL_NONE : TRANSITION_CONFIG_VALID_LEVEL_ERROR;
					if (r_valid == TRANSITION_CONFIG_VALID_LEVEL_NONE) {
						return String(vc->get_variable_name()) + get_op_text() + String(value->get_variable_name());
					} else {
						return str_localize(R"XXX("value" can't convert to the type of "VariableConfig".)XXX");
					}
				} else {
					return str_localize(R"XXX("value" is not a valid "VariableConfig".)XXX");
				}
			} else {
				r_valid = Variant::can_convert(p_ver->get_value().get_type(), Variant::Type(vc->get_type())) ? TRANSITION_CONFIG_VALID_LEVEL_NONE : TRANSITION_CONFIG_VALID_LEVEL_ERROR;
				if (r_valid == TRANSITION_CONFIG_VALID_LEVEL_NONE) {
					String value_text = "";
					switch (vc->get_type()) {
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
					return String(vc->get_variable_name()) + get_op_text() + value_text;
				} else {
					return str_localize("\"value\" can't convert to the type "
										"of \"variable_config\".");
				}
			}
		} else {
			r_valid = TRANSITION_CONFIG_VALID_LEVEL_NONE;
			String vrn = { vc->get_variable_name() };
			switch (p_ver->get_trigger_type()) {
				case VariableExpressionConfig::TRIGGER_TYPE_NORMAL:
					return str_localize("Trigger: ") + vrn;
					break;
				case VariableExpressionConfig::TRIGGER_TYPE_SOLO:
					return str_localize("Solo Trigger: ") + vrn;
					break;
				case VariableExpressionConfig::TRIGGER_TYPE_UNION:
					return str_localize("Union Trigger: ") + vrn;
					break;
				default:
					r_valid = TRANSITION_CONFIG_VALID_LEVEL_ERROR;
					return str_localize("Invalid Trigger Type:") + vrn;
					break;
			}
		}
	} else {
		return str_localize("Has not valid 'variable_config'");
	}
}

List<String> FsmEditor::get_transition_config_valid_and_texts(const Ref<TransitionConfig> &p_transition_config, TransitionConfigValidLevel &r_valid) const {
	List<String> ret;
	r_valid = TRANSITION_CONFIG_VALID_LEVEL_ERROR;

	switch (p_transition_config->get_type()) {
		case TransitionConfig::TRANSITION_TYPE_AUTO: {
			r_valid = (p_transition_config->get_auto_mode() >= 0 && p_transition_config->get_auto_mode() < TransitionConfig::AUTO_TRANSIT_MODE_MAX) ? TRANSITION_CONFIG_VALID_LEVEL_NONE : TRANSITION_CONFIG_VALID_LEVEL_ERROR;
			switch (p_transition_config->get_auto_mode()) {
				case TransitionConfig::AUTO_TRANSIT_MODE_DELAY_TIMER: {
					ret.push_back(str_localize("Auto: ") + vformat(str_localize("Delay %d msec."), p_transition_config->get_auto_delay_msec()));
				} break;
				case TransitionConfig::AUTO_TRANSIT_MODE_FSM_EXIT: {
					ret.push_back(str_localize("Auto: ") + str_localize("When sub FSM exit."));
				} break;
				case TransitionConfig::AUTO_TRANSIT_MODE_MANUAL: {
					ret.push_back(str_localize("Auto: ") + str_localize("After calling \"manual_exit()\"."));
				} break;
				case TransitionConfig::AUTO_TRANSIT_MODE_UPDATE_TIMES: {
					ret.push_back(str_localize("Auto: ") + vformat(str_localize("After \"_update()\" being called %d times."), itos(p_transition_config->get_auto_times())));
				} break;
				case TransitionConfig::AUTO_TRANSIT_MODE_PHYSICS_UPDATE_TIMES: {
					ret.push_back(str_localize("Auto: ") + vformat(str_localize("After \"_physics_update()\" being called %d times."), itos(p_transition_config->get_auto_times())));
				} break;
				case TransitionConfig::AUTO_TRANSIT_MODE_ANIMATION_FINISH: {
					auto anim = p_transition_config->get_from_state_config()->get_animation_name();
					if (String(anim).strip_edges().is_empty()) {
						anim = p_transition_config->get_from_state_config()->get_state_name();
					}
					ret.push_back(str_localize("Auto: ") + vformat(str_localize("After playing animation \"%s\" finish."), anim));
					if (auto hfsm = HFSMEditorPlugin::get_singleton()->get_hfsm_editor()->get_editing_hfsm()) {
						if (auto anim_player = hfsm->get_animation_player()) {
							if (!anim_player->has_animation(anim)) {
								r_valid = TRANSITION_CONFIG_VALID_LEVEL_ERROR;
								ret.push_back(str_localize("Error: The AnimationPlayer which is setted to editing HFSM has not animation \"%s\"."));
							}
						} else {
							r_valid = TRANSITION_CONFIG_VALID_LEVEL_WARNING;
							ret.push_back(str_localize("Warning: The editing HFSM has not setted an AnimationPlayer."));
						}
					}
				} break;
				default:
					ret.push_back(String::utf8("不应发生: 非法自动转换类型。"));
					break;
			}
		} break;
		case TransitionConfig::TRANSITION_TYPE_EXPRESSION: {
			// 非运行时无法检测表达式合法性， 故只要表达式不为空就认为合法
			if (p_transition_config->get_expression_text().is_empty()) {
				ret.push_back(str_localize("Empty expression!"));
			} else {
				r_valid = TRANSITION_CONFIG_VALID_LEVEL_NONE;
				ret.push_back(String("Expression: ") + p_transition_config->get_expression_text());
				ret.push_back(String("Comment: ") + p_transition_config->get_expression_comment());
			}
		} break;
		case TransitionConfig::TRANSITION_TYPE_VARIABLE_EXPRESSIONS: {
			auto variable_expression_config_list = p_transition_config->get_variable_expression_config_list();
			if (variable_expression_config_list.size() > 0) {
				r_valid = TRANSITION_CONFIG_VALID_LEVEL_NONE;
				ret.push_back(str_localize("Variable Expressions: ") + String(p_transition_config->is_and_mode() ? "AND" : "OR"));
				for (auto i = 0; i < variable_expression_config_list.size(); i++) {
					Ref<VariableExpressionConfig> vec = variable_expression_config_list[i];
					if (vec.is_valid()) {
						ret.push_back(get_variable_expression_config_valid_and_text(vec, r_valid));
						if (r_valid == TRANSITION_CONFIG_VALID_LEVEL_ERROR) {
							break;
						}
					} else {
						r_valid = TRANSITION_CONFIG_VALID_LEVEL_ERROR;
						ret.push_back(vformat(str_localize("Invalid \"VariableExpressionConfig\", index %d."), i));
						break;
					}
				}
			} else {
				ret.push_back(str_localize("Variable Expressions: ") + str_localize("Have not valid Variable Expression."));
			}
		} break;
#ifdef FULL_VERSION
		case TransitionConfig::TRANSITION_TYPE_SCRIPT: {
			auto transition_script = p_transition_config->get_transition_script();
			if (transition_script.is_valid()) {
				r_valid = TRANSITION_CONFIG_VALID_LEVEL_NONE;

				auto base = transition_script->get_instance_base_type();
				if (base == StringName(Transition::get_class_static())) {
					r_valid = TRANSITION_CONFIG_VALID_LEVEL_NONE;
				}
				IF_GDM(else {
					r_valid = ClassDB::is_parent_class(base, Transition::get_class_static()) ? TRANSITION_CONFIG_VALID_LEVEL_NONE : TRANSITION_CONFIG_VALID_LEVEL_ERROR;
				})

				if (r_valid == TRANSITION_CONFIG_VALID_LEVEL_NONE) {
					bool existing = false;
					IF_GDM(existing = FileAccess::exists(transition_script->get_path());)
					IF_GDE(existing = FileAccess::file_exists(transition_script->get_path());)
					ret.push_back(
							str_localize(existing ? "Script: " : "Built-in Script: ") + transition_script->get_path());
				} else {
					ret.push_back(str_localize("Script isn't extends from \"Transition\"."));
				}
			} else {
				ret.push_back(str_localize("Script is invalid!"));
			}
		} break;
#endif //FULL_VERSION
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
			connection_layer->queue_redraw();
		} break;
		case NOTIFICATION_READY: {
			set_process(false);
			connection_layer->connect("draw", TCALLABLE(_draw_layer_draw));

			connect("gui_input", TCALLABLE(_gui_input_internal));
			connect("end_node_move", TCALLABLE(_end_node_move));
			set_editor_inspector_signal_connected(true);

			propagate_notification(NOTIFICATION_THEME_CHANGED);
		} break;
		case NOTIFICATION_DRAW: {
			connection_layer->queue_redraw();
		} break;
		case EditorSettings::NOTIFICATION_EDITOR_SETTINGS_CHANGED:
		case NOTIFICATION_THEME_CHANGED: {
			auto base_control = cast_to<Control>(EditorInterface::get_singleton()->get_base_control());
			font = base_control->get_theme_default_font();
			activity_color = EditorInterface::get_singleton()->get_base_control()->get_theme_color("activity", "GraphEdit");
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

			if (!p_deactive_all && sn->get_state_config()->get_state_name() == p_state_name) {
				sn->set_debug_actived(true);
				sn->set_self_modulate(Color::named("GREEN"));
				next_activated = sn;
			} else {
				sn->set_debug_actived(false);
				sn->set_self_modulate(Color::named("WHITE"));
			}
		}
	}

	foreach_connection_by_names([this](const StringName &p_from, const StringName &p_to) {
		set_connection_activity(p_from, 0, p_to, 0, 0.0);
		return false;
	});

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
			sn->get_state_config()->emit_signal(SNAME("changed"));
		}
	}
	queue_redraw();
	queuing_refresh = false;
}

void FsmEditor::__queue_redraw_request() {
	if (!queuing_redraw) {
		call_deferred(TNAMEOF(__queue_redraw));
		queuing_redraw = true;
	}
}

void FsmEditor::__queue_redraw() {
	queue_redraw();
	queuing_redraw = false;
}

void FsmEditor::queue_refresh() {
	if (!queuing_refresh) {
		call_deferred(TNAMEOF(__queue_refresh));
		queuing_refresh = true;
	}
}

}; // namespace HFSM2
