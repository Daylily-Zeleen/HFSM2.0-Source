#pragma once

#include "../hfsm_global.h"

#ifdef GDEXTENSION_BUILD
#include <godot_cpp/classes/graph_edit.hpp>
#include <godot_cpp/classes/label.hpp>
#include <godot_cpp/classes/panel.hpp>
#include <godot_cpp/classes/popup_menu.hpp>

using namespace godot;

#else // GDEXTENSION_BUILD
#include <scene/gui/graph_edit.h>

#endif // GDEXTENSION_BUILD

namespace Hfsm {
class StateNode;
class TransitionConfig;
class FSMConfig;
class StateConfig;

class FsmEditor : public GraphEdit {
	GDCLASS(FsmEditor, GraphEdit)
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

	PackedVector2Array get_connection_line_with_zoom(StateNode *p_from, StateNode *to);
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

public:
	void queue_refresh();

	FsmEditor(bool p_debug_mode = false);
	static FsmEditor *create_fsm_editor(HBoxContainer *p_path_btn_container, bool p_debug_mode);

#ifdef GDEXTENSION_BUILD
	PackedVector2Array GD_(get_connection_line)(const Vector2 &p_from, const Vector2 &p_to) const override { return get_connection_line_internal(p_from, p_to); }
	bool _is_in_input_hotzone(Object *p_in_node, int64_t p_in_port, const Vector2 &p_mouse_position) { return is_node_hotzone(p_in_node, p_in_port, p_mouse_position); }
	bool _is_in_output_hotzone(Object *p_in_node, int64_t p_in_port, const Vector2 &p_mouse_position) { return is_node_hotzone(p_in_node, p_in_port, p_mouse_position); }
#else // GDEXTENSION_BUILD
	PackedVector2Array get_connection_line(const Vector2 &p_from, const Vector2 &p_to) override { return get_connection_line_internal(p_from, p_to); }
	bool is_in_input_hotzone(GraphNode *p_in_node, int p_in_port, const Vector2 &p_mouse_position, const Vector2i &p_port_size) override { return is_node_hotzone(p_in_node, p_in_port, p_mouse_position); }
	bool is_in_output_hotzone(GraphNode *p_in_node, int p_in_port, const Vector2 &p_mouse_position, const Vector2i &p_port_size) override { return is_node_hotzone(p_in_node, p_in_port, p_mouse_position); }
#endif // GDEXTENSION_BUILD

	void edit_fsm_config(const Ref<FSMConfig> &p_fsm_config, HBoxContainer *p_path_button_container, const Ref<FSMConfig> &p_root_config);

	void debug_highlight_active_state(const StringName &p_state_name, bool p_deactive_all);
};

}; // namespace Hfsm
