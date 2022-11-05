#include "state_res.hpp"
#include "fsm.hpp"
#include "fsm_res.hpp"
#include "hfsm.hpp"
namespace Hfsm {

#pragma region StateRes
StateRes::StateRes() {}
StateRes::~StateRes() {}

bool StateRes::_set(const StringName &p_name, const Variant &p_property) {
    if (p_name == StringName("state_node") && Object::cast_to<Node>(p_property)) {
        set_meta({"state_node"}, Object::cast_to<Node>(p_property));
        return true;
    }
    return false;
}
bool StateRes::_get(const StringName &p_name, Variant &r_property) const {
    if (p_name == StringName("state_node")) {
        if (has_meta({"state_node"})) {
            auto meta = get_meta({"state_node"});
            if (UtilityFunctions::is_instance_valid(meta)) {
                r_property = meta;
                return true;
            }
        }
        r_property = Variant();
        return true;
    }
    return false;
}
void StateRes::_get_property_list(List<PropertyInfo> *p_list) const { p_list->push_back(PropertyInfo(Variant::OBJECT, "state_node", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_EDITOR)); }

void StateRes::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_name"), &StateRes::get_name);
    ClassDB::bind_method(D_METHOD("set_name", "name"), &StateRes::set_name);
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "name"), "set_name", "get_name");

    ClassDB::bind_method(D_METHOD("get_type"), &StateRes::get_type);
    ClassDB::bind_method(D_METHOD("set_type", "name"), &StateRes::set_type);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "type"), "set_type", "get_type");

    ClassDB::bind_method(D_METHOD("get_state_script"), &StateRes::get_state_script);
    ClassDB::bind_method(D_METHOD("set_state_script", "script"), &StateRes::set_state_script);
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "state_script", PROPERTY_HINT_RESOURCE_TYPE, "Script"), "set_state_script", "get_state_script");

    ClassDB::bind_method(D_METHOD("is_nested"), &StateRes::is_nested);
    ClassDB::bind_method(D_METHOD("set_nested", "nested"), &StateRes::set_nested);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "nested"), "set_nested", "is_nested");

    ClassDB::bind_method(D_METHOD("get_fsm_res"), &StateRes::get_fsm_res);
    ClassDB::bind_method(D_METHOD("set_fsm_res", "fsm_res"), &StateRes::set_fsm_res);
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "fsm_res", PROPERTY_HINT_RESOURCE_TYPE, "FsmRes"), "set_fsm_res", "get_fsm_res");

    ClassDB::bind_method(D_METHOD("get_editor_offet"), &StateRes::get_editor_offet);
    ClassDB::bind_method(D_METHOD("set_editor_offset", "offet"), &StateRes::set_editor_offset);
    ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "editor_offet"), "set_editor_offset", "get_editor_offet");

    ClassDB::bind_method(D_METHOD("get_reset_properties_when_entry"), &StateRes::get_reset_properties_when_entry);
    ClassDB::bind_method(D_METHOD("set_reset_properties_when_entry", "reset"), &StateRes::set_reset_properties_when_entry);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "reset_properties_when_entry"), "set_reset_properties_when_entry", "get_reset_properties_when_entry");

    ClassDB::bind_method(D_METHOD("get_reset_nested_fsm_when_entry"), &StateRes::get_reset_nested_fsm_when_entry);
    ClassDB::bind_method(D_METHOD("set_reset_nested_fsm_when_entry", "reset"), &StateRes::set_reset_nested_fsm_when_entry);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "reset_nested_fsm_when_entry"), "set_reset_nested_fsm_when_entry", "get_reset_nested_fsm_when_entry");

    ClassDB::bind_method(D_METHOD("get_size_in_editor"), &StateRes::get_size_in_editor);
    ClassDB::bind_method(D_METHOD("set_size_in_editor", "new_size"), &StateRes::set_size_in_editor);
    ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "size_in_editor"), "set_size_in_editor", "get_size_in_editor");

    // BIND_CONSTANT(STATE_TYPE_NORMAL);
    // BIND_CONSTANT(STATE_TYPE_ENTRY);
    // BIND_CONSTANT(STATE_TYPE_EXIT);
    // BIND_CONSTANT(STATE_TYPE_MAX);
}

void StateRes::set_name(const StringName &name) {
    _name = name;
    emit_signal("changed");

    // notify_property_list_changed();
}
StringName StateRes::get_name() const { return _name; }

void StateRes::set_type(State::StateType state_type) {
    _type = state_type;
    emit_signal("changed");
    // notify_property_list_changed();
}
State::StateType StateRes::get_type() const { return _type; }
void StateRes::set_state_script(Ref<Script> script) {
    _script = script;
    emit_signal("changed");
    // notify_property_list_changed();
}
Ref<Script> StateRes::get_state_script() const { return _script; }

void StateRes::set_nested(bool nested) {
    _nested = nested;
    emit_signal("changed");
    // notify_property_list_changed();
}
bool StateRes::is_nested() const { return _nested; }

void StateRes::set_fsm_res(Ref<FsmRes> fsm_res) {
    _fsm_res = fsm_res;
    emit_signal("changed");
    // notify_property_list_changed();
}
Ref<FsmRes> StateRes::get_fsm_res() const { return _fsm_res; }

void StateRes::set_editor_offset(Vector2 offset) {
    _editor_offset = offset;
    emit_signal("changed");
    // notify_property_list_changed();
}
Vector2 StateRes::get_editor_offet() const { return _editor_offset; }
void StateRes::set_reset_properties_when_entry(bool v) {
    _reset_properties_when_entry = v;
    emit_signal("changed");
    // notify_property_list_changed();
}
bool StateRes::get_reset_properties_when_entry() const { return _reset_nested_fsm_when_entry; }
void StateRes::set_reset_nested_fsm_when_entry(bool v) {
    _reset_nested_fsm_when_entry = v;
    emit_signal("changed");
    // notify_property_list_changed();
}
bool StateRes::get_reset_nested_fsm_when_entry() const { return _reset_nested_fsm_when_entry; }

//
Ref<RefCounted> StateRes::create_state(HFSM *hfsm, const Fsm *fsm) {
    static const auto state_class_name = StringName("State");
    Ref<State> r;
    r.instantiate();
    r->set_name(_name);
    r->_hfsm = hfsm;
    r->_type = _type;
    r->_reset_when_entry = _reset_properties_when_entry;
    // 路径
    // State 一定包含于 Fsm
    r->_path.append_array(fsm->get_path());

    // 脚本处理
    auto script = _script;
    // 校验脚本是否合法
    if (script.is_valid()) {
        if (script->get_instance_base_type() == state_class_name) {
            r->set_script(script);
            if (script.is_valid()) {
                // 拾取所有属性
                auto properties = script->get_script_property_list();
                auto base_script = script->get_base_script();
                while (base_script != nullptr) {
                    properties.append_array(base_script->get_script_property_list());
                    base_script = base_script->get_base_script();
                }
                // 记录默认值
                for (uint32_t i = 0; i < properties.size(); i++) {
                    Dictionary p = static_cast<Dictionary>(properties[i]);
                    String p_name = p[Variant("name")];
                    if (p_name != "nested_state" && !hfsm->get_agents().keys().has(p_name)) {
                        // 非代理节点也非内嵌状态对象时，确定为用户自定义属性
                        r->_property_to_defatul_value.insert(p_name, r->get(p_name));
                    }
                }
            }
        } else {
            String path_text;
            for (size_t i = 0; i < r->get_path().size(); i++) {
                path_text = path_text + Ref<State>(r->get_path()[i])->get_name();
                if (i != i < r->get_path().size() - 1)
                    path_text = path_text + String("/");
            }
            ERR_FAIL_V_MSG(r, path_text + String(": Script is not extends from 'State'."));
        }
    }
    // 内嵌状态机
    // UtilityFunctions::print(get_name(), " ",_nested);
    if (_fsm_res.is_valid()) {
        r->_fsm = static_cast<FsmRes *>(get_fsm_res().ptr())->create_fsm(hfsm, r, fsm->_fsm_update_queue);
    }
    return r;
}

#pragma endregion

}; // namespace Hfsm