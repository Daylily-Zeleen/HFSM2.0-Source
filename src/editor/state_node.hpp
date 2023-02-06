#pragma once

#include <godot_cpp/classes/editor_undo_redo_manager.hpp>

#include <godot_cpp/classes/button.hpp>
#include <godot_cpp/classes/check_box.hpp>
#include <godot_cpp/classes/editor_script_picker.hpp>
#include <godot_cpp/classes/graph_node.hpp>
#include <godot_cpp/classes/option_button.hpp>
#include <godot_cpp/classes/spin_box.hpp>

#include "src/core/state_res.hpp"

using namespace godot;
namespace Hfsm {

class StateNode : public GraphNode {
	GDCLASS(StateNode, GraphNode)
protected:
	static void _bind_methods();

private:
	Ref<StateRes> state_res;
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
	static StateNode *create_state_node(Ref<StateRes> target_state_res);
	bool __has_duplicate_name(const String &to_test_name);
	Array __get_brother_state_res_list();
	void __reset_state_res();
	void __setup_state_res(Ref<StateRes> to_set);
	void __set_pos_from_res();
	// ==================
	void __on_resize();
	void __cancel_name_changed();
	void __accept_name_changed(const String &new_name);
	void __type_option_btn_item_selected(int32_t idx);
	void __set_has_sub_fsm_check_box(bool pressed);
	void __request_edit_sub_fsm_res();
	void __script_selected(Ref<Script> script, bool edit);
	void __script_changed(Ref<Script> script);
	void __resize_requested(Vector2 new_minsize);
	void __resize();
	void __dragged(Vector2 from, Vector2 to);
	void __setup_structure();
	// ==================
	String str_localize(const String &en_key) const;
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
