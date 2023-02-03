#include "fsm.hpp"
#include "hfsm.hpp"
#include "state.hpp"
#include "transitions/transition_base.hpp"

namespace Hfsm {

#pragma region Fsm

Fsm::Fsm() = default;
Fsm::~Fsm() = default;

void Fsm::reset() {
	_current_state = _current_entry_state;
	for (auto &&state : _state_list) {
		state->reset();
		if (state->_fsm) {
			state->_fsm->reset();
		}
	}
}

void Fsm::entry(Ref<State> entry_state) {
	static auto empty_state = Ref<State>();
	if (_reset_when_entry) {
		for (auto &&state : _state_list) {
			state->reset();
		}
	}

	if (entry_state.is_valid()) {
		_current_state = entry_state;
	} else {
		_current_state = _current_entry_state;
	}
	if (_current_state.is_valid()) {
		_current_state->entry();
		_running = true;
		_hfsm->transited(empty_state, _current_state);
		_hfsm->entered(_current_state);
	}
}
void Fsm::check_transit_and_get_update_queue(Vector<Fsm *> *&update_queue) {
	ERR_FAIL_COND(_running == false);
	for (auto &&transition : _current_state->_transition_list) {
		// TODO:: 改为真正的虚方法
		if (transition->can_transit()) {
			// 退出当前状态
			if (!_current_state->is_exited()) {
				_current_state->exit(false);
				_hfsm->exited(_current_state);
			}

			// TODO:: 是否应该在进入新状态完成后再发射信号？
			// 暂定未进入
			// 切换并进入新状态, hfsm 先切换并发射信号，FSM 再进行切换
			auto to_state = transition->get_to_state();
			_hfsm->transited(_current_state, to_state);
			_current_state = to_state;
			// 进入新状态（新状态如果是退出状态，则立刻完成进入行为后立刻退出，已在State::entry()中处理
			_current_state->entry();
			_hfsm->entered(_current_state);
			// if _current_state in _current_exit_state_list:
			// 跳出循环
			break;
		}
	}

	if (!_current_state->is_exited()) { // 当前状态仍在运行
		// 设置更新路径
		update_queue = &_fsm_update_queue;
		if (_current_state->_fsm && _current_state->_fsm->_running) {
			// 当前状态具有内嵌状态机且正在运行
			// 尝试对其进行转换并获取更新队列
			_current_state->_fsm->check_transit_and_get_update_queue(update_queue);
		}
	} else {
		// 已停止運行
		_running = false;
	}
	// 当前状态机不再运行时不对参数做改变
}
void Fsm::update(double delta) {
	_current_state->update(delta);
	_hfsm->updated(_current_state, delta);
}
void Fsm::physics_update(double delta) {
	_current_state->physics_update(delta);
	_hfsm->physic_updated(_current_state, delta);
}
bool Fsm::force_transit(const StringName &target_state_name) {
	auto target_state = get_state(target_state_name);
	ERR_FAIL_COND_V(!target_state.is_valid(), false);
	ERR_FAIL_COND_V(!_current_state.is_valid(), false);
	//  退出当前状态
	if (!_current_state->is_exited()) {
		_current_state->exit();
	}
	_hfsm->transited(_current_state, target_state);
	_current_state = target_state;
	_current_state->entry();
	_hfsm->entered(_current_state);
	return true;
}
bool Fsm::force_transit_state(Ref<State> &target_state) {
	ERR_FAIL_NULL_V(target_state, false);
	ERR_FAIL_COND_V(target_state->_fsm == this, false);
	ERR_FAIL_COND_V(!_current_state.is_valid(), false);
	//  退出当前状态
	if (!_current_state->is_exited()) {
		_current_state->exit();
	}
	_hfsm->transited(_current_state, target_state);
	_current_state = target_state;
	_current_state->entry();
	_hfsm->entered(_current_state);
	return true;
}

#ifdef ROLLBACK_NET_CODE
Array Fsm::_save_state() {
	Array ret;
	ret.push_back(_current_state);
	ret.push_back(_running);
	for (auto &&state : _state_list) {
		ret.push_back(state->save_state());
	}
	return ret;
}
void Fsm::_load_state(const Array &state) {
	uint64_t idx = 0;
	_running = state[idx];
	idx++;
	_current_state = state[idx];
	for (auto &&s : _state_list) {
		idx++;
		s->load_state(state[idx]);
	}
}
void Fsm::_interpolate_state(const Array &old_state, const Array &new_state, real_t weight) {
	uint64_t idx = 1;
	for (auto &&s : _state_list) {
		idx++;
		s->interpolate_state(old_state[idx], new_state[idx], weight);
	}
}
Array Fsm::_get_local_input() {
	Array ret;
	for (auto &&s : _state_list) {
		ret.push_back(s->get_local_input());
	}
	return ret;
}
Array Fsm::_predict_remote_input(const Array &previous_input, int64_t ticks_since_real_input) {
	Array ret;
	for (size_t i = 0; i < _state_list.size(); i++) {
		ret.push_back(_state_list.get(i)->predict_remote_input(previous_input[i], ticks_since_real_input));
	}
	return ret;
}
void Fsm::_network_process(Array &input) {
	for (size_t i = 0; i < _state_list.size(); i++) {
		_state_list.get(i)->network_process(Array(input[i]));
	}
}
void Fsm::_network_preprocess(Array &input) {
	for (size_t i = 0; i < _state_list.size(); i++) {
		_state_list.get(i)->network_preprocess(Array(input[i]));
	}
}
void Fsm::_network_postprocess(Array &input) {
	for (size_t i = 0; i < _state_list.size(); i++) {
		_state_list.get(i)->network_postprocess(Array(input[i]));
	}
}
Dictionary &Fsm::_network_spawn_preprocess(Dictionary &data) {
	for (auto &&s : _state_list) {
		s->network_spawn_preprocess(data);
	}
	return data;
}
void Fsm::_network_spawn(Dictionary &data) {
	for (auto &&s : _state_list) {
		s->network_spawn(data);
	}
}
void Fsm::_network_despawn() {
	for (auto &&s : _state_list) {
		s->network_despawn();
	}
}

#endif

// void Fsm::exit() {
//     // ERR_FAIL_NULL(_current_state);
//     // _current_state->exit();
//     _running = false;
//     _hfsm->exited(_current_state);
//     _hfsm->transited(_current_state, Ref<State>());
// }
void Fsm::exit_by_state() {
	_running = false;
	_hfsm->exited(_current_state);
	_hfsm->transited(_current_state, ([]() -> Ref<State> & {
		static Ref<State> null = {};
		return null;
	})());
}
Ref<State> Fsm::get_state(const StringName &state_name) {
	for (auto &&state : _state_list) {
		if (state->get_name() == state_name) {
			return state;
		}
	}
	return {};
}

#pragma endregion

}; // namespace Hfsm