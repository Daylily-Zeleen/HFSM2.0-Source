#pragma once

#include <godot_cpp/classes/editor_undo_redo_manager.hpp>

#include <godot_cpp/classes/graph_node.hpp>

#include "godot_cpp/classes/line_edit.hpp"
// #include <godot_cpp/classes/button.hpp>
#include <godot_cpp/classes/check_box.hpp>
#include <godot_cpp/classes/editor_script_picker.hpp>
#include <godot_cpp/classes/option_button.hpp>
#include <godot_cpp/classes/script.hpp>
#include <godot_cpp/classes/spin_box.hpp>

using namespace godot;
// namespace godot {
// class Button;
// };

namespace Hfsm {
class StateRes;
class StateNode : public GraphNode {
	GDCLASS(StateNode, GraphNode)
protected:
	static void _bind_methods();

private:
	Ref<class StateRes> state_res;
	// ==================
	LineEdit *name_line_edit = nullptr;
	OptionButton *type_option_btn = nullptr;
	CheckBox *has_sub_fsm_check_box = nullptr;

	Button *sub_fsm_btn = nullptr;
	EditorScriptPicker *script_picker = nullptr;

	// 新特性
	OptionButton *anim_btn = nullptr;
#ifdef FULL_VERSION
	SpinBox *anim_blend_time_spin_box = nullptr;
	SpinBox *anim_speed_spin_box = nullptr;
	CheckBox *anim_reverse_check_box = nullptr;
	Button *anim_params_visible_btn = nullptr;
#endif
	enum {
		IN_TYPE,
		OUT_TYPE
	};
	// ==================
	Ref<class StateRes> get_state_res() const { return state_res; }
	static StateNode *create_state_node(const Ref<StateRes> &p_target_state_res);
	bool __has_duplicate_name(const String &p_to_test_name);
	Array __get_brother_state_res_list();
	void __reset_state_res();
	void __setup_state_res(const Ref<StateRes> &p_to_set);
	void __set_pos_from_res();
	// ==================
	void __on_resize();
	void __cancel_name_changed();
	void __accept_name_changed(const String &p_new_name);
	void __type_option_btn_item_selected(int32_t p_idx);
	void __set_has_sub_fsm_check_box(bool p_pressed);
	void __request_edit_sub_fsm_res();
	void __script_selected(const Ref<Script> &p_script, bool p_edit);
	void __script_changed(const Ref<Script> &p_script);
	void __resize_requested(Vector2 p_new_minsize);
	void __resize();
	void __dragged(Vector2 p_from, Vector2 p_to);
	void __setup_structure();
	// ==================
	String str_localize(const String &p_en_key) const;

	friend class HFSMEditor;
	friend class StateNodesEditor;

public:
	StateNode();
	void _notification(int p_what);
	void _ready() override;
	const Color IN_COLOR = godot::Color::named("orange");
	const Color OUT_COLOR = godot::Color::named("green");
};

} // namespace Hfsm
