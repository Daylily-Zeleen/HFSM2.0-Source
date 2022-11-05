#include "fsm_res.hpp"

#include "fsm.hpp"
#include "state_res.hpp"
#include "transition_res.hpp"
#include "transitions/transition_base.hpp"

namespace Hfsm {

#pragma region FsmRes

void FsmRes::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_nested_state_res"), &FsmRes::get_nested_state_res);
    ClassDB::bind_method(D_METHOD("set_nested_state_res", "state_res"), &FsmRes::set_nested_state_res);
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "nested_state_res", PROPERTY_HINT_RESOURCE_TYPE, "StateRes", PROPERTY_USAGE_NONE), "set_nested_state_res", "get_nested_state_res");

    ClassDB::bind_method(D_METHOD("get_state_res_list"), &FsmRes::get_state_res_list);
    ClassDB::bind_method(D_METHOD("_set_state_res_list", "place_holder"), &FsmRes::_set_state_res_list);

    auto typed_StateRes_array_hint_string = String("{0}/{1}:StateRes").format(Array::make(Variant::OBJECT, PROPERTY_HINT_RESOURCE_TYPE));
    ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "state_res_list", PROPERTY_HINT_TYPE_STRING, typed_StateRes_array_hint_string), "_set_state_res_list", "get_state_res_list");

    ClassDB::bind_method(D_METHOD("get_transition_res_list"), &FsmRes::get_transition_res_list);
    ClassDB::bind_method(D_METHOD("_set_transition_res_list", "place_holder"), &FsmRes::_set_transition_res_list);

    auto typed_TransitionRes_array_hint_string = String("{0}/{1}:TransitionRes").format(Array::make(Variant::OBJECT, PROPERTY_HINT_RESOURCE_TYPE));
    ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "transition_res_list", PROPERTY_HINT_TYPE_STRING, typed_TransitionRes_array_hint_string), "_set_transition_res_list", "get_transition_res_list");

    ClassDB::bind_method(D_METHOD("get_variable_res_list"), &FsmRes::get_variable_res_list);
    ClassDB::bind_method(D_METHOD("_set_variable_res_list", "place_holder"), &FsmRes::_set_variable_res_list);
    auto typed_VariableRes_array_hint_string = String("{0}/{1}:HFSMVariableRes").format(Array::make(Variant::OBJECT, PROPERTY_HINT_RESOURCE_TYPE));
    ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "variable_res_list", PROPERTY_HINT_TYPE_STRING, typed_VariableRes_array_hint_string, godot::PropertyUsageFlags::PROPERTY_USAGE_STORAGE),
                 "_set_variable_res_list", "get_variable_res_list");

    ClassDB::bind_method(D_METHOD("add_state_res", "new_state_res"), &FsmRes::add_state_res);
    ClassDB::bind_method(D_METHOD("add_transition_res", "new_transition_res"), &FsmRes::add_transition_res);
    ClassDB::bind_method(D_METHOD("add_variable_res", "new_variable_res"), &FsmRes::add_variable_res);

    ClassDB::bind_method(D_METHOD("remove_state_res", "to_remove_state_res"), &FsmRes::remove_state_res);
    ClassDB::bind_method(D_METHOD("remove_transition_res", "to_remove_transition_res"), &FsmRes::remove_transition_res);
    ClassDB::bind_method(D_METHOD("remove_variable_res", "remove_variable_res"), &FsmRes::remove_variable_res);
}

FsmRes::FsmRes() {}
FsmRes::~FsmRes() {}

void FsmRes::set_nested_state_res(const Ref<StateRes> &state_res) {
    _nested_state_res = state_res;
    notify_property_list_changed();
}
Ref<StateRes> FsmRes::get_nested_state_res() const { return _nested_state_res; }

void FsmRes::add_state_res(Ref<StateRes> state_res) {
    if (_state_res_list.find(state_res) >= 0)
        return;
    bool ununique = false;
    do {
        ununique = false;
        for (size_t i = 0; i < _state_res_list.size(); i++) {
            Ref<StateRes> sr = _state_res_list[i];
            StringName name = state_res->get_name();
            StringName be_check = sr->get_name();
            if (name == be_check) {
                name = String("@") + String(name);
                state_res->call("set_name", name);
                ununique = true;
                break;
            }
        }
    } while (ununique);

    _state_res_list.push_back(state_res);
    emit_changed();
}

void FsmRes::add_transition_res(Ref<TransitionRes> transition_res) {
    if (_transition_res_list.find(transition_res) >= 0)
        return;
    Ref<StateRes> add_from_state = transition_res->call("get_from_state_res");
    Ref<StateRes> add_to_state = transition_res->call("get_to_state_res");
    for (size_t i = 0; i < _transition_res_list.size(); i++) {
        Ref<TransitionRes> tr = _transition_res_list[i];
        Ref<StateRes> existed_from_res = tr->call("get_from_state_res");
        Ref<StateRes> existed_to_res = tr->call("get_to_state_res");
        ERR_FAIL_COND_MSG(add_from_state == add_to_state && existed_from_res == existed_to_res, "不应发生:存在相同的转换");
    }

    _transition_res_list.push_back(transition_res);
}

void FsmRes::remove_transition_res(Ref<TransitionRes> transition_res) {
    if (_transition_res_list.find(transition_res) >= 0) {
        _transition_res_list.erase(transition_res);
    } else {
        Ref<StateRes> add_from_state = transition_res->call("get_from_state_res");
        Ref<StateRes> add_to_state = transition_res->call("get_to_state_res");
        for (size_t i = 0; i < _transition_res_list.size(); i++) {
            Ref<TransitionRes> tr = _transition_res_list[i];
            Ref<StateRes> existed_from_res = tr->call("get_from_state_res");
            Ref<StateRes> existed_to_res = tr->call("get_to_state_res");
            if (add_from_state == add_to_state && existed_from_res == existed_to_res) {
                _transition_res_list.erase(_transition_res_list[i]);
                UtilityFunctions::printerr("不应发生:"
                                           "不存在要移除的转换，但存在相同的连接方式，以将其移除。");
                return;
            }
        }
    }
}

Fsm *FsmRes::create_fsm(HFSM *hfsm, const Ref<State> &nested_state, const Vector<Hfsm::Fsm *> &nested_fsm_update_queue) {
    Fsm *r = memnew(Fsm);
    r->_hfsm = hfsm;
    // Fsm 不一定包含于 State
    if (nested_state.is_valid()) {
        r->_nested_state = nested_state;
        r->_path.append_array(r->_nested_state->get_path());
        r->_path.append(r->_nested_state);
        // TODO ??
        r->_fsm_update_queue.append_array(nested_fsm_update_queue);
    }
    r->_fsm_update_queue.push_back(r);

    // 构造状态列表
    auto state_res2state = VMap<Ref<StateRes>, Ref<State>>();
    for (size_t i = 0; i < _state_res_list.size(); i++) {
        Ref<StateRes> state_res = _state_res_list[i];
        auto state = state_res->create_state(hfsm, r);
        state_res2state.insert(state_res, state);

        r->_state_list.push_back(state);
    }
    // 构造转换列表
    for (size_t i = 0; i < _transition_res_list.size(); i++) {
        Ref<TransitionRes> transition_res = _transition_res_list[i];
        Ref<StateRes> from_res, to_res;
        TransitionBase *transition = transition_res->create_transition(hfsm, from_res, to_res);
        //  添加到起始状态的转换列表中
        auto from_state = state_res2state[from_res];
        static_cast<State *>(from_state.ptr())->_transition_list.append(transition);
        // 设置转换的起始与目标状态
        transition->_from_state = from_state;
        transition->_to_state = state_res2state[to_res];
    }
    // 整理起始与结束状态
    for (auto &&state : r->_state_list) {
        if (state->get_type() == State::STATE_TYPE_ENTRY) {
            r->_current_entry_state = state;
            // r->_default_entry_state = state;
        } else if (state->get_type() == State::STATE_TYPE_EXIT) {
            r->_current_exit_state_list.append(state);
            // r->_default_exit_state_list.append(state);
        }
    }

    return r;
}

void FsmRes::_set_variable_res_list(Array variable_res_list) {
    _variable_res_list = TypedArray<HFSMVariableRes>(variable_res_list);
    for (size_t i = 0; i < _variable_res_list.size(); i++) {
        Ref<HFSMVariableRes> vr = Object::cast_to<HFSMVariableRes>(_variable_res_list[i]);
        if (vr.is_valid()) {
            if (vr->get_fsm_res() != this) {
                vr->set_fsm_res(this);
            }
        } else {
            _variable_res_list[i] = HFSMVariableRes::create_new(this);
        }
    }
}
#pragma endregion

} // namespace Hfsm
