#pragma once

#include <godot_cpp/classes/button.hpp>
#include <godot_cpp/classes/editor_undo_redo_manager.hpp>
#include <godot_cpp/classes/font.hpp>
#include <godot_cpp/classes/graph_edit.hpp>
#include <godot_cpp/classes/input_event.hpp>
#include <godot_cpp/classes/label.hpp>
#include <godot_cpp/classes/panel.hpp>
#include <godot_cpp/classes/popup_menu.hpp>


#include "core/transitions/variable_expressions/variable_expression_res.hpp"

using namespace godot;

namespace Hfsm {
class StateNode;
class TransitionRes;
class FsmRes;
class StateRes;

class StateNodesEditor : public GraphEdit {
	GDCLASS(StateNodesEditor, GraphEdit)
protected:
	static void _bind_methods();

private:
	enum {
		ITEM_ADD_STATE,
		ITEM_CUT_STATE,
		ITEM_COPY_STATES,
		ITEM_PASTE_STATES,
		ITEM_DUPLICATE_STATES,
		ITEM_DELETE,
		ITEM_CONVERT_TO_FSM,
	};

	enum {
		KEYCODE_ADD_STATE = int(KEY_A) | KEY_MASK_SHIFT,
		KEYCODE_CUT_STATE = int(KEY_X) | KEY_MASK_CTRL,
		KEYCODE_COPY_STATES = int(KEY_C) | KEY_MASK_CTRL,
		KEYCODE_PASTE_STATES = int(KEY_V) | KEY_MASK_CTRL,
		KEYCODE_DUPLICATE_STATES = int(KEY_D) | KEY_MASK_CTRL,
		KEYCODE_DELETE_STATES = KEY_DELETE,
	};
	// Ref<EditorUndoRedoManager> _undo_redo;
	PopupMenu *menu = nullptr;
	Panel *mask_panel = nullptr;
	Label *not_state_alert = nullptr;
	Button *create_btn = nullptr;
	StateNode *__hovered_state_node = nullptr;
	HBoxContainer *path_button_container = nullptr;
	// ==============
	Ref<FsmRes> current_fsm_res;
	real_t activity_amount = 1.0;
	TypedArray<StateRes> copied_state_res_list = TypedArray<StateRes>();
	TypedArray<StringName> selected_state_name_list = TypedArray<StringName>();
	TypedArray<TransitionRes> selected_transition_res_list = TypedArray<TransitionRes>();
	TypedArray<TransitionRes> copied_transition_res_list = TypedArray<TransitionRes>();
	TypedArray<StringName> __bakcup_selected_state_name_list = TypedArray<StringName>();
	Color activity_color;
	bool dealing_move = false;
	bool _left_pressing = false;
	PackedVector2Array _disconnect_line = PackedVector2Array();
	bool updating = false;
	const float TRANSITION_SELECT_EXTENT = 10.0f;
	const float CONN_POS_OFFSET = 50.0f;
	const float MOVE_ZONE_HIGHT = 30.0f;
	const Vector2 SCALE_DRAGGER_SIZE = Vector2(30.0f, 30.0f);
	const int32_t HOTZONE_RADIUS = 10;
	const PackedVector2Array TRIANGLE_POINTS = PackedVector2Array(
			Array::make(Vector2(20, 0), Vector2(-15, 10), Vector2(-15, -10)));
	const Vector2 DUPLICATE_OFFSET = Vector2(50, 50);
	Ref<Font> font;

	// ========== SetGet =========
	bool is_dealing_move_states();
	void set_dealing_move_states(bool p_dealing);
	void __set_current_fsm_res(const Ref<FsmRes> &p_to_set);
	void __set_selected_state_name_list(const TypedArray<StringName> &p_to_set);
	TypedArray<StateNode> get_selected_state_nodes();
	void __set_selected_transition_res_list(const TypedArray<TransitionRes> &p_to_set);
	void __set_copied_transition_list(const TypedArray<TransitionRes> &p_to_set);
	void __set_copied_state_res_list(const TypedArray<StateRes> &p_to_set);
	// ========功能=========

	void edit_fsm_res(const Ref<FsmRes> &p_fsm_res);
	void update_cnnection();
	void __update_conntion();
	void __set_updating(bool p_to_set);
	void __undo_redo_select_nodes();
	void ___deal_selection_action();
	void __try_disconnect(Vector2 p_pos1, Vector2 p_pos2);

	bool __is_judge(Vector2 p_apos1, Vector2 p_apos2, Vector2 p_bpos1, Vector2 p_bpos2);
	void __delete_transition(const StringName &p_from, int32_t p_from_slot,
			const StringName &p_to, int32_t p_to_slot);
	TypedArray<TransitionRes> __try_select_transitions_at_pos(Vector2 pos);

	TypedArray<Vector2> __get_connection_line_with_zoom(StateNode *p_from, StateNode *to);
	Ref<TransitionRes> __get_transition_res(StateNode *p_from, StateNode *p_to);
	bool __is_node_hotzone(Object *p_in_node, int64_t p_in_port, const Vector2 &p_mouse_position);
	TypedArray<StateNode> __get_selected_state_nodes();
	void __select_state_nodes(const TypedArray<StringName> &p_to_select_State_name_list);
	StateNode *____create_state_node(const Ref<StateRes> &p_state_res);

	StateNode *___get_top_state_node_which_hovered();
	TypedArray<StateRes> ___get_selected_state_res_list();
	void __select_mamually(const TypedArray<StateNode> &p_target_nodes);
	// ==================
	void __on_current_fsm_res_changed();
	void __check_empty_fsm_res_or_not(const Ref<FsmRes> &p_fsm_res);

	void __on_popup_menu_id_pressed(int32_t p_id);
	void __on_delete_nodes_request(const Array &p_nodes);
	void __on_connection_request(const StringName &p_from, int p_from_slot,
			const StringName &p_to, int p_to_slot);

	void __on_popup_request(Vector2 p_position);
	void __on_create_btn_pressed();
	void __on_transition_res_updated();

	void __on_node_selected(Object *p_node);
	void __on_node_deselected(Object *p_node);

	// ======== 检查 ==========
	String __get_variable_expression_res_valid_and_text(
			const Ref<VariableExpressionRes> &p_ver, bool &r_valid) const;
	List<String> __get_transition_res_valid_and_texts(
			const Ref<TransitionRes> &p_transition_res, bool &r_valid) const;
	// ==================
	String str_localize(const String &p_en_key) const;

	friend class StateNode;
	friend class HFSMEditor;

public:
	static StateNodesEditor *create_state_nodes_edit(HBoxContainer *p_path_btn_container);

	void _ready() override;

	void _process(real_t p_delta);
	void _gui_input(const Ref<InputEvent> &p_event) override;
	bool _is_in_input_hotzone(Object *p_in_node, int64_t p_in_port, const Vector2 &p_mouse_position);
	bool _is_in_output_hotzone(Object *p_in_node, int64_t p_in_port, const Vector2 &p_mouse_position);
	PackedVector2Array _get_connection_line(const Vector2 &p_from, const Vector2 &p_to) const override;

	void _draw() override;
};

}; // namespace Hfsm
