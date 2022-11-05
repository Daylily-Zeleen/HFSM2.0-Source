#ifndef STATE_RES_H
#define STATE_RES_H

#include <godot_cpp/classes/Resource.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/script.hpp>

#include <godot_cpp/core/binder_common.hpp>

#include "state.hpp"

using namespace godot;

namespace Hfsm {

class HFSM;
class Fsm;
class State;
class FsmRes;

class StateRes : public Resource {
    GDCLASS(StateRes, Resource)

protected:
    static void _bind_methods();
    String _to_string() const {
        return String("[StateRes:{0}]").replace("{0}", itos(get_instance_id()));
    }

public:
    bool _set(const StringName &p_name, const Variant &p_property);
    bool _get(const StringName &p_name, Variant &r_property) const;
    void _get_property_list(List<PropertyInfo> *p_list) const;

    // enum StateType {
    //     STATE_TYPE_NORMAL,
    //     STATE_TYPE_ENTRY,
    //     STATE_TYPE_EXIT,
    //     STATE_TYPE_MAX,
    // };

    StateRes();
    ~StateRes();

    void set_name(const StringName &name);
    StringName get_name() const;
    void set_type(State::StateType state_type);
    State::StateType get_type() const;
    void set_state_script(Ref<Script> script);
    Ref<Script> get_state_script() const;
    void set_nested(bool nested);
    bool is_nested() const;
    void set_fsm_res(Ref<FsmRes> fsm_res);
    Ref<FsmRes> get_fsm_res() const;
    void set_editor_offset(Vector2 offset);
    Vector2 get_editor_offet() const;
    void set_reset_properties_when_entry(bool v);
    bool get_reset_properties_when_entry() const;
    void set_reset_nested_fsm_when_entry(bool v);
    bool get_reset_nested_fsm_when_entry() const;

    Ref<RefCounted> create_state(HFSM *hfsm, const Fsm *fsm);

    Vector2 get_size_in_editor() { return _size_in_editor; }
    void set_size_in_editor(Vector2 new_size) {
        _size_in_editor = new_size;
    }

private:
    Ref<Script> _script;
    // 避免循环依赖
    Ref<FsmRes> _fsm_res;
    Vector2 _size_in_editor;
    Vector2 _editor_offset;
    String _name = "state";
    State::StateType _type = State::STATE_TYPE_NORMAL;
    bool _nested = false;
    bool _reset_properties_when_entry = true;
    bool _reset_nested_fsm_when_entry = false;
};

}; // namespace Hfsm

#endif