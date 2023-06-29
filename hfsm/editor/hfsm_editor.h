#pragma once

#ifdef GDEXTENSION_BUILD
#include <godot_cpp/classes/h_box_container.hpp>
#include <godot_cpp/classes/label.hpp>
#include <godot_cpp/classes/panel.hpp>

using namespace godot;

#else
#include <scene/gui/box_container.h>
#include <scene/gui/label.h>
#endif // GDEXTENSION_BUILD

namespace Hfsm {
class FsmEditor;
class StateRes;
class FsmRes;
class HFSM;

class HFSMEditor : public Control {
	GDCLASS(HFSMEditor, Control)
protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	HFSMEditor(bool p_debug_mode = false);

	void edit_hfsm(HFSM *p_hfsm);
	HFSM *get_editing_hfsm();
	void edit_fsm_res_in_hfsm(Ref<FsmRes> p_fsm_res, const Ref<FsmRes> &p_root_fsm_res = nullptr);

	void debug_highlight_activate_state(const PackedStringArray &p_active_path);

	static HFSMEditor *create_hfsm_editor(bool p_debug_mode = false);

private:
	HBoxContainer *path_button_container = nullptr;
	FsmEditor *fsm_editor = nullptr;
	//  TODO::实际显示内容
	Label *tip_label = nullptr;
	// Label *history_label = nullptr;
	//
	Panel *mask_panel = nullptr;
	Label *not_hfsm_label = nullptr;

	HFSM *hfsm = nullptr;

	// 调试模式下hfsm为nullptr
	const bool debug_mode = false;

	bool try_set_nested_state_res_for_fsm_res_recursively(Ref<FsmRes> &p_fsm_res, Ref<FsmRes> p_to_search_fsm_res = nullptr);
	void initialize();

	void _inspector_property_edited(const String &p_properrty);
	void _inspector_edited_object_changed();
	void _edit_fsm_requested(Ref<FsmRes> &p_fsm_res);

	void set_connect_inspector_signal(bool p_connect);

	Ref<FsmRes> editing_root_fsm_res;
};

}; // namespace Hfsm
