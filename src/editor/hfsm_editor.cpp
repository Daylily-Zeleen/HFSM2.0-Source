#include "hfsm_editor.hpp"

#include <godot_cpp/classes/margin_container.hpp>
#include <godot_cpp/classes/panel_container.hpp>
#include <godot_cpp/classes/v_box_container.hpp>

// #include <godot_cpp/variant/utility_functions.hpp>

#include "src/core/fsm_res.hpp"
#include "src/core/state_res.hpp"
#include "state_nodes_editor.hpp"

#include "src/core/hfsm.hpp"

#include "hfsm_editor_plugin.hpp"

using namespace godot;
namespace Hfsm {
void HFSMEditor::_bind_methods() {}

HFSMEditor::HFSMEditor() {}
bool HFSMEditor::find_nested_state_res(Ref<FsmRes> fsm_res,
                                       Ref<FsmRes> to_search_fsm_res) {
    if (fsm_res.is_null())
        return false;
    if (to_search_fsm_res.is_null()) {
        to_search_fsm_res = get_editing_hfsm()->get_root_fsm_res();
    }
    if (to_search_fsm_res.is_null())
        return false;
    if (fsm_res == to_search_fsm_res)
        return true;
    auto sr_list = to_search_fsm_res->get_state_res_list();
    for (size_t i = 0; i < sr_list.size(); i++) {
        Ref<StateRes> sr = sr_list[i];
        if (sr->get_fsm_res().is_valid()) {
            if (sr->get_fsm_res() == fsm_res) {
                fsm_res->set_nested_state_res(sr);
                return true;
            } else {
                if (find_nested_state_res(fsm_res, sr->get_fsm_res())) {
                    return true;
                }
            }
        }
    }
    return false;
}
// if not is_instance_valid(fsm_res):
// 	return false
// if fsm_res == to_search_fsm_res:
// 	return true
// for sr in to_search_fsm_res.state_res_list:
// 	if is_instance_valid(sr.fsm_res):
// 		if sr.fsm_res == fsm_res:
// 			fsm_res.nested_state_res = sr
// 			return true
// 		else:
// 			if __find_nested_state_res(fsm_res, sr.fsm_res):
// 				return true
// return false

Ref<FsmRes> HFSMEditor::get_nested_fsm_res(Ref<StateRes> state_res,
                                           Ref<FsmRes> fsm_res) {
    if (fsm_res.is_null()) {
        fsm_res = get_editing_hfsm()->get_root_fsm_res();
    }
    if (fsm_res.is_valid()) {
        auto state_res_list = fsm_res->get_state_res_list();
        for (size_t i = 0; i < state_res_list.size(); i++) {
            Ref<StateRes> sr = state_res_list[i];
            if (sr == state_res)
                return fsm_res;
            else {
                if (sr->get_fsm_res().is_valid()) {
                    auto r = get_nested_fsm_res(state_res, sr->get_fsm_res());
                    if (r.is_valid())
                        return r;
                }
            }
        }
    }
    return nullptr;
}
void HFSMEditor::request_edit_fsm_res(Ref<FsmRes> fsm_res) {
    if (fsm_res->get_nested_state_res().is_null()) {
        find_nested_state_res(fsm_res);
    }
    state_nodes_editor->edit_fsm_res(fsm_res);
}

void HFSMEditor::_ready() {
    not_hfsm_label->set_text(HfsmEditorPlugin::str_localize(
        "Plese select a 'HFSM' node to start edit."));
}
void HFSMEditor::edit(HFSM *hfsm) {
    _hfsm = hfsm;
    if (_hfsm)
        mask_panel->hide();
    else
        mask_panel->show();
    if (_hfsm && _hfsm->get_root_fsm_res().is_valid()) {
        state_nodes_editor->edit_fsm_res(_hfsm->get_root_fsm_res());
    } else {
        state_nodes_editor->edit_fsm_res(nullptr);
    }
    // state_nodes_editor->edit_fsm_res((_hfsm &&
    // _hfsm->get_root_fsm_res().is_valid()) ? _hfsm->get_root_fsm_res()
    //                                        : nullptr);
}
HFSM *HFSMEditor::get_editing_hfsm() { return _hfsm; }
HFSMEditor *HFSMEditor::create_hfsm_editor() {
    auto r = memnew(HFSMEditor);
    r->set_custom_minimum_size(Vector2(0, 200));

    r->set_anchors_preset(LayoutPreset::PRESET_FULL_RECT);

    auto vbox = memnew(VBoxContainer);
    r->add_child(vbox);
    vbox->set_anchors_preset(LayoutPreset::PRESET_FULL_RECT);
    auto up_panel_container = memnew(PanelContainer);
    vbox->add_child(up_panel_container);
    auto up_margin_contianer = memnew(MarginContainer);
    // TODO:: 调整边距
    up_panel_container->add_child(up_margin_contianer);
    r->path_button_container = memnew(HBoxContainer);
    auto up_label = memnew(Label);
    r->path_button_container->add_child(up_label);

    //
    r->state_nodes_editor =
        StateNodesEditor::create_state_nodes_edit(r->path_button_container);
    vbox->add_child(r->state_nodes_editor);

    auto button_h_box = memnew(HBoxContainer);
    vbox->add_child(button_h_box);
    r->tip_label = memnew(Label);
    r->tip_label->set_h_size_flags(0 | 2);
    button_h_box->add_child(r->tip_label);
    r->history_label = memnew(Label);
    r->history_label->set_h_size_flags(8 | 2);
    button_h_box->add_child(r->history_label);

    //
    r->mask_panel = memnew(Panel);
    r->mask_panel->set_anchors_preset(LayoutPreset::PRESET_FULL_RECT);
    r->mask_panel->set_self_modulate(Color(0, 0, 0, 0.6));
    r->add_child(r->mask_panel);
    
    r->not_hfsm_label = memnew(Label);
    r->not_hfsm_label->set_text(String("请选中一个 HFSM 节点开始编辑"));
    r->not_hfsm_label->set_vertical_alignment(VERTICAL_ALIGNMENT_CENTER);
    r->not_hfsm_label->set_horizontal_alignment(HORIZONTAL_ALIGNMENT_CENTER);
    r->mask_panel->add_child(r->not_hfsm_label);

    r->mask_panel->hide();
    return r;
}
}; // namespace Hfsm
