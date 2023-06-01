#include "hfsm.hpp"
#include "fsm.hpp"
#include "hfsm_variable.hpp"
#include "hfsm_variable_res.hpp"
// #include "state.hpp"
#include "fsm_res.hpp"

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <utility>

using namespace godot;

namespace Hfsm {

bool HFSM::_set(const StringName &p_name, const Variant &p_property) {
	if (p_name == StringName("variable_list")) {
		root_fsm_res->set_variable_res_list(p_property.operator godot::Array());
		return true;
	} else if (p_name == StringName("root_fsm_res")) {
		set_root_fsm_res(Object::cast_to<FsmRes>(p_property.operator godot::Object *()));
		return true;
	}
	return false;
}
bool HFSM::_get(const StringName &p_name, Variant &r_property) const {
	if (p_name == StringName("variable_list")) {
		if (root_fsm_res.is_valid()) {
			r_property = root_fsm_res->get_variable_res_list();
		}
		return true;
	} else if (p_name == StringName("root_fsm_res")) {
		r_property = get_root_fsm_res();
		return true;
	}
	return false;
}
void HFSM::_get_property_list(List<PropertyInfo> *p_list) const {
	p_list->push_back(PropertyInfo(Variant::OBJECT, "root_fsm_res", PROPERTY_HINT_RESOURCE_TYPE, "FsmRes", PROPERTY_USAGE_STORAGE, "FsmRes"));
	if (Engine::get_singleton()->is_editor_hint()) {
		auto typed_VariableRes_array_hint_string = String("{0}/{1}:HFSMVariableRes").format(Array::make(Variant::OBJECT, PROPERTY_HINT_RESOURCE_TYPE));
		p_list->push_back(PropertyInfo(Variant::ARRAY, "variable_list", PROPERTY_HINT_TYPE_STRING, typed_VariableRes_array_hint_string, PROPERTY_USAGE_EDITOR));
	}
}

void HFSM::_bind_methods() {
	GDBIND_BEGIN(HFSM);
	GDBIND_METHOD(is_inited);
	GDBIND_METHOD(get_agents);
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

	GDADD_PROPERTY(INT, update_type, PROPERTY_HINT_ENUM, String("Idle And Physics,Idle,Physics,Manual"));

	GDADD_PROPERTY_BOOL(active, PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NONE);

	GDADD_PROPERTY_BOOL(debug);

	GDBIND_SETGET(root_fsm_res);

	GDADD_PROPERTY(DICTIONARY, context);

	GDADD_PROPERTY(OBJECT, animation_player, PROPERTY_HINT_NODE_PATH_TO_EDITED_NODE, "AnimationPlayer", PROPERTY_USAGE_DEFAULT);

	GDBIND_METHOD(___on_tree_entered__);
	GDBIND_METHOD(___on_ready__);
	GDBIND_METHOD(__on_animation_finished);

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
	BIND_CONSTANT(UPDATE_TYPE_IDLE_AND_PHYSICS);
	BIND_CONSTANT(UPDATE_TYPE_IDLE);
	BIND_CONSTANT(UPDATE_TYPE_PHYSICS);
	BIND_CONSTANT(UPDATE_TYPE_MANUAL);

	//  信号
	ADD_SIGNAL(MethodInfo("inited"));
	ADD_SIGNAL(MethodInfo("updated", PropertyInfo(Variant::OBJECT, "state", PROPERTY_HINT_NONE, "", 6U, StringName("State")), PropertyInfo(Variant::FLOAT, "delta")));
	ADD_SIGNAL(MethodInfo("physic_updated", PropertyInfo(Variant::OBJECT, "state", PROPERTY_HINT_NONE, "", 6U, StringName("State")), PropertyInfo(Variant::FLOAT, "delta")));
	ADD_SIGNAL(MethodInfo("transited", PropertyInfo(Variant::OBJECT, "from_state", PROPERTY_HINT_NONE, "", 6U, StringName("State")),
			PropertyInfo(Variant::OBJECT, "to_state", PROPERTY_HINT_NONE, "", 6U, StringName("State"))));
	ADD_SIGNAL(MethodInfo("entered", PropertyInfo(Variant::OBJECT, "state", PROPERTY_HINT_NONE, "", 6U, StringName("State"))));
	ADD_SIGNAL(MethodInfo("exited", PropertyInfo(Variant::OBJECT, "state", PROPERTY_HINT_NONE, "", 6U, StringName("State"))));
}

HFSM::HFSM() {
	root_fsm_res.instantiate();
#ifndef DEBUG_IN_EDITOR
	if (Engine::get_singleton()->is_editor_hint()) {
		return;
	}
#endif
	static const StringName sn = "tree_entered";
	connect(sn, TCALLABLE(___on_tree_entered__));
}

HFSM::~HFSM() {
	if (root_fsm) {
		memdelete(root_fsm);
	}
}
void HFSM::set_root_fsm_res(const Ref<FsmRes> &p_root_fsm_res) {
	root_fsm_res = p_root_fsm_res;
	notify_property_list_changed();
}
Ref<FsmRes> HFSM::get_root_fsm_res() const { return root_fsm_res; }

void HFSM::___on_tree_entered__() {
#ifndef DEBUG_IN_EDITOR
	if (Engine::get_singleton()->is_editor_hint()) {
		return;
	}
#endif
	if (is_inited()) {
		return; // 整个生命周期只初始化一次
	}

	static const StringName sn = "ready";
	if (get_owner()) {
		get_owner()->connect(sn, TCALLABLE(___on_ready__));
	} else {
		connect(sn, TCALLABLE(___on_ready__));
	}
}

void HFSM::___on_ready__() {
	static const StringName sn = "inited";
#ifndef DEBUG_IN_EDITOR
	if (Engine::get_singleton()->is_editor_hint()) {
		return;
	}
#endif
	// 拾取 代理节点
	auto keys = agents.keys();
	for (auto i = keys.size() - 1; i >= 0; i--) {
		Node *a;
		if (get_owner()) {
			a = get_owner()->get_node_or_null(agents[keys[static_cast<int>(i)]]);
		} else {
			a = get_node_or_null(agents[keys[static_cast<int>(i)]]);
		}
		if (a) {
			agents[keys[static_cast<int>(i)]] = a;
		} else {
			agents.erase(keys[static_cast<int>(i)]);
		}
	}

	// 为表达式转换提供必要参数
	expression_objs_names.resize(HfsmGlobal::name2singleton.size() + agents.size());
	expression_objs.resize(expression_objs_names.size());
	int idx = 0;
	auto arr = HfsmGlobal::name2singleton.get_array();
	for (auto i = 0; i < HfsmGlobal::name2singleton.size(); i++) {
		expression_objs_names.set(idx, arr[i].key);
		expression_objs[idx] = arr[i].value;
		idx++;
	}
	auto agent_names = agents.keys();
	for (auto i = 0; i < agent_names.size(); i++) {
		expression_objs_names.set(idx, agents[agent_names[i]]);
		expression_objs[idx] = agents[agent_names[i]];
		idx++;
	}

	// 生成hfsm
	generate_hfsm();
	// yield(get_tree(),"idle_frame")
	if (is_active()) {
		restart();
	}
	inited = true;
	emit_signal(sn);
}

void HFSM::_ready() {
	if (!root_fsm_res.is_valid()) {
		root_fsm_res.instantiate();
	}
#ifndef DEBUG_IN_EDITOR
	if (Engine::get_singleton()->is_editor_hint()) {
		set_process(false);
		set_physics_process(false);
	}
#endif
	// else // ?
	//     _root_fsm_res->is_deleted_state_script();
}

void HFSM::generate_hfsm() {
	trigger_list.clear();
	// 生成根状态机
	Vector<Hfsm::Fsm *> tmp;
	root_fsm = root_fsm_res->create_fsm(this, Ref<State>(), tmp);
	// 生成变量列表
	for (auto i = 0; i < root_fsm_res->variable_res_list.size(); i++) {
		Ref<HFSMVariableRes> vr = root_fsm_res->variable_res_list[i];
		variable_blackboard.insert(vr->get_variable_name(), vr->create_variable());
	}
	//
	set_debug(debug);
	//
	if (!Engine::get_singleton()->is_editor_hint()) {
		root_fsm_res.unref();
	}
}

void HFSM::manual_update() {
	ERR_FAIL_COND(is_active());
	ERR_FAIL_COND(update_type == UPDATE_TYPE_MANUAL);
	_process(get_process_delta_time());
}

void HFSM::manual_physics_update() {
	ERR_FAIL_COND(is_active());
	ERR_FAIL_COND(update_type == UPDATE_TYPE_MANUAL);
	_process(get_process_delta_time());
}

void HFSM::restart() {
	root_fsm->reset();
	set_active(true);
	set_update_type(update_type);
	root_fsm->entry();
}

Ref<HFSMVariable> HFSM::get_var(const StringName &p_variable_name) {
	ERR_FAIL_COND_V(!variable_blackboard.has(p_variable_name), Ref<HFSMVariable>());
	return variable_blackboard[p_variable_name];
}

Array HFSM::get_vars() {
	Array ret;
	ret.resize(variable_blackboard.size());
	auto arr = variable_blackboard.get_array();
	for (auto i = 0; i < ret.size(); i++) {
		ret[i] = arr[i].value;
	}
	return ret;
}

Variant HFSM::get_var_value(const StringName &p_variable_name) { return get_var(p_variable_name)->get_value(); }
Dictionary HFSM::get_vars_value() {
	Dictionary r;
	auto arr = variable_blackboard.get_array();
	for (size_t i = 0; i < variable_blackboard.size(); i++) {
		r[arr[i].key] = arr[i].value->get_value();
	}
	return r;
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

void HFSM::set_update_type(UpdateType p_update_type) {
	update_type = UpdateType(p_update_type);
#ifndef DEBUG_IN_EDITOR
	if (Engine::get_singleton()->is_editor_hint()) {
		return;
	}
#endif
	switch (update_type) {
		case UPDATE_TYPE_IDLE:
			set_physics_process(false);
			set_process(true);
			break;
		case UPDATE_TYPE_PHYSICS:
			set_physics_process(true);
			set_process(false);
			break;
		case UPDATE_TYPE_IDLE_AND_PHYSICS:
			set_physics_process(true);
			set_process(true);
			break;
		case UPDATE_TYPE_MANUAL:
			set_physics_process(false);
			set_process(false);
			break;
		default:
			break;
	}
	notify_property_list_changed();
}

void HFSM::_process(double p_delta) {
	if (is_active() && is_inited()) {
		switch (update_type) {
			case UPDATE_TYPE_PHYSICS:
			case UPDATE_TYPE_IDLE_AND_PHYSICS: {
				if (root_fsm->running) {
					root_fsm->check_transit_and_get_update_queue(active_fsm_list);
				}
				flush_trigger();
			} break;
			default:
				break;
		}

		for (auto &&fsm : *active_fsm_list) {
			fsm->update(p_delta);
			if (update_type == UPDATE_TYPE_IDLE) {
				fsm->physics_update(p_delta);
			}
		}
	}
}

void HFSM::_physics_process(double p_delta) {
	if (is_active() && is_inited()) {
		// TODO:: 两次检测？
		if (root_fsm->running) {
			root_fsm->check_transit_and_get_update_queue(active_fsm_list);
		}
		flush_trigger();

		for (auto &&fsm : *active_fsm_list) {
			fsm->physics_update(p_delta);
			if (update_type == UPDATE_TYPE_IDLE) {
				fsm->update(p_delta);
			}
		}
	}
}

void HFSM::flush_trigger() {
	for (auto &&t : trigger_list) {
		t->flush_trigger();
	}
}

// 信号发射器 , 由 fsm 调用
void HFSM::updated(Ref<State> &p_state, double p_delta) {
	static const StringName sn = "updated";
	switch (update_type) {
		case UPDATE_TYPE_IDLE_AND_PHYSICS:
		case UPDATE_TYPE_IDLE:
			emit_signal(sn, p_state, p_delta);
			break;
		default:
			break;
	}
}

void HFSM::physic_updated(Ref<State> &p_state, double p_delta) {
	static const StringName sn = "physic_updated";
	switch (update_type) {
		case UPDATE_TYPE_IDLE_AND_PHYSICS:
		case UPDATE_TYPE_PHYSICS:
			emit_signal(sn, p_state, p_delta);
			break;
		default:
			break;
	}
}

void HFSM::transited(Ref<State> &p_from_state, Ref<State> &p_to_state) {
	static const StringName sn = "transited";
	previous_state = p_from_state;
	current_state = p_to_state;
	if (current_state.is_valid() && animation_player) {
		auto anim = current_state->get_animation_name_for_playing();
		if (animation_player->has_animation(anim)) {
#ifdef FULL_VERSION
			animation_player->play(anim, current_state->get_animation_blend_time(), current_state->get_animation_speed(), current_state->is_animation_reverse());
#else
			animation_player->play(anim);
#endif
			current_state->animation_playing = true;
		} else {
			current_state->animation_playing = false;
		}
	} else {
		current_state->animation_playing = false;
	}
	if (current_state.is_valid()) {
		// 无论如何，只有最新的状态能播放状态。
		previous_state->animation_playing = false;
	}
	emit_signal(sn, p_from_state, p_to_state);
}

void HFSM::entered(Ref<State> &p_state) {
	static const StringName sn = "entered";
	emit_signal(sn, p_state);
}
void HFSM::exited(Ref<State> &p_state) {
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
	_active_fsm_list = reinterpret_cast<Vector<Fsm *> *>(int64_t(state[4]));
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
	auto cb = TCALLABLE(__on_animation_finished);
	if (animation_player && animation_player->is_connected(sn, cb)) {
		animation_player->disconnect(sn, cb);
	}
	animation_player = p_animtion_player;
	if (animation_player && !animation_player->is_connected(sn, cb)) {
		animation_player->connect(sn, cb);
	}
}

}; // namespace Hfsm
