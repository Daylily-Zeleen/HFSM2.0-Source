/*
** EPITECH PROJECT, 2022
** HFSM
** File description:
** state.cpp
*/

#include "state.h"
#include "fsm.h"
#include "hfsm.h"
#include "transitions/transition_base.h"

#ifdef GDEXTENSION_BUILD
#include <godot_cpp/templates/list.hpp>

#ifdef TOOLS_ENABLED
#include <godot_cpp/classes/engine.hpp>
#endif // TOOLS_ENABLED

#else

#ifdef TOOLS_ENABLED
#include <core/config/engine.h>
#endif // TOOLS_ENABLED

#include <core/templates/list.h>

#endif // GDEXTENSION_BUILD

namespace Hfsm {

void State::_bind_methods() {
	GDBIND_BEGIN(State);

	GDBIND_METHOD(get_name);
	GDBIND_METHOD(get_hfsm);
	GDBIND_METHOD(is_exited);
	GDBIND_METHOD(manual_exit);
	GDBIND_METHOD(get_animation_name_for_playing);
	GDBIND_METHOD(is_animation_playing);
	GDADD_PROPERTY(STRING_NAME, animation_name);

	IF_FULL_VERSION({
		GDADD_PROPERTY(FLOAT, animation_blend_time);
		GDADD_PROPERTY(FLOAT, animation_speed);
		GDADD_PROPERTY_BOOL(animation_reverse);
	})

	GDVIRTUAL_BIND(_initialize);
	GDVIRTUAL_BIND(_entry);
	GDVIRTUAL_BIND(_update, "delta");
	GDVIRTUAL_BIND(_physics_update, "delta");
	GDVIRTUAL_BIND(_exit);

	// ClassDB::bind_method(D_METHOD("_initialize"), &State::_initialize);
	// ClassDB::bind_method(D_METHOD("_entry"), &State::_entry);
	// ClassDB::bind_method(D_METHOD("_update", "delta"), &State::_update);
	// ClassDB::bind_method(D_METHOD("_physics_update", "delta"),
	//                      &State::_physics_update);
	// ClassDB::bind_method(D_METHOD("_exit"), &State::_exit);

	// ClassDB::bind_method(D_METHOD("get_path"), &State::get_path);
	// ClassDB::bind_method(D_METHOD("get_type"), &State::get_type);

#ifdef ROLLBACK_NET_CODE
	BIND_VIRTUAL_METHOD(State, _save_state);
	BIND_VIRTUAL_METHOD(State, _load_state);
	BIND_VIRTUAL_METHOD(State, _interpolate_state);
	BIND_VIRTUAL_METHOD(State, _get_local_input);
	BIND_VIRTUAL_METHOD(State, _predict_remote_input);
	BIND_VIRTUAL_METHOD(State, _network_process);
	BIND_VIRTUAL_METHOD(State, _network_preprocess);
	BIND_VIRTUAL_METHOD(State, _network_postprocess);
	BIND_VIRTUAL_METHOD(State, _network_spawn_preprocess);
	BIND_VIRTUAL_METHOD(State, _network_spawn);
	BIND_VIRTUAL_METHOD(State, _network_despawn);
#endif

	BIND_CONSTANT(STATE_TYPE_NORMAL);
	BIND_CONSTANT(STATE_TYPE_ENTRY);
	BIND_CONSTANT(STATE_TYPE_EXIT);
	// BIND_CONSTANT(STATE_TYPE_MAX);

	ADD_SIGNAL(MethodInfo("animation_finished"));
}

void State::initialize_state() {
	IF_GDE(call(SNAME("_initialize"));)
	IF_GDM(GDVIRTUAL_CALL(_initialize);)
}

void State::entry_state() {
	IF_GDE(call(SNAME("_entry"));)
	IF_GDM(GDVIRTUAL_CALL(_entry);)
}
void State::update_state(real_t p_delta) {
	IF_GDE(call(SNAME("_update"), p_delta);)
	IF_GDM(GDVIRTUAL_CALL(_update, p_delta);)
}

void State::physics_update_state(real_t p_delta) {
	IF_GDE(call(SNAME("_physics_update"), p_delta);)
	IF_GDM(GDVIRTUAL_CALL(_physics_update, p_delta);)
}

void State::exit_state() {
	IF_GDE(call(SNAME("_exit"));)
	IF_GDM(GDVIRTUAL_CALL(_exit);)
}

State::~State() {
	for (auto &&transition : transition_list) {
		if (transition) {
			memdelete(transition);
		}
	}
	if (sub_fsm) {
		memdelete(sub_fsm);
	}
}

// setget
void State::set_name(const StringName &p_name) {
#ifdef TOOLS_ENABLED
	if (!Engine::get_singleton()->is_editor_hint()) {
		name = p_name;
	} else
#endif // TOOLS_ENABLED
	{
		if (name == StringName("")) {
			name = p_name;
		} else {
			ERR_FAIL_MSG("HFSM: Can not set state name when running.");
		}
	}
	if (animation_name == StringName()) {
		anim_name_for_playing = name;
	} else {
		anim_name_for_playing = animation_name;
	}
}

HFSM *State::get_hfsm() { return hfsm; }
bool State::is_exited() { return exited; }

void State::manual_exit() {
	if (!is_exited()) {
		exited = true;
		exit();
	}
}

void State::entry() {
	exited = false;
	for (auto &&transition : transition_list) {
		transition->refresh();
	}

	// TODO:: Playing anim first or calling entry_state() first to allow developers setup animation property first?
	entry_state();
	try_play_anim();

	if (sub_fsm) {
		sub_fsm->entry();
	}
	//  如果是退出状态，则在完成进入行为后立即退出
	if (type == STATE_TYPE_EXIT) {
		exit();
	}
}
void State::update(real_t p_delta) {
	if (!exited) {
		update_state(p_delta);
	}
}

void State::physics_update(real_t p_delta) {
	if (!exited) {
		physics_update_state(p_delta);
	}
}
void State::exit(bool p_terminated_by_upper_level) {
	if (!exited) {
		exited = true;
		if (!p_terminated_by_upper_level) {
			auto queue = List<Ref<State>>();
			queue.push_back(this);

			while (queue.back()->get()->get_sub_fsm() && queue.back()->get()->get_sub_fsm()->is_running()) {
				queue.push_back(queue.back()->get()->get_sub_fsm()->get_current_state());
			}

			while (!queue.is_empty()) {
				queue.back()->get()->exit(true);
				queue.pop_back();
			}
		} else if (sub_fsm && sub_fsm->is_running()) {
			sub_fsm->exit_by_state();
		}

		animation_playing = false;
		exit_state();
	}
}
// void State::reset() {}

// 新特性 动画状态机
inline void State::try_play_anim() {
	if (auto anim_player = hfsm->get_animation_player()) {
		auto anim = get_animation_name_for_playing();
		if (anim_player->has_animation(anim)) {
#ifdef FULL_VERSION
			anim_player->play(anim, animation_blend_time, animation_speed, animation_reverse);
#else
			anim_player->play(anim);
#endif
			animation_playing = true;
		}
	}
}

StringName State::get_animation_name() const { return animation_name; }
void State::set_animation_name(const StringName &p_anim_name) {
	animation_name = p_anim_name;
	if (!is_exited()) {
		try_play_anim();
	}

	if (animation_name != StringName()) {
		anim_name_for_playing = animation_name;
	} else {
		anim_name_for_playing = get_name();
	}
}

// bool State::is_animation_playing() const {
// 	if (auto ap = hfsm->get_animation_player()) {
// 		if (ap->get_current_animation() == animation_name) {
// 			return ap->is_playing();
// 		}
// 	}
// 	return false;
// }

#ifdef FULL_VERSION
double State::get_animation_blend_time() const { return animation_blend_time; }
void State::set_animation_blend_time(double p_blend_time) { animation_blend_time = p_blend_time; }
double State::get_animation_speed() const { return animation_speed; }
void State::set_animation_speed(double p_speed) { animation_speed = p_speed; }
bool State::is_animation_reverse() const { return animation_reverse; }
void State::set_animation_reverse(bool p_reverse) { animation_reverse = p_reverse; }
#endif

#ifdef ROLLBACK_NET_CODE
Array State::save_state() {
	Array ret;
	// 自己的状态
	ret.push_back(_save_state());
	// 子状态机状态
	if (_fsm) {
		ret.push_back(_fsm->_save_state());
	}
	// 转换的状态
	for (auto &&t : _transition_list) {
		ret.push_back(t->_save_state());
	}
}
void State::load_state(const Array &state) {
	uint64_t i = 0;
	// 自己的
	_load_state(state[i]);
	// 子状态机
	if (_fsm) {
		i++;
		_fsm->_load_state(state[i]);
	}
	// 转换的状态
	for (auto &&t : _transition_list) {
		i++;
		t->_load_state(state[i]);
	}
}
void State::interpolate_state(const Array &old_state, const Array &new_state, real_t weight) {
	uint64_t i = 0;
	// 自己的
	_interpolate_state(old_state[i], new_state[i], weight);
	// 子状态机
	if (_fsm) {
		i++;
		_fsm->_interpolate_state(old_state[i], new_state[i], weight);
	}
	// 转换的状态
	for (auto &&t : _transition_list) {
		i++;
		t->_interpolate_state(old_state[i], new_state[i], weight);
	}
}
Array State::get_local_input() {
	Array ret;
	// 自己的状态
	ret.push_back(_get_local_input());
	// 子状态机状态
	if (_fsm) {
		ret.push_back(_fsm->_get_local_input());
	}
	// 转换的状态
	for (auto &&t : _transition_list) {
		ret.push_back(t->_get_local_input());
	}
	return ret;
}
Array State::predict_remote_input(const Array &previous_input, int64_t ticks_since_real_input) {
	Array ret;
	// 自己的状态
	uint64_t i = 0;
	ret.push_back(_predict_remote_input(previous_input[i], ticks_since_real_input));
	// 子状态机状态
	if (_fsm) {
		i++;
		ret.push_back(_fsm->_predict_remote_input(previous_input[i], ticks_since_real_input));
	}
	// 转换的状态
	for (auto &&t : _transition_list) {
		i++;
		ret.push_back(t->_predict_remote_input(previous_input[i], ticks_since_real_input));
	}
	return ret;
}
void State::network_process(Array &input) {
	uint64_t i = 0;
	_network_process(Dictionary(input[i]));
	// 子状态机状态
	if (_fsm) {
		i++;
		_fsm->_network_process(Array(input[i]));
	}
	// 转换的状态
	for (auto &&t : _transition_list) {
		i++;
		t->_network_process(input[i]);
	}
}
void State::network_preprocess(Array &input) {
	uint64_t i = 0;
	_network_preprocess(Dictionary(input[i]));
	// 子状态机状态
	if (_fsm) {
		i++;
		_fsm->_network_preprocess(Array(input[i]));
	}
	// 转换的状态
	for (auto &&t : _transition_list) {
		i++;
		t->_network_preprocess(input[i]);
	}
}
void State::network_postprocess(Array &input) {
	uint64_t i = 0;
	_network_postprocess(Dictionary(input[i]));
	// 子状态机状态
	if (_fsm) {
		i++;
		_fsm->_network_postprocess(Array(input[i]));
	}
	// 转换的状态
	for (auto &&t : _transition_list) {
		i++;
		t->_network_postprocess(input[i]);
	}
}
Dictionary &State::network_spawn_preprocess(Dictionary &data) {
	_network_spawn_preprocess(data);
	// 子状态机状态
	if (_fsm) {
		_fsm->_network_spawn_preprocess(data);
	}
	// 转换的状态
	for (auto &&t : _transition_list) {
		t->_network_spawn_preprocess(data);
	}
	return data;
}
void State::network_spawn(Dictionary &data) {
	_network_spawn(data);
	// 子状态机状态
	if (_fsm) {
		_fsm->_network_spawn(data);
	}
	// 转换的状态
	for (auto &&t : _transition_list) {
		t->_network_spawn(data);
	}
}
void State::network_despawn() {
	_network_despawn();
	// 子状态机状态
	if (_fsm) {
		_fsm->_network_despawn();
	}
	// 转换的状态
	for (auto &&t : _transition_list) {
		t->_network_despawn();
	}
}

Variant State::_save_state() { return Variant(); }
void State::_load_state(const Variant &state) {}
void State::_interpolate_state(const Variant &old_state, const Variant &new_state, real_t weight) {}
Variant State::_get_local_input() { return Variant(); }
Variant State::_predict_remote_input(const Variant &previous_input, int64_t ticks_since_real_input) { return Variant(); }
void State::_network_process(Dictionary &input) {}
void State::_network_preprocess(Dictionary &input) {}
void State::_network_postprocess(Dictionary &input) {}
Dictionary &State::_network_spawn_preprocess(Dictionary &data) { return data; }
void State::_network_spawn(Dictionary &data) {}
void State::_network_despawn() {}
#endif

#pragma endregion

}; // namespace Hfsm