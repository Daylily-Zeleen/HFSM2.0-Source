#pragma once

#include <godot_cpp/classes/button.hpp>
#include <godot_cpp/classes/editor_undo_redo_manager.hpp>
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
	void set_dealing_move_states(bool dealing);
	void __set_current_fsm_res(const Ref<FsmRes> &to_set);
	void __set_selected_state_name_list(const TypedArray<StringName> &to_set);
	TypedArray<StateNode> get_selected_state_nodes();
	void __set_selected_transition_res_list(const TypedArray<TransitionRes> &to_set);
	void __set_copied_transition_list(const TypedArray<TransitionRes> &to_set);
	void __set_copied_state_res_list(const TypedArray<StateRes> &to_set);
	// ========功能=========

	void edit_fsm_res(const Ref<FsmRes> &fsm_res);
	void update_cnnection();
	void __update_conntion();
	void __set_updating(bool to_set);
	void __undo_redo_select_nodes();
	void ___deal_selection_action();
	void __try_disconnect(Vector2 pos1, Vector2 pos2);

	bool __is_judge(Vector2 apos1, Vector2 apos2, Vector2 bpos1, Vector2 bpos2);
	void __delete_transition(const StringName &from, int32_t from_slot,
			const StringName &to, int32_t to_slot);
	TypedArray<TransitionRes> __try_select_transitions_at_pos(Vector2 pos);

	TypedArray<Vector2> __get_connection_line_with_zoom(StateNode *from,
			StateNode *to);
	Ref<TransitionRes> __get_transition_res(StateNode *from, StateNode *to);
	bool __is_node_hotzone(Object *in_node, int64_t in_port,
			const Vector2 &mouse_position);
	TypedArray<StateNode> __get_selected_state_nodes();
	void __select_state_nodes(const TypedArray<StringName> &to_select_State_name_list);
	StateNode *____create_state_node(const Ref<StateRes> &state_res);

	StateNode *___get_top_state_node_which_hovered();
	TypedArray<StateRes> ___get_selected_state_res_list();
	void ___select_mamually(const TypedArray<StateNode> &target_nodes);
	// ==================
	void __on_current_fsm_res_changed();
	void __check_empty_fsm_res_or_not(const Ref<FsmRes> &fsm_res);
	// ========HACK==========
	// void __on_copy_requested() { __on_popup_menu_id_pressed(ITEM_ADD_STATE); }
	// void __on_paste_requested() { __on_popup_menu_id_pressed(ITEM_PASTE_STATES); }
	// void __on_duplicate_requested() { __on_popup_menu_id_pressed(ITEM_DUPLICATE_STATES); }
	// void __on_edit_fsm_res_requeted();
	// ========HACK==========
	void __on_popup_menu_id_pressed(int32_t id);
	void __on_delete_nodes_request(const Array &nodes);
	void __on_connection_request(const StringName &from, int from_slot,
			const StringName &to, int to_slot);

	void __on_popup_request(Vector2 position);
	void __on_create_btn_pressed();
	void __on_transition_res_updated();

	void __on_node_selected(Object *node);
	void __on_node_deselected(Object *node);

	// ======== 检查 ==========
	String __get_variable_expression_res_valid_and_text(
			const Ref<VariableExpressionRes> &ver, bool &r_valid) const;
	List<String> __get_transition_res_valid_and_texts(
			const Ref<TransitionRes> &transition_res, bool &r_valid) const;
	// ==================
	String str_localize(const String &en_key) const;

	friend class StateNode;
	friend class HFSMEditor;

public:
	StateNodesEditor();
	static StateNodesEditor *create_state_nodes_edit(HBoxContainer *path_btn_container);

	void _ready() override;

	void _process(real_t delta);
	void _gui_input(const Ref<InputEvent> &event) override;
	bool _is_in_input_hotzone(Object *in_node, int64_t in_port, const Vector2 &mouse_position);
	bool _is_in_output_hotzone(Object *in_node, int64_t in_port, const Vector2 &mouse_position);
	PackedVector2Array _get_connection_line(const Vector2 &from, const Vector2 &to) const override;

	void _draw() override;
};

}; // namespace Hfsm
