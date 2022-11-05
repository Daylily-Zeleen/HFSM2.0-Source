#include "state_node.hpp"

#include <godot_cpp/classes/time.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include <godot_cpp/classes/editor_interface.hpp>
#include <godot_cpp/classes/h_box_container.hpp>
#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/classes/v_box_container.hpp>

#include "hfsm_editor_plugin.hpp"
#include "src/core/fsm_res.hpp"
#include "state_nodes_editor.hpp"

using namespace godot;
namespace Hfsm {

String StateNode::str_localize(const String &en_key) const {
    return HfsmEditorPlugin::str_localize(en_key);
}

void StateNode::_bind_methods() {
    ClassDB::bind_method(D_METHOD("__cancel_name_changed"),
                         &StateNode::__cancel_name_changed);
    ClassDB::bind_method(D_METHOD("__accept_name_changed"),
                         &StateNode::__accept_name_changed);
    ClassDB::bind_method(D_METHOD("__type_option_btn_item_selected", "idx"),
                         &StateNode::__type_option_btn_item_selected);
    ClassDB::bind_method(D_METHOD("__set_has_sub_fsm_check_box", "pressed"),
                         &StateNode::__set_has_sub_fsm_check_box);
    ClassDB::bind_method(D_METHOD("__request_edit_sub_fsm_res"),
                         &StateNode::__request_edit_sub_fsm_res);
    ClassDB::bind_method(D_METHOD("__script_selected", "script", "edit"),
                         &StateNode::__script_selected);
    ClassDB::bind_method(D_METHOD("__script_changed", "script"),
                         &StateNode::__script_changed);
    ClassDB::bind_method(D_METHOD("__resize_requested", "size"),
                         &StateNode::__resize_requested);
    ClassDB::bind_method(D_METHOD("__reset_state_res"),
                         &StateNode::__reset_state_res);
    ClassDB::bind_method(D_METHOD("__resize"), &StateNode::__resize);
    ClassDB::bind_method(D_METHOD("__dragged"), &StateNode::__dragged);
}

StateNode *StateNode::create_state_node(Ref<StateRes> target_state_res) {
    if (target_state_res.is_null())
        return nullptr;
    auto r = memnew(StateNode);
    r->__setup_structure();
    r->__setup_state_res(target_state_res);
    r->set_name(String("@") + itos(Time::get_singleton()->get_ticks_msec() +
                                   r->get_instance_id()));
    target_state_res->set("state_node", r);
    return r;
}
bool StateNode::__has_duplicate_name(const String &to_test_name) {
    auto brothers = __get_brother_state_res_list();
    for (size_t i = 0; i < brothers.size(); i++) {
        auto sr = Object::cast_to<StateRes>(brothers[i]);
        if (sr && sr != state_res.ptr()) {
            if (sr->get_name() == to_test_name) {
                return true;
            }
        }
    }
    return false;
}
Array StateNode::__get_brother_state_res_list() {
    auto nested_fsm_res = HfsmEditorPlugin::get_singleton()
                              ->get_hfsm_editor()
                              ->get_nested_fsm_res(state_res);
    if (nested_fsm_res.is_null()) {
        return Array();
    } else
        return nested_fsm_res->get_state_res_list();
}

void StateNode::__reset_state_res() { __setup_state_res(state_res); }

void StateNode::__setup_state_res(Ref<StateRes> to_set) {
    if (state_res.is_valid()) {
        if (state_res->is_connected("changed",
                                    Callable(this, "__reset_state_res"))) {
            state_res->disconnect("changed",
                                  Callable(this, "__reset_state_res"));
        }
    }
    state_res = to_set;
    if (state_res.is_valid()) {
        if (!state_res->is_connected("changed",
                                     Callable(this, "__reset_state_res"))) {
            state_res->connect("changed", Callable(this, "__reset_state_res"));
        }
    } else {
        name_line_edit->set_text("<error>");
        set_title("<error>");
        return;
    }
    //
    name_line_edit->set_text(state_res->get_name());
    set_title(name_line_edit->get_text());
    // 类型
    type_option_btn->clear();
    switch (state_res->get_type()) {
    case State::STATE_TYPE_ENTRY: {
        type_option_btn->add_item("Entry", State::STATE_TYPE_ENTRY);
        break;
    }
    case State::STATE_TYPE_NORMAL:
    case State::STATE_TYPE_EXIT: {
        type_option_btn->add_item("Entry", State::STATE_TYPE_ENTRY);
        type_option_btn->add_item("Normal", State::STATE_TYPE_NORMAL);
        type_option_btn->add_item("Exit", State::STATE_TYPE_EXIT);
        break;
    }
    default:
        break;
    }
    type_option_btn->select(
        type_option_btn->get_item_index(state_res->get_type()));
    notify_property_list_changed();
    // 子状态机
    has_sub_fsm_check_box->set_pressed(state_res->get_fsm_res().is_valid());
    sub_fsm_btn->set_disabled(!has_sub_fsm_check_box->is_pressed());
    // 脚本
    script_picker->set_edited_resource(state_res->get_state_script());
    // 位置s
    if (!is_inside_tree())
        return;
    __set_pos_from_res();
}

void StateNode::__set_pos_from_res() {
    auto parent = Object::cast_to<StateNodesEditor>(get_parent());
    if (parent) {
        auto zoom = parent->get_zoom();
        auto scroll_offsetr = parent->get_scroll_ofs();
        set_position_offset(state_res->get_editor_offet());
    }
}
// ==================
void StateNode::__on_resize() {
    auto size = get_size();
    size.y = 0;
    set_size(size);
}
void StateNode::__cancel_name_changed() {
    name_line_edit->set_text(state_res->get_name());
    __on_resize();
}
void StateNode::__accept_name_changed(const String &new_name) {
    if (name_line_edit->get_name() == state_res->get_name())
        return;
    if (__has_duplicate_name(name_line_edit->get_text())) {
        UtilityFunctions::printerr(
            str_localize("HFSM: has duplicated State name: "),
            name_line_edit->get_text());
        name_line_edit->set_text(state_res->get_name());
        return;
    }
    // undoredo
    auto unro_redo = HfsmEditorPlugin::create_action("Change state name");
    unro_redo->add_do_property(state_res.ptr(), "name",
                               name_line_edit->get_text());
    unro_redo->add_do_property(this, "title", name_line_edit->get_text());
    unro_redo->add_undo_property(state_res.ptr(), "name",
                                 state_res->get_name());
    unro_redo->add_undo_property(this, "title", state_res->get_name());
    unro_redo->commit_action();
}
void StateNode::__type_option_btn_item_selected(int32_t idx) {
    auto id = type_option_btn->get_item_id(idx);
    if (id < 0 || id > 3)
        return;
    auto target_type = (State::StateType)id;
    if (state_res->get_type() == target_type)
        return;
    switch (target_type) {
    case State::STATE_TYPE_NORMAL: {
        if (state_res->get_type() == State::STATE_TYPE_ENTRY) {
            UtilityFunctions::printerr(
                "HFSM::", state_res->get_type(),
                str_localize(
                    ": this state is Entry State, can't set to other type."));
            return;
        }
        auto undo_redo = HfsmEditorPlugin::create_action("Change state type");
        undo_redo->add_do_method(state_res.ptr(), StringName("set_type"),
                                 State::STATE_TYPE_NORMAL);
        undo_redo->add_undo_method(state_res.ptr(), StringName("set_type"),
                                   state_res->get_type());
        undo_redo->commit_action();
    } break;
    case State::STATE_TYPE_ENTRY: {
        auto undo_redo = HfsmEditorPlugin::create_action("Change state type");
        auto brother_state_res_list = __get_brother_state_res_list();
        for (size_t i = 0; i < brother_state_res_list.size(); i++) {
            Ref<StateRes> sr = brother_state_res_list[i];
            if (sr.is_valid() && sr != state_res.ptr() &&
                sr->get_type() == State::STATE_TYPE_ENTRY) {
                undo_redo->add_do_method(sr.ptr(), StringName("set_type"),
                                         State::STATE_TYPE_NORMAL);
                undo_redo->add_undo_method(sr.ptr(), StringName("set_type"),
                                           state_res->get_type());
            }
        }
        undo_redo->add_do_method(state_res.ptr(), StringName("set_type"),
                                 State::STATE_TYPE_ENTRY);
        undo_redo->add_undo_method(state_res.ptr(), StringName("set_type"),
                                   state_res->get_type());
        undo_redo->commit_action();
    } break;
    case State::STATE_TYPE_EXIT: {
        if (state_res->get_type() == State::STATE_TYPE_ENTRY) {
            UtilityFunctions::printerr(
                "HFSM::", state_res->get_type(),
                str_localize(
                    ": this state is Entry State, can't set to other type."));
            auto undo_redo =
                HfsmEditorPlugin::create_action("Change state type");
            undo_redo->add_do_method(state_res.ptr(), StringName("set_type"),
                                     State::STATE_TYPE_EXIT);
            undo_redo->add_undo_method(state_res.ptr(), StringName("set_type"),
                                       state_res->get_type());
            undo_redo->commit_action();
        }
    } break;
    default:
        break;
    }
}

void StateNode::__set_has_sub_fsm_check_box(bool pressed) {
    if (state_res->get_fsm_res().is_null() && !pressed)
        return;
    if (state_res->get_fsm_res().is_valid() && pressed)
        return;
    auto undo_redo = HfsmEditorPlugin::create_action("set Sub-FSM");
    Ref<FsmRes> new_sub_fsm;
    new_sub_fsm.instantiate();
    new_sub_fsm->set_nested_state_res(state_res);
    undo_redo->add_do_method(state_res.ptr(), StringName("set_fsm_res"),
                             pressed ? new_sub_fsm : nullptr);
    undo_redo->add_undo_method(state_res.ptr(), StringName("set_fsm_res"),
                               state_res->get_fsm_res());
    undo_redo->commit_action();
}
void StateNode::__request_edit_sub_fsm_res() {
    if (state_res->get_fsm_res().is_valid()) {
        HfsmEditorPlugin::get_singleton()
            ->get_hfsm_editor()
            ->request_edit_fsm_res(state_res->get_fsm_res());
    }
}
void StateNode::__script_selected(Ref<Script> script, bool edit) {
    HfsmEditorPlugin::get_singleton()->get_editor_interface()->edit_resource(
        script);
}
void StateNode::__script_changed(Ref<Script> script) {
    if (Object::cast_to<Script>(get_script())) {
        if (state_res->get_state_script().is_null()) {
            // 新建
            if (Engine::get_singleton()->is_editor_hint() &&
                script->get_source_code().is_empty()) {
                // TODO::添加模板
                script->set_source_code("");
            }
        }
        if (script == state_res->get_state_script())
            return;
        auto undo_redo = HfsmEditorPlugin::create_action("Attach state script");
        undo_redo->add_do_method(state_res.ptr(), "set_state_script", script);
        undo_redo->add_undo_method(state_res.ptr(), "set_state_script",
                                   state_res->get_state_script());
        undo_redo->commit_action();
    }
}
void StateNode::__resize_requested(Vector2 new_minsize) {
    auto size = get_size();
    size.x = new_minsize.x;
    call_deferred("__resize");
}
void StateNode::__resize() {
    if (Input::get_singleton()->is_mouse_button_pressed(MOUSE_BUTTON_LEFT))
        return;
    if (get_size().is_equal_approx(state_res->get_size_in_editor()))
        return;
    auto undo_redo = HfsmEditorPlugin::create_action("resized");
    undo_redo->add_do_property(this, "size", get_size());
    undo_redo->add_undo_property(this, "size", state_res->get_size_in_editor());
    undo_redo->add_do_method(state_res.ptr(), StringName("set_size_in_editor"),
                             get_size());
    undo_redo->add_undo_method(state_res.ptr(),
                               StringName("set_size_in_editor"),
                               state_res->get_size_in_editor());
    undo_redo->commit_action();
}
void StateNode::__dragged(Vector2 from, Vector2 to) {
    auto parent = Object::cast_to<StateNodesEditor>(get_parent());
    if (!parent)
        return;
    if (parent->is_dealing_move_states())
        return;
    parent->set_dealing_move_states(true);
    auto undo_redo = HfsmEditorPlugin::create_action("move states");
    auto nodes = get_parent()->get_children();
    for (size_t i = 0; i < nodes.size(); i++) {
        auto node = Object::cast_to<StateNode>(nodes[i]);
        if (node) {
            if (!node->state_res->get_editor_offet().is_equal_approx(
                    node->get_position_offset())) {
                undo_redo->add_do_method(node->state_res.ptr(),
                                         StringName("set_editor_offset"),
                                         node->get_position_offset());
                undo_redo->add_undo_method(node->state_res.ptr(),
                                           StringName("set_editor_offset"),
                                           node->state_res->get_editor_offet());
            }
        }
    }
    undo_redo->commit_action();
    parent->set_dealing_move_states(false);
}
void StateNode::_ready() {
    if (state_res.is_null())
        return;
    auto size = get_size();
    size.y = 0;
    set_size(size);
    state_res->set_size_in_editor(get_size());
    __set_pos_from_res();
}

void StateNode::__setup_structure() {
    // 节点结构====
    {
        auto v_box = memnew(VBoxContainer);
        add_child(v_box);
        // 名称输入行
        name_line_edit = memnew(LineEdit);
        v_box->add_child(name_line_edit);
        name_line_edit->set_auto_translate(false);
        name_line_edit->set_placeholder("state name");
        name_line_edit->set_expand_to_text_length_enabled(true);
        name_line_edit->set_h_size_flags(SIZE_EXPAND_FILL);
        // 类型选择框
        type_option_btn = memnew(OptionButton);
        v_box->add_child(type_option_btn);
        // 子状态机
        auto h_box = memnew(HBoxContainer);
        v_box->add_child(h_box);
        has_sub_fsm_check_box = memnew(CheckBox);
        h_box->add_child(has_sub_fsm_check_box);
        has_sub_fsm_check_box->set_h_size_flags(SIZE_EXPAND);
        sub_fsm_btn = memnew(Button);
        sub_fsm_btn->set_text(str_localize("Sub FSM"));
        h_box->add_child(sub_fsm_btn);
        // 脚本拾取器
        script_picker = memnew(EditorScriptPicker);
        script_picker->set_base_type("Script");
        v_box->add_child(script_picker);
    }
    set_slot(0, true, IN_TYPE, IN_COLOR, true, OUT_TYPE, OUT_COLOR);
    set_resizable(true);

    // 信号功能连接
    { // 名称输入行
        name_line_edit->connect("focus_exited",
                                Callable(this, "__cancel_name_changed"));
        name_line_edit->connect("text_change_rejected",
                                Callable(this, "__cancel_name_changed"));
        name_line_edit->connect("text_submitted",
                                Callable(this, "__accept_name_changed"));
        // 类型选择框
        type_option_btn->connect(
            "item_selected", Callable(this, "__type_option_btn_item_selected"));
        // 子状态机
        has_sub_fsm_check_box->connect(
            "toggled", Callable(this, "__set_has_sub_fsm_check_box"));
        sub_fsm_btn->connect("pressed",
                             Callable(this, "__request_edit_fsm_res"));
        // 脚本拾取器
        script_picker->connect("resource_selected",
                               Callable(this, "__script_selected"));
        script_picker->connect("resource_changed",
                               Callable(this, "__script_changed"));
        // 自身
        connect("resized", Callable(this, "__resize"));
        connect("resize_request", Callable(this, "__resize_requested"));
        connect("dragged", Callable(this, "__dragged"));
    }
}

void StateNode::_notification(int p_what) {
    if (p_what == NOTIFICATION_PARENTED || p_what == NOTIFICATION_UNPARENTED) {
        auto state_nodes_dite = Object::cast_to<StateNodesEditor>(get_parent());
        if (state_nodes_dite) {
            state_nodes_dite->update_cnnection();
        }
    }
}

StateNode::StateNode() {}

} // namespace Hfsm
