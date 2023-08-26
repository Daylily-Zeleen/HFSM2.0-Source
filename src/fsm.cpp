/**************************************************************************/
/*  fsm.cpp                                                               */
/**************************************************************************/
/*                         This file is part of:                          */
/*                   Hierarchical Finite State Machine                    */
/*            https://github.com/Daylily-Zeleen/HFSM2.0-Source            */
/**************************************************************************/
/* Copyright (c) 2023-present Daylily Zeleen.                             */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#include "fsm.h"
#include "hfsm.h"
#include "state.h"
#include "transitions/transition_base.h"

namespace HFSM2 {

#pragma region FSM

// void FSM::reset() {
// 	current_state = current_entry_state;
// 	for (auto &&state : state_list) {
// 		state->reset();
// 		if (state->get_sub_fsm()) {
// 			state->get_sub_fsm()->reset();
// 		}
// 	}
// }

void FSM::entry() { //(const Ref<State> &p_entry_state) {
	// if (reset_when_entry) {
	// 	for (auto &&state : state_list) {
	// 		state->reset();
	// 	}
	// }

	// if (p_entry_state.is_valid()) {
	// 	current_state = p_entry_state;
	// } else {
	current_state = current_entry_state;
	// }
	if (current_state.is_valid()) {
		current_state->entry();
		running = true;
		hfsm->emit_transited(Ref<State>(), current_state);
		hfsm->emit_entered(current_state);
	}
}

LocalVector<FSM *> *FSM::try_transit_and_get_update_queue() {
	ERR_FAIL_COND_V(!is_running(), nullptr);

	if (auto transition = current_state->try_transit()) {
		// 退出当前状态
		if (!current_state->is_exited()) {
			current_state->exit(false);
			hfsm->emit_exited(current_state);
		}

		auto to_state = transition->get_to_state();
		// 进入新状态（新状态如果是退出状态，则立刻完成进入行为后立刻退出，已在State::entry()中处理
		to_state->entry();
		hfsm->emit_transited(current_state, to_state);
		current_state = to_state;
		hfsm->emit_entered(current_state);
		// if _current_state in _current_exit_state_list:
	}

	if (!current_state->is_exited()) { // 当前状态仍在运行
		// 设置更新路径
		auto current_state_sub_fsm = current_state->get_sub_fsm();
		if (current_state_sub_fsm && current_state_sub_fsm->is_running()) {
			// 当前状态具有内嵌状态机且正在运行
			// 尝试对其进行转换并获取更新队列
			return current_state_sub_fsm->try_transit_and_get_update_queue();
		}
		return &fsm_update_queue;
	} else {
		// 已停止運行
		running = false;
	}
	return nullptr;
}

void FSM::update(double p_delta) {
	current_state->update(p_delta);
	hfsm->emit_updated(current_state, p_delta);
}

void FSM::physics_update(double p_delta) {
	current_state->physics_update(p_delta);
	hfsm->emit_physic_updated(current_state, p_delta);
}
void FSM::exit_by_state() {
	running = false;
	hfsm->emit_exited(current_state);
	hfsm->emit_transited(current_state, Ref<State>());
}

void FSM::set_nested_state(const Ref<State> &p_nested_state, const LocalVector<FSM *> &p_nested_fsm_update_queue) {
	// FSM 不一定包含于 State
	IF_DEV(ERR_FAIL_COND(p_nested_state.is_null());)

	nested_state = p_nested_state;
	path.append_array(nested_state->get_path());
	path.append(nested_state);

	fsm_update_queue.reserve(p_nested_fsm_update_queue.size() + 1);
	for (const auto &E : p_nested_fsm_update_queue) {
		fsm_update_queue.push_back(E);
	}
	fsm_update_queue.push_back(this);
}

// bool FSM::force_transit(const StringName &p_target_state_name) {
// 	auto target_state = get_state(p_target_state_name);
// 	return force_transit_state(target_state);
// }

// bool FSM::force_transit_state(Ref<State> &p_target_state) {
// 	ERR_FAIL_COND_V(p_target_state->get_sub_fsm() == this, false);
// 	ERR_FAIL_COND_V(p_target_state.is_null(), false);
// 	ERR_FAIL_COND_V(current_state.is_null(), false);
// 	//  退出当前状态
// 	if (!current_state->is_exited()) {
// 		current_state->exit();
// 	}
// 	hfsm->emit_transited(current_state, p_target_state);
// 	current_state = p_target_state;
// 	current_state->entry();
// 	hfsm->emit_entered(current_state);
// 	return true;
// }

// void FSM::exit() {
//     // ERR_FAIL_NULL(_current_state);
//     // _current_state->exit();
//     _running = false;
//     _hfsm->exited(_current_state);
//     _hfsm->transited(_current_state, Ref<State>());
// }

// Ref<State> FSM::get_state(const StringName &p_state_name) {
// 	for (auto &&state : state_list) {
// 		if (state->get_name() == p_state_name) {
// 			return state;
// 		}
// 	}
// 	return {};
// }

#ifdef ROLLBACK_NET_CODE
Array FSM::_save_state() {
	Array ret;
	ret.push_back(_current_state);
	ret.push_back(_running);
	for (auto &&state : _state_list) {
		ret.push_back(state->save_state());
	}
	return ret;
}
void FSM::_load_state(const Array &state) {
	uint64_t idx = 0;
	_running = state[idx];
	idx++;
	_current_state = state[idx];
	for (auto &&s : _state_list) {
		idx++;
		s->load_state(state[idx]);
	}
}
void FSM::_interpolate_state(const Array &old_state, const Array &new_state, real_t weight) {
	uint64_t idx = 1;
	for (auto &&s : _state_list) {
		idx++;
		s->interpolate_state(old_state[idx], new_state[idx], weight);
	}
}
Array FSM::_get_local_input() {
	Array ret;
	for (auto &&s : _state_list) {
		ret.push_back(s->get_local_input());
	}
	return ret;
}
Array FSM::_predict_remote_input(const Array &previous_input, int64_t ticks_since_real_input) {
	Array ret;
	for (size_t i = 0; i < _state_list.size(); i++) {
		ret.push_back(_state_list.get(i)->predict_remote_input(previous_input[i], ticks_since_real_input));
	}
	return ret;
}
void FSM::_network_process(Array &input) {
	for (size_t i = 0; i < _state_list.size(); i++) {
		_state_list.get(i)->network_process(Array(input[i]));
	}
}
void FSM::_network_preprocess(Array &input) {
	for (size_t i = 0; i < _state_list.size(); i++) {
		_state_list.get(i)->network_preprocess(Array(input[i]));
	}
}
void FSM::_network_postprocess(Array &input) {
	for (size_t i = 0; i < _state_list.size(); i++) {
		_state_list.get(i)->network_postprocess(Array(input[i]));
	}
}
Dictionary &FSM::_network_spawn_preprocess(Dictionary &data) {
	for (auto &&s : _state_list) {
		s->network_spawn_preprocess(data);
	}
	return data;
}
void FSM::_network_spawn(Dictionary &data) {
	for (auto &&s : _state_list) {
		s->network_spawn(data);
	}
}
void FSM::_network_despawn() {
	for (auto &&s : _state_list) {
		s->network_despawn();
	}
}

#endif

#pragma endregion

}; // namespace HFSM2
