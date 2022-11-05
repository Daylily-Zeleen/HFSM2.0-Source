#ifndef FSM_RES_H
#define FSM_RES_H

#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/templates/vector.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "hfsm_variable_res.hpp"
#include "state.hpp"
#include "state_res.hpp"
#include "transition_res.hpp"

using namespace godot;

namespace Hfsm {
class HFSM;
class Fsm;
class HFSMVariableRes;

// 状态机资源
class FsmRes : public Resource {
    GDCLASS(FsmRes, Resource)

protected:
    static void _bind_methods();

    String _to_string() const { return String("[FsmRes:{0}]").replace("{0}", itos(get_instance_id())); }

public:
    FsmRes();
    ~FsmRes();

    Fsm *create_fsm(HFSM *hfsm, const Ref<State> &nested_state, const Vector<Hfsm::Fsm *> &nested_fsm_update_queue);

    void set_nested_state_res(const Ref<StateRes> &state_res);
    Ref<StateRes> get_nested_state_res() const;

    TypedArray<StateRes> get_state_res_list() const { return _state_res_list; }
    // 转换列表
    TypedArray<TransitionRes> get_transition_res_list() const { return _transition_res_list; }
    // 变量列表
    TypedArray<HFSMVariableRes> get_variable_res_list() const { return _variable_res_list; }

    void add_state_res(Ref<StateRes> state_res);

    void add_transition_res(Ref<TransitionRes> transition_res);

    void add_variable_res(Ref<HFSMVariableRes> variable_res) {
        if (_variable_res_list.find(variable_res) >= 0)
            return;
        _variable_res_list.push_back(variable_res);
    }

    // 未删除相关的 TransitionRes, 需要在编辑器里处理 undoredo
    void remove_state_res(Ref<StateRes> state_res) {
        _state_res_list.erase(state_res);
        emit_changed();
    }

    void remove_transition_res(Ref<TransitionRes> transition_res);

    void remove_variable_res(Ref<HFSMVariableRes> variable_res) { _variable_res_list.erase(variable_res); }

    // void set_state_res_list(Array state_res_list){
    //     _state_res_list = Vector<Ref<StateRes>>(state_res_list);
    // }

    // 编辑器方法
    // void set_unique_entry_state(Ref<StateRes> &state_res);
    // Ref<StateRes> get_exist_entry_res();
    // void add_state_res(Ref<StateRes> &new_state_res);
    // void deleted_state_res(Ref<StateRes> &deleted_state_res);
    // void make_state_name_unique(Ref<StateRes> &state_res);
    // void add_transition_res(Ref<TransitionRes> &new_transition_res);
    // void delete_transition_res(Ref<TransitionRes> &deleted_transition_res);

    // TODO:: 这是啥？？？
    // Ref<FsmRes> duplicate_self();

    // VMap<Ref<Script>, Ref<StateRes>> get_state_script_to_state();
    // bool is_deleted_state_script();
    // Vector<Ref<StateRes>> get_all_nested_state_res();

private:
    // 自己的状态列表
    void _set_state_res_list(Array state_res_list) { _state_res_list = TypedArray<StateRes>(state_res_list); }
    void _set_transition_res_list(Array transition_res_list) { _transition_res_list = TypedArray<TransitionRes>(transition_res_list); }
    void _set_variable_res_list(Array variable_res_list);

    // 所在的状态
    Ref<StateRes> _nested_state_res;

    // 自己的状态列表
    TypedArray<StateRes> _state_res_list;
    // 变量列表
    TypedArray<HFSMVariableRes> _variable_res_list;
    // 转换列表
    TypedArray<TransitionRes> _transition_res_list;

    friend class HFSM;
    friend class HFSMVariableRes;
};

}; // namespace Hfsm

#endif