/**************************************************************************/
/*  hfsm.cpp                                                              */
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

#include "hfsm.h"
#include "fsm.h"
#include "variable.h"
#include "variable_config.h"

#if defined(DEBUG_ENABLED) && defined(TOOLS_ENABLED)
#include "../editor/hfsm_debugger_plugin.h"
#endif // defined ( DEBUG_ENABLED) && defined (TOOLS_ENABLED)

#ifndef DEBUG_IN_EDITOR
#ifdef TOOLS_ENABLED
#ifdef GDEXTENSION_BUILD
#include <godot_cpp/classes/engine.hpp>
#else
#include <core/config/engine.h>
#endif // GDEXTENSION_BUILD
#endif // TOOLS_ENABLED
#endif // DEBUG_IN_EDITOR

using namespace godot;

namespace Hfsm {

bool HFSM::_set(const StringName &p_name, const Variant &p_property) {
	IF_TOOLS(
			if (p_name == StringName("variable_list")) {
				root_fsm_config->set_variable_config_list(p_property);
				return true;
			})
	return false;
}

bool HFSM::_get(const StringName &p_name, Variant &r_property) const {
	IF_TOOLS(
			if (p_name == StringName("variable_list")) {
				if (root_fsm_config.is_valid()) {
					r_property = root_fsm_config->get_variable_config_list();
				}
				return true;
			})
	return false;
}

void HFSM::_get_property_list(List<PropertyInfo> *p_list) const {
	IF_TOOLS(
			if (Engine::get_singleton()->is_editor_hint() && root_fsm_config.is_valid()) {
				auto typed_VariableRes_array_hint_string = vformat("%d/%d:%s", Variant::OBJECT, PROPERTY_HINT_RESOURCE_TYPE, VariableConfig::get_class_static());
				p_list->push_back(PropertyInfo(Variant::ARRAY, "variable_list", PROPERTY_HINT_TYPE_STRING, typed_VariableRes_array_hint_string, PROPERTY_USAGE_EDITOR));
			})
}

void HFSM::_bind_methods() {
	GDBIND_BEGIN(HFSM);
	// GDBIND_METHOD(get_agents);
	GDBIND_METHOD(get_current_state);
	GDBIND_METHOD(get_previous_state);
	GDBIND_METHOD(restart);

	GDBIND_METHOD(get_var, "variable_name");
	GDBIND_METHOD(get_vars);
	GDBIND_METHOD(get_var_value, "variable_name");
	GDBIND_METHOD(get_vars_value);

	GDBIND_METHOD(set_var, "variable_name", "value");
	GDBIND_METHOD(set_trigger, "trigger_name");
	GDBIND_METHOD(set_boolean, "boolean_name", "value");
	GDBIND_METHOD(set_integer, "interger_name", "value");
	GDBIND_METHOD(set_float, "float_name", "value");
	GDBIND_METHOD(set_string, "string_name", "value");

	GDBIND_METHOD(manual_update);
	GDBIND_METHOD(manual_physics_update);

	GDBIND_METHOD(rebuild_hfsm);

	GDADD_PROPERTY(INT, update_type, PROPERTY_HINT_ENUM, "Idle And Physics,Idle,Physics,Manual", PROPERTY_USAGE_DEFAULT, "HFSMUpdateType");

	GDADD_PROPERTY_BOOL(active, PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NONE);

	GDADD_PROPERTY(OBJECT, animation_player, PROPERTY_HINT_NODE_TYPE, AnimationPlayer::get_class_static(), PROPERTY_USAGE_DEFAULT, AnimationPlayer::get_class_static());

	GDADD_PROPERTY_RESOURCE(root_fsm_config);

	GDBIND_CALBACK(_animation_finished);

#ifdef ROLLBACK_NET_CODE
	BIND_VIRTUAL_METHOD(HFSM, _save_state);
	BIND_VIRTUAL_METHOD(HFSM, _load_state);
	BIND_VIRTUAL_METHOD(HFSM, _interpolate_state);
	BIND_VIRTUAL_METHOD(HFSM, _get_local_input);
	BIND_VIRTUAL_METHOD(HFSM, _predict_remote_input);
	BIND_VIRTUAL_METHOD(HFSM, _network_process);
	BIND_VIRTUAL_METHOD(HFSM, _network_preprocess);
	BIND_VIRTUAL_METHOD(HFSM, _network_postprocess);
	BIND_VIRTUAL_METHOD(HFSM, _network_spawn_preprocess);
	BIND_VIRTUAL_METHOD(HFSM, _network_spawn);
	BIND_VIRTUAL_METHOD(HFSM, _network_despawn);
#endif
	// 常量
	BIND_ENUM_CONSTANT(HFSM_UPDATE_TYPE_IDLE_AND_PHYSICS);
	BIND_ENUM_CONSTANT(HFSM_UPDATE_TYPE_IDLE);
	BIND_ENUM_CONSTANT(HFSM_UPDATE_TYPE_PHYSICS);
	BIND_ENUM_CONSTANT(HFSM_UPDATE_TYPE_MANUAL);

	//  信号
	const auto PropertyInfoState = [](const String &p_name = "state") {
		return PropertyInfo(Variant::OBJECT, p_name, PROPERTY_HINT_NONE, "",
				PROPERTY_USAGE_DEFAULT, State::get_class_static());
	};
	ADD_SIGNAL(MethodInfo("updated", PropertyInfoState(), PropertyInfo(Variant::FLOAT, "delta")));
	ADD_SIGNAL(MethodInfo("physic_updated", PropertyInfoState(), PropertyInfo(Variant::FLOAT, "delta")));
	ADD_SIGNAL(MethodInfo("transited", PropertyInfoState("from_state"), PropertyInfoState("to_state")));
	ADD_SIGNAL(MethodInfo("entered", PropertyInfoState()));
	ADD_SIGNAL(MethodInfo("exited", PropertyInfoState()));
}

HFSM::HFSM() = default;

HFSM::~HFSM() {
	if (root_fsm) {
		memdelete(root_fsm);
		IF_DEBUG_TOOL(HfsmDebuggerPlugin::send_debug_destroy(this);)
	}
}

void HFSM::set_root_fsm_config(const Ref<FSMConfig> &p_root_fsm_config) {
	root_fsm_config = p_root_fsm_config;
	notify_property_list_changed();
}

Ref<FSMConfig> HFSM::get_root_fsm_config() const {
	return root_fsm_config;
}

void HFSM::manual_update() {
	ERR_FAIL_COND(update_type != HFSM_UPDATE_TYPE_MANUAL);
	process_internal(get_process_delta_time());
}

void HFSM::manual_physics_update() {
	ERR_FAIL_COND(update_type != HFSM_UPDATE_TYPE_MANUAL);
	physics_process_internal(get_process_delta_time());
}

void HFSM::restart() {
	if (!root_fsm) {
		return;
	}
	// root_fsm->reset();
	set_active(true);
	set_update_type(update_type);
	root_fsm->entry();
}

Ref<Variable> HFSM::get_var(const StringName &p_variable_name) {
	ERR_FAIL_COND_V(!variable_blackboard.has(p_variable_name), Ref<Variable>());
	return variable_blackboard[p_variable_name];
}

TypedArray<Variable> HFSM::get_vars() {
	TypedArray<Variable> ret;
	ret.resize(variable_blackboard.size());
	auto arr = variable_blackboard.get_array();
	for (auto i = 0; i < ret.size(); i++) {
		ret[i] = arr[i].value;
	}
	return ret;
}

Variant HFSM::get_var_value(const StringName &p_variable_name) { return get_var(p_variable_name)->get_value(); }
Dictionary HFSM::get_vars_value() {
	Dictionary ret;
	auto arr = variable_blackboard.get_array();
	for (size_t i = 0; i < variable_blackboard.size(); i++) {
		ret[arr[i].key] = arr[i].value->get_value();
	}
	return ret;
}

void HFSM::set_var(const StringName &p_variable_name, const Variant &p_value) {
	ERR_FAIL_COND(!variable_blackboard.has(p_variable_name));
	variable_blackboard[p_variable_name]->set_value(p_value);
}

void HFSM::set_trigger(const StringName &p_trigger_name) { variable_blackboard[p_trigger_name]->set_value(true); }
void HFSM::set_boolean(const StringName &p_boolean_name, bool p_value) { variable_blackboard[p_boolean_name]->set_value(p_value); }
void HFSM::set_integer(const StringName &p_interger_name, int64_t p_value) { variable_blackboard[p_interger_name]->set_value(p_value); }
void HFSM::set_float(const StringName &p_float_name, double p_value) { variable_blackboard[p_float_name]->set_value(p_value); }
void HFSM::set_string(const StringName &p_string_name, const String &p_value) { variable_blackboard[p_string_name]->set_value(p_value); }

void HFSM::set_update_type(HFSMUpdateType p_update_type) {
	update_type = HFSMUpdateType(p_update_type);
#ifndef DEBUG_IN_EDITOR
	IF_TOOLS(
			if (Engine::get_singleton()->is_editor_hint()) {
				return;
			})
#endif // DEBUG_IN_EDITOR
	switch (update_type) {
		case HFSM_UPDATE_TYPE_IDLE: {
			set_physics_process(false);
			set_process(true);
		} break;
		case HFSM_UPDATE_TYPE_PHYSICS: {
			set_physics_process(true);
			set_process(false);
		} break;
		case HFSM_UPDATE_TYPE_IDLE_AND_PHYSICS: {
			set_physics_process(true);
			set_process(true);
		} break;
		case HFSM_UPDATE_TYPE_MANUAL: {
			set_physics_process(false);
			set_process(false);
		} break;
		default:
			break;
	}
	notify_property_list_changed();
}

bool HFSM::rebuild_hfsm() {
	ERR_FAIL_COND_V(root_fsm_config.is_null(), false);

	if (root_fsm) {
		memdelete(root_fsm);
		IF_DEBUG_TOOL(HfsmDebuggerPlugin::send_debug_destroy(this);)
	}

	// Clear Variables
	trigger_list.clear();
	while (!variable_blackboard.is_empty()) {
		variable_blackboard.erase(variable_blackboard.getk(variable_blackboard.size() - 1));
	}

	// Build
	root_fsm = root_fsm_config->create_fsm(this);

	// Create variables.
	auto variable_config_list = root_fsm_config->get_variable_config_list();
	for (auto i = 0; i < variable_config_list.size(); i++) {
		Ref<VariableConfig> vc = variable_config_list[i];
		auto var = vc->create_variable();

		variable_blackboard.insert(var->get_variable_name(), var);

		if (var->is_trigger()) {
			trigger_list.push_back(var);
		}
	}

	IF_DEBUG_TOOL(HfsmDebuggerPlugin::send_debug_built(this);)
	return true;
}

void HFSM::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
#ifndef DEBUG_IN_EDITOR
			IF_TOOLS(
					if (Engine::get_singleton()->is_editor_hint()) {
						set_process(false);
						set_physics_process(false);
						return;
					})
#endif // DEBUG_IN_EDITOR

			IF_TOOLS({
				if (!Engine::get_singleton()->is_editor_hint()) {
					// 生成hfsm
					if (rebuild_hfsm()) {
						if (is_active()) {
							restart();
						}
					}
				}
			})

			IF_NOT_TOOLS(if (rebuild_hfsm()) {
				if (is_active()) {
					restart();
				}
			})

		} break;
		case NOTIFICATION_PROCESS: {
			if (!is_active()) {
				return;
			}
			process_internal(get_process_delta_time());
		} break;
		case NOTIFICATION_PHYSICS_PROCESS: {
			if (!is_active()) {
				return;
			}
			physics_process_internal(get_physics_process_delta_time());
		} break;
#if defined(DEBUG_ENABLED) && defined(TOOLS_ENABLED)
		case NOTIFICATION_ENTER_TREE: {
			if (root_fsm) {
				HfsmDebuggerPlugin::send_debug_built(this);
			}
		} break;
		case NOTIFICATION_EXIT_TREE: {
			if (root_fsm) {
				HfsmDebuggerPlugin::send_debug_destroy(this);
			}
		} break;
#endif // defined ( DEBUG_ENABLED) && defined (TOOLS_ENABLED)

		default:
			break;
	}
}

void HFSM::process_internal(double p_delta) {
	switch (update_type) {
		case HFSM_UPDATE_TYPE_PHYSICS:
		case HFSM_UPDATE_TYPE_IDLE_AND_PHYSICS: {
			if (root_fsm->is_running()) {
				active_fsm_list = root_fsm->try_transit_and_get_update_queue();
			}
			flush_trigger();
		} break;
		default:
			break;
	}

	if (!active_fsm_list) {
		return;
	}

	for (auto &&fsm : *active_fsm_list) {
		fsm->update(p_delta);
		if (update_type == HFSM_UPDATE_TYPE_IDLE) {
			fsm->physics_update(p_delta);
		}
	}
}

void HFSM::physics_process_internal(double p_delta) {
	if (root_fsm->is_running()) {
		active_fsm_list = root_fsm->try_transit_and_get_update_queue();
	}

	flush_trigger();

	if (!active_fsm_list) {
		return;
	}

	for (auto &&fsm : *active_fsm_list) {
		fsm->physics_update(p_delta);
		if (update_type == HFSM_UPDATE_TYPE_IDLE) {
			fsm->update(p_delta);
		}
	}
}

void HFSM::flush_trigger() {
	for (auto &&t : trigger_list) {
		t->flush_trigger();
	}
}

// 信号发射器 , 由 fsm 调用
void HFSM::emit_updated(const Ref<State> &p_state, double p_delta) {
	static const StringName sn = "updated";
	switch (update_type) {
		case HFSM_UPDATE_TYPE_IDLE_AND_PHYSICS:
		case HFSM_UPDATE_TYPE_IDLE:
			emit_signal(sn, p_state, p_delta);
			break;
		default:
			break;
	}
}

void HFSM::emit_physic_updated(const Ref<State> &p_state, double p_delta) {
	static const StringName sn = "physic_updated";
	switch (update_type) {
		case HFSM_UPDATE_TYPE_IDLE_AND_PHYSICS:
		case HFSM_UPDATE_TYPE_PHYSICS:
			emit_signal(sn, p_state, p_delta);
			break;
		default:
			break;
	}
}

void HFSM::emit_transited(const Ref<State> &p_from_state, const Ref<State> &p_to_state) {
	static const StringName sn = "transited";
	previous_state = p_from_state;
	current_state = p_to_state;
	emit_signal(sn, p_from_state, p_to_state);
	IF_DEBUG_TOOL(HfsmDebuggerPlugin::send_debug_update_active_path(this));
}

void HFSM::emit_entered(const Ref<State> &p_state) {
	static const StringName sn = "entered";
	emit_signal(sn, p_state);
}
void HFSM::emit_exited(const Ref<State> &p_state) {
	static const StringName sn = "exited";
	emit_signal(sn, p_state);
}

#ifdef ROLLBACK_NET_CODE

Array HFSM::_save_state() {
	Array ret;
	// 保存变量状态
	Array vs;
	auto arr = _variable_blackboard.get_array();
	for (size_t i = 0; i < _variable_blackboard.size(); i++) {
		vs.push_back(arr[i].value->get_value());
	}
	ret.push_back(vs);
	// 保存根状态机状态
	ret.push_back(_root_fsm->_save_state());
	// 保存自身属性
	ret.push_back(_current_state);
	ret.push_back(_previous_state);
	ret.push_back(int64_t(_active_fsm_list));
	ret.push_back(_active);
	// ret.push_back(_force_all_state_entry_behavior);
	// ret.push_back(_force_all_fsm_entry_behavior);

	return ret;
}
void HFSM::_load_state(const Array &state) {
	// 重载变量
	Array vs = state[0];
	auto arr = _variable_blackboard.get_array();
	for (size_t i = 0; i < _variable_blackboard.size(); i++) {
		arr[i].value->set_value(vs[i]);
	}
	// 重载状态
	_root_fsm->_load_state(state[1]);
	// 自身状态
	_current_state = Ref<State>(state[2]);
	_previous_state = state[3];
	_active_fsm_list = reinterpret_cast<Vector<FSM *> *>(int64_t(state[4]));
	_active = state[5];
	// _force_all_state_entry_behavior = state[6];
	// _force_all_fsm_entry_behavior = state[7];
}
void HFSM::_interpolate_state(const Array &old_state, const Array &new_state, real_t weight) {
	// 状态
	_root_fsm->_interpolate_state(old_state[1], new_state[1], weight);
}
Array HFSM::_get_local_input() { return _root_fsm->_get_local_input(); }
Array HFSM::_predict_remote_input(const Array &previous_input, int64_t ticks_since_real_input) { return _root_fsm->_predict_remote_input(previous_input, ticks_since_real_input); }
void HFSM::_network_process(Array &input) { _root_fsm->_network_process(input); }

void HFSM::_network_preprocess(Array &input) { _root_fsm->_network_preprocess(input); }
void HFSM::_network_postprocess(Array &input) { _root_fsm->_network_postprocess(input); }
Dictionary &HFSM::_network_spawn_preprocess(Dictionary &data) { return _root_fsm->_network_spawn_preprocess(data); }
void HFSM::_network_spawn(Dictionary &data) { return _root_fsm->_network_spawn(data); }
void HFSM::_network_despawn() { return _root_fsm->_network_despawn(); }

#endif

void HFSM::set_animation_player(AnimationPlayer *p_animtion_player) {
	static const StringName sn = "animation_finished";
	auto cb = TCALLABLE(_animation_finished);
	if (animation_player && animation_player->is_connected(sn, cb)) {
		animation_player->disconnect(sn, cb);
	}
	animation_player = p_animtion_player;
	if (animation_player && !animation_player->is_connected(sn, cb)) {
		animation_player->connect(sn, cb);
	}
}
}; // namespace Hfsm
