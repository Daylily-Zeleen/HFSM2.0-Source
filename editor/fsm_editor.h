/**************************************************************************/
/*  fsm_editor.h                                                          */
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

#pragma once

#include "../hfsm_global.h"

#ifdef GDEXTENSION_BUILD
#include <godot_cpp/classes/graph_edit.hpp>
#include <godot_cpp/classes/label.hpp>
#include <godot_cpp/classes/panel.hpp>
#include <godot_cpp/classes/popup_menu.hpp>

#include <godot_cpp/classes/font.hpp>
using namespace godot;

#else // GDEXTENSION_BUILD
#include <scene/gui/graph_edit.h>
#endif // GDEXTENSION_BUILD

namespace godot {
class EditorInspector;
} //namespace godot

namespace HFSM2 {
class StateNode;
class TransitionConfig;
class FSMConfig;
class StateConfig;

class FSMEditor : public GraphEdit {
	GDCLASS(FSMEditor, GraphEdit)
protected:
	static void _bind_methods();
	void _notification(int p_what);

private:
	enum MenuOption {
		ITEM_ADD_STATE,
		ITEM_CUT_STATE,
		ITEM_COPY_STATES,
		ITEM_PASTE_STATES,
		ITEM_DUPLICATE_STATES,
		ITEM_DELETE,
		ITEM_CONVERT_TO_FSM,
	};

	enum TransitionConfigValidLevel {
		TRANSITION_CONFIG_VALID_LEVEL_NONE,
		TRANSITION_CONFIG_VALID_LEVEL_WARNING,
		TRANSITION_CONFIG_VALID_LEVEL_ERROR,
	};

	enum ShortCutKeyCode {
		KEYCODE_ADD_STATE = int(KEY(A)) | int(KEY_MASK(SHIFT)),
		KEYCODE_CUT_STATE = int(KEY(X)) | int(KEY_MASK(CMD_OR_CTRL)),
		KEYCODE_COPY_STATES = int(KEY(C)) | int(KEY_MASK(CMD_OR_CTRL)),
		KEYCODE_PASTE_STATES = int(KEY(V)) | int(KEY_MASK(CMD_OR_CTRL)),
		KEYCODE_DUPLICATE_STATES = int(KEY(D)) | int(KEY_MASK(CMD_OR_CTRL)),
#ifdef GDEXTENSION_BUILD
		KEYCODE_DELETE_STATES = int(KEY_DELETE),
#else
		KEYCODE_DELETE_STATES = int(Key::KEY_DELETE),
#endif // GDEXTENSION_BUILD
	};

	PopupMenu *menu = nullptr;
	Panel *mask_panel = nullptr;
	Label *mask_hint = nullptr;
	StateNode *__hovering_state_node = nullptr;

	Control *draw_layer = nullptr;
	// ==============
	Ref<FSMConfig> current_root_fsm_config;
	Ref<FSMConfig> current_fsm_config;
	TypedArray<StateConfig> copied_state_config_list; // = TypedArray<StateConfig>();
	TypedArray<StringName> selected_state_name_list; // = TypedArray<StringName>();
	TypedArray<TransitionConfig> selected_transition_config_list; // = TypedArray<TransitionConfig>();
	TypedArray<TransitionConfig> copied_transition_config_list; // = TypedArray<TransitionConfig>();
	TypedArray<StringName> bakcup_selected_state_name_list; // = TypedArray<StringName>();
	Color activity_color;
	PackedVector2Array disconnect_line = PackedVector2Array();
	const Vector2 SCALE_DRAGGER_SIZE = Vector2(30.0f, 30.0f);
	const int32_t HOTZONE_RADIUS = 10;
	const PackedVector2Array TRIANGLE_POINTS;
	const Vector2 DUPLICATE_OFFSET = Vector2(50, 50);
	Ref<Font> font;

	// ========== UNDO REDO =========
	void __set_current_fsm_config(const Ref<FSMConfig> &p_to_set, const Ref<FSMConfig> &p_root);
	void __set_selected_state_name_list(const TypedArray<StringName> &p_to_set);
	void __set_selected_transition_config_list(const TypedArray<TransitionConfig> &p_to_set);
	void __set_copied_transition_list(const TypedArray<TransitionConfig> &p_to_set);
	void __set_copied_state_config_list(const TypedArray<StateConfig> &p_to_set);
	void __select_state_nodes(const TypedArray<StringName> &p_to_select_State_name_list);
	void __select_mamually(const TypedArray<StateNode> &p_target_nodes);
	void __set_blocking_redraw(bool p_blocking_redraw) { blocking_redraw = p_blocking_redraw; }
	// ========功能=========
	void try_disconnect(const Vector2 &p_pos1, const Vector2 &p_pos2);
	TypedArray<StateNode> get_selected_state_nodes();

	bool is_judge(const Vector2 &p_apos1, const Vector2 &p_apos2, const Vector2 &p_bpos1, const Vector2 &p_bpos2);
	TypedArray<TransitionConfig> try_select_transitions_at_pos(const Vector2 &pos);

	PackedVector2Array get_connection_line_with_zoom_for_display(StateNode *p_from, StateNode *to);
	Ref<TransitionConfig> get_transition_config(StateNode *p_from, StateNode *p_to);
	bool is_node_hotzone(Object *p_in_node, int64_t p_in_port, const Vector2 &p_mouse_position);
	StateNode *create_state_node(const Ref<StateConfig> &p_state_config, const Ref<FSMConfig> &p_fsm_config = nullptr);

	StateNode *get_top_state_node_which_hovering();
	TypedArray<StateConfig> get_selected_state_config_list();

	// ======= CALLBACK ==========
	void _draw_layer_draw();
	void _popup_menu_id_pressed(int32_t p_id);
	void _delete_nodes_request(const Array &p_nodes);
	void _connection_request(const StringName &p_from, int p_from_slot, const StringName &p_to, int p_to_slot);
	void _popup_request(const Vector2 &p_position);
	void _transition_config_updated();
	void _node_selected(Object *p_node);
	void _node_deselected(Object *p_node);
	void _disconnect_inspecting_transition_config();
	void _gui_input_internal(const Ref<InputEvent> &p_event);
	void _end_node_move();
	void _edit_sub_fsm_requested(const Ref<FSMConfig> &p_sub_fsm_config);
	void _state_node_reconnected_requested(const StringName &p_old_name, const StringName &p_new_name);
	void _debug_tween_activity(float p_activity, const StringName &p_from, const StringName &p_to);
	// ======== 检查 ==========
	String get_variable_expression_config_valid_and_text(const Ref<class VariableExpressionConfig> &p_ver, TransitionConfigValidLevel &r_valid) const;
	List<String> get_transition_config_valid_and_texts(const Ref<TransitionConfig> &p_transition_config, TransitionConfigValidLevel &r_valid) const;
	// ==================
	String str_localize(const String &p_en_key) const;

	// void draw_internal();
	PackedVector2Array get_connection_line_internal(const Vector2 &p_from, const Vector2 &p_to) const;

	bool connection_dirty = false;
	bool selection_dirty = false;
	bool blocking_redraw = false;

	Ref<TransitionConfig> inspecting_transition_config;

	void initialize();

	bool is_blocking_redraw() const { return blocking_redraw; }

	// debug
	const bool debug_mode = false;

	float debug_activity = 0.0;
	StringName debug_activity_from = "";
	StringName debug_activity_to = "";

	Ref<FSMConfig> get_nested_fsm_config(const Ref<StateConfig> &p_state_config, const Ref<FSMConfig> &p_root_fsm_config = nullptr);

	StateNode *_get_state_node(const NodePath &p_path);

	bool queuing_refresh = false;
	void __queue_refresh();
	bool queuing_redraw = false;
	void __queue_redraw_request();
	void __queue_redraw();

	template <typename TFuncAction, std::enable_if<std::is_invocable_r_v<bool, TFuncAction, StringName, StringName>> *valve = nullptr>
	void foreach_connection_by_names(TFuncAction p_action) {
		IF_GDE({
			Array conn_list = call("get_connection_list");
			for (auto i = 0; i < conn_list.size(); i++) {
				Dictionary conn = conn_list[i];
				const StringName from = conn.has("from") ? conn["from"] : conn["from_node"];
				const StringName to = conn.has("to") ? conn["to"] : conn["to_node"];

				if (p_action(from, to)) {
					return;
				}
			}
		})
		IF_GDM({
			List<Connection> conn_list;
			get_connection_list(&conn_list);
			for (auto conn : conn_list) {
				if (p_action(conn.from_node, conn.to_node)) {
					return;
				}
			}
		})
	}

	template <typename TFuncAction, std::enable_if<std::is_invocable_r_v<bool, TFuncAction, StateNode *, StateNode *>> *valve = nullptr>
	void foreach_connection_by_nodes(TFuncAction p_action) {
		const auto action = [this, p_action](const StringName &p_from, const StringName &p_to) -> bool {
			auto from = _get_state_node({ p_from });
			auto to = _get_state_node({ p_to });
			return p_action(from, to);
		};
		foreach_connection_by_names(action);
	}

#ifdef GDE_COMPATIBILITY_ENABLED
	// Incompatible APIs
	struct IncompatibleAPIs {
		StringName get_scroll_ofs;
		StringName set_scroll_ofs;
		StringName set_snap;
		StringName get_snap;
		StringName set_use_snap;
		StringName is_using_snap;
		StringName get_zoom_hbox;
		// For StateNode
		StringName state_node_get_input_port_position;
		StringName state_node_get_output_port_position;
	};

	IncompatibleAPIs incompatible_apis;
#endif // GDE_COMPATIBILITY_ENABLED

	Vector2 _get_scroll_offset() {
		IF_GDM(return get_scroll_offset();)
		IF_NOT_GDE_COMPATIBLE(return get_scroll_offset());
		IF_GDE_COMPATIBLE(return call(incompatible_apis.get_scroll_ofs));
	}

	void _set_scroll_offset(const Vector2 &p_offset) {
		IF_GDM(set_scroll_offset(p_offset);)
		IF_NOT_GDE_COMPATIBLE(set_scroll_offset(p_offset);)
		IF_GDE_COMPATIBLE(call(incompatible_apis.set_scroll_ofs, p_offset);)
	}

	void _set_snapping_distance(int p_snapping_distance) {
		IF_GDM(set_snapping_distance(p_snapping_distance);)
		IF_NOT_GDE_COMPATIBLE(set_snapping_distance(p_snapping_distance);)
		IF_GDE_COMPATIBLE(call(incompatible_apis.set_snap, p_snapping_distance);)
	}

	int _get_snapping_distance() {
		IF_GDM(return get_snapping_distance();)
		IF_NOT_GDE_COMPATIBLE(return get_snapping_distance();)
		IF_GDE_COMPATIBLE(return call(incompatible_apis.get_snap);)
	}

	void _set_snapping_enabled(bool p_enabled) {
		IF_GDM(set_snapping_enabled(p_enabled);)
		IF_NOT_GDE_COMPATIBLE(set_snapping_enabled(p_enabled);)
		IF_GDE_COMPATIBLE(call(incompatible_apis.set_use_snap, p_enabled);)
	}
	bool _is_snapping_enabled() {
		IF_GDM(return is_snapping_enabled();)
		IF_NOT_GDE_COMPATIBLE(return is_snapping_enabled();)
		IF_GDE_COMPATIBLE(return call(incompatible_apis.is_using_snap);)
	}

	HBoxContainer *_get_menu_hbox() {
		IF_GDM(return get_menu_hbox();)
		IF_NOT_GDE_COMPATIBLE(return get_menu_hbox();)
		IF_GDE_COMPATIBLE(return Object::cast_to<HBoxContainer>(call(incompatible_apis.get_zoom_hbox));)
	}

	Vector2 _state_node_get_output_port_position(StateNode *p_state_node, int p_port_idx) const;
	Vector2 _state_node_get_input_port_position(StateNode *p_state_node, int p_port_idx) const;

public:
	void queue_refresh();

	FSMEditor(bool p_debug_mode = false);

#ifdef GDEXTENSION_BUILD
	PackedVector2Array GD_(get_connection_line)(const Vector2 &p_from, const Vector2 &p_to) const override { return get_connection_line_internal(p_from, p_to); }
	bool _is_in_input_hotzone(Object *p_in_node, int64_t p_in_port, const Vector2 &p_mouse_position) { return is_node_hotzone(p_in_node, p_in_port, p_mouse_position); }
	bool _is_in_output_hotzone(Object *p_in_node, int64_t p_in_port, const Vector2 &p_mouse_position) { return is_node_hotzone(p_in_node, p_in_port, p_mouse_position); }
#else // GDEXTENSION_BUILD
	PackedVector2Array get_connection_line(const Vector2 &p_from, const Vector2 &p_to) override { return get_connection_line_internal(p_from, p_to); }
	bool is_in_input_hotzone(GraphNode *p_in_node, int p_in_port, const Vector2 &p_mouse_position, const Vector2i &p_port_size) override { return is_node_hotzone(p_in_node, p_in_port, p_mouse_position); }
	bool is_in_output_hotzone(GraphNode *p_in_node, int p_in_port, const Vector2 &p_mouse_position, const Vector2i &p_port_size) override { return is_node_hotzone(p_in_node, p_in_port, p_mouse_position); }
#endif // GDEXTENSION_BUILD

	// TODO:: p_as_action ?? currently change target Fsm is not action, to avoid mark scene as dirty and request save.
	void edit_fsm_config(const Ref<FSMConfig> &p_fsm_config, HBoxContainer *p_path_button_container, const Ref<FSMConfig> &p_root_config, bool p_as_action = false);

	void debug_highlight_active_state(const StringName &p_state_name, bool p_deactive_all);

	static FSMEditor *create_fsm_editor(HBoxContainer *p_path_btn_container, bool p_debug_mode);
};

#define GET_EDITOR_INSPECTOR() EditorInterface::get_singleton()->get_inspector()
#define INSPECT_OBJECT(p_object) EditorInterface::get_singleton()->inspect_object(p_object)
#define GET_EDITOR_THEME() EditorInterface::get_singleton()->get_editor_theme()

}; // namespace HFSM2
