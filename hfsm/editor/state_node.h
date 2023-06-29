#pragma once

#ifdef GDEXTENSION_BUILD
#include <godot_cpp/classes/check_box.hpp>
#include <godot_cpp/classes/graph_node.hpp>
#include <godot_cpp/classes/line_edit.hpp>
#include <godot_cpp/classes/option_button.hpp>
#include <godot_cpp/classes/script.hpp>

#include <godot_cpp/classes/editor_script_picker.hpp>
using namespace godot;
#else
#include <scene/gui/graph_node.h>

#include <editor/editor_resource_picker.h>
#include <scene/gui/check_box.h>
#include <scene/gui/line_edit.h>
#include <scene/gui/option_button.h>

#endif // GDEXTENSION_BUILD

namespace Hfsm {

class StateNode : public GraphNode {
	GDCLASS(StateNode, GraphNode)
protected:
	static void _bind_methods();

	void _notification(int p_what);

private:
	Ref<class FsmRes> nested_fsm_res;
	Ref<class StateRes> state_res;
	// ==================
	LineEdit *name_line_edit = nullptr;
	OptionButton *type_option_btn = nullptr;
	CheckBox *has_sub_fsm_check_box = nullptr;

	Button *sub_fsm_btn = nullptr;
	EditorScriptPicker *script_picker = nullptr;

	void set_state_res(const Ref<class StateRes> &p_state_res);

	// 动画交给监视器
	// ==================
	bool __has_duplicate_name(const String &p_to_test_name);
	// ==================
	void _setup_state_res();
	void __on_resize();
	void _cancel_name_changed();
	void _accept_name_changed(const String &p_new_name);
	void _type_option_btn_item_selected(int32_t p_idx);
	void _set_has_sub_fsm_check_box(bool p_pressed);
	void _request_edit_sub_fsm_res();
	void _script_selected(const Ref<Script> &p_script, bool p_edit);
	void _script_changed(const Ref<Script> &p_script);
	void _resize_requested(Vector2 p_new_minsize);
	void _resize();
	void initialize();
	// ==================
	String str_localize(const String &p_en_key) const;

	const bool debug_mode = false;
	bool debug_actived = false;

public:
	static Color IN_COLOR;
	static Color OUT_COLOR;

	enum {
		IN_TYPE,
		OUT_TYPE
	};

	Ref<class StateRes> get_state_res() const;
	static StateNode *create_state_node(Ref<StateRes> p_target_state_res, const Ref<class FsmRes> &p_nested_fsm_res, bool p_debug = false);

	void set_debug_actived(bool p_actived);
	bool is_debug_actived() const;

	StateNode(bool p_debug_mode = false);
};

} // namespace Hfsm
