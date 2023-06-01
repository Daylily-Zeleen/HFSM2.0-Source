#include "fsm_res.hpp"

#include "fsm.hpp"
#include "state_res.hpp"
#include "transition_res.hpp"
#include "transitions/transition_base.hpp"

namespace Hfsm {

#pragma region FsmRes

void FsmRes::_bind_methods() {
	GDBIND_BEGIN(FsmRes);
	GDADD_PROPERTY_RESOURCE(nested_state_res, PROPERTY_USAGE_NONE);

	GDADD_PROPERTY_TYPED_ARRAY(state_res_list, StateRes);
	GDADD_PROPERTY_TYPED_ARRAY(transition_res_list, TransitionRes);
	GDADD_PROPERTY_TYPED_ARRAY(variable_res_list, HFSMVariableRes, PROPERTY_USAGE_STORAGE);

	GDBIND_METHOD(add_state_res, "new_state_res");
	GDBIND_METHOD(add_transition_res, "new_transition_res");
	GDBIND_METHOD(add_variable_res, "new_variable_res");

	GDBIND_METHOD(remove_state_res, "to_remove_state_res");
	GDBIND_METHOD(remove_transition_res, "to_remove_transition_res");
	GDBIND_METHOD(remove_variable_res, "remove_variable_res");
}

void FsmRes::set_nested_state_res(const Ref<StateRes> &p_state_res) {
	nested_state_res = p_state_res;
	emit_changed();
}
Ref<StateRes> FsmRes::get_nested_state_res() const { return nested_state_res; }

void FsmRes::add_state_res(const Ref<StateRes> &p_state_res) {
	if (state_res_list.find(p_state_res) >= 0) {
		return;
	}
	bool ununique = false;
	do {
		ununique = false;
		for (auto i = 0; i < state_res_list.size(); i++) {
			Ref<StateRes> sr = state_res_list[i];
			StringName name = p_state_res->get_state_name();
			StringName be_check = sr->get_state_name();
			if (name == be_check) {
				name = String("@") + String(name);
				p_state_res->set_state_name(name);
				ununique = true;
				break;
			}
		}
	} while (ununique);

	state_res_list.push_back(p_state_res);
	emit_changed();
}

void FsmRes::add_transition_res(const Ref<TransitionRes> &p_transition_res) {
	if (transition_res_list.find(p_transition_res) >= 0) {
		return;
	}
	Ref<StateRes> add_from_state = p_transition_res->get_from_state_res();
	Ref<StateRes> add_to_state = p_transition_res->get_to_state_res();
	for (size_t i = 0; i < transition_res_list.size(); i++) {
		Ref<TransitionRes> tr = transition_res_list[i];
		Ref<StateRes> existed_from_res = tr->get_from_state_res();
		Ref<StateRes> existed_to_res = tr->get_to_state_res();
		ERR_FAIL_COND_MSG(add_from_state == add_to_state && existed_from_res == existed_to_res, "不应发生:存在相同的转换");
	}

	transition_res_list.push_back(p_transition_res);
}

void FsmRes::remove_transition_res(const Ref<TransitionRes> &p_transition_res) {
	if (transition_res_list.find(p_transition_res) >= 0) {
		transition_res_list.erase(p_transition_res);
	} else {
		Ref<StateRes> add_from_state = p_transition_res->get_from_state_res();
		Ref<StateRes> add_to_state = p_transition_res->get_to_state_res();
		for (size_t i = 0; i < transition_res_list.size(); i++) {
			Ref<TransitionRes> tr = transition_res_list[i];
			Ref<StateRes> existed_from_res = tr->get_from_state_res();
			Ref<StateRes> existed_to_res = tr->get_to_state_res();
			if (add_from_state == add_to_state && existed_from_res == existed_to_res) {
				transition_res_list.erase(transition_res_list[i]);
				UtilityFunctions::printerr("不应发生: 不存在要移除的转换，但存在相同的连接方式，以将其移除。");
				return;
			}
		}
	}
}

Fsm *FsmRes::create_fsm(HFSM *p_hfsm, const Ref<State> &p_nested_state, const Vector<Hfsm::Fsm *> &p_nested_fsm_update_queue) {
	Fsm *r = memnew(Fsm);
	r->hfsm = p_hfsm;
	// Fsm 不一定包含于 State
	if (p_nested_state.is_valid()) {
		r->nested_state = p_nested_state;
		r->path.append_array(r->nested_state->get_path());
		r->path.append(r->nested_state);
		// TODO ??
		r->fsm_update_queue.append_array(p_nested_fsm_update_queue);
	}
	r->fsm_update_queue.push_back(r);

	// 构造状态列表
	auto state_res2state = VMap<Ref<StateRes>, Ref<State>>();
	for (size_t i = 0; i < state_res_list.size(); i++) {
		Ref<StateRes> state_res = state_res_list[i];
		auto state = state_res->create_state(p_hfsm, r);
		state_res2state.insert(state_res, state);

		r->state_list.push_back(state);
	}
	// 构造转换列表
	for (size_t i = 0; i < transition_res_list.size(); i++) {
		Ref<TransitionRes> transition_res = transition_res_list[i];
		Ref<StateRes> from_res, to_res;
		TransitionBase *transition = transition_res->create_transition(p_hfsm, from_res, to_res);
		//  添加到起始状态的转换列表中
		auto from_state = state_res2state[from_res];
		static_cast<State *>(from_state.ptr())->transition_list.append(transition);
		// 设置转换的起始与目标状态
		transition->from_state = from_state;
		transition->to_state = state_res2state[to_res];
	}
	// 整理起始与结束状态
	for (auto &&state : r->state_list) {
		if (state->get_type() == State::STATE_TYPE_ENTRY) {
			r->current_entry_state = state;
			// r->_default_entry_state = state;
		} else if (state->get_type() == State::STATE_TYPE_EXIT) {
			r->current_exit_state_list.append(state);
			// r->_default_exit_state_list.append(state);
		}
	}

	return r;
}

void FsmRes::set_variable_res_list(const Array &p_variable_res_list) {
	variable_res_list = decltype(variable_res_list)(p_variable_res_list);
	for (size_t i = 0; i < variable_res_list.size(); i++) {
		Ref<HFSMVariableRes> vr = Object::cast_to<HFSMVariableRes>(variable_res_list[i]);
		if (vr.is_valid()) {
			if (vr->get_fsm_res() != this) {
				vr->set_fsm_res(this);
			}
		} else {
			variable_res_list[i] = HFSMVariableRes::create_new(this);
		}
	}
}
#pragma endregion

} // namespace Hfsm
