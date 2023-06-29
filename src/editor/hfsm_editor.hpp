#pragma once

#include <godot_cpp/classes/h_box_container.hpp>
#include <godot_cpp/classes/label.hpp>
#include <godot_cpp/classes/panel.hpp>

#include <godot_cpp/classes/editor_inspector_plugin.hpp>
#include <godot_cpp/classes/editor_plugin.hpp>

#include <godot_cpp/classes/translation_server.hpp>

#include <core/fsm_res.hpp>

using namespace godot;
namespace Hfsm {
class StateNodesEditor;
class StateRes;
// class FsmRes;
class HFSM;

class HFSMEditor : public Control {
	GDCLASS(HFSMEditor, Control)
protected:
	static void _bind_methods() {}

public:
	void _ready() override;
	void edit(HFSM *p_hfsm);
	HFSM *get_editing_hfsm();
	void request_edit_fsm_res(const Ref<FsmRes> &p_fsm_res);
	Ref<FsmRes> get_nested_fsm_res(const Ref<StateRes> &p_state_res, Ref<FsmRes> p_fsm_res = nullptr);

	static HFSMEditor *create_hfsm_editor();

private:
	HBoxContainer *path_button_container = nullptr;
	StateNodesEditor *state_nodes_editor = nullptr;
	//  TODO::实际显示内容
	Label *tip_label = nullptr;
	Label *history_label = nullptr;
	//
	Panel *mask_panel = nullptr;
	Label *not_hfsm_label = nullptr;

	HFSM *hfsm = nullptr;
	bool find_nested_state_res(const Ref<FsmRes> &p_fsm_res,
			Ref<FsmRes> p_to_search_fsm_res = nullptr);
};

}; // namespace Hfsm
