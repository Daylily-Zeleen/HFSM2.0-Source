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
		_root_fsm_res->_set_variable_res_list(p_property.operator godot::Array());
		return true;
	} else if (p_name == StringName("root_fsm_res")) {
		set_root_fsm_res(Object::cast_to<FsmRes>(p_property.operator godot::Object *()));
		return true;
	}
	return false;
}
bool HFSM::_get(const StringName &p_name, Variant &r_property) const {
	if (p_name == StringName("variable_list")) {
		if (_root_fsm_res.is_valid()) {
			r_property = _root_fsm_res->get_variable_res_list();
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
	ClassDB::bind_method(D_METHOD("is_inited"), &HFSM::is_inited);
	ClassDB::bind_method(D_METHOD("get_agents"), &HFSM::get_agents);
	ClassDB::bind_method(D_METHOD("get_current_state"), &HFSM::get_current_state);
	ClassDB::bind_method(D_METHOD("get_previous_state"), &HFSM::get_previous_state);
	ClassDB::bind_method(D_METHOD("restart"), &HFSM::restart);

	ClassDB::bind_method(D_METHOD("get_var", "variable_name"), &HFSM::get_var);
	ClassDB::bind_method(D_METHOD("get_vars"), &HFSM::get_vars);
	ClassDB::bind_method(D_METHOD("get_var_value", "variable_name"), &HFSM::get_var_value);
	ClassDB::bind_method(D_METHOD("get_vars_value"), &HFSM::get_vars_value);

	ClassDB::bind_method(D_METHOD("set_var", "variable_name", "value"), &HFSM::set_var);
	ClassDB::bind_method(D_METHOD("set_trigger", "trigger_name"), &HFSM::set_trigger);
	ClassDB::bind_method(D_METHOD("set_boolean", "boolean_name", "value"), &HFSM::set_boolean);
	ClassDB::bind_method(D_METHOD("set_integer", "interger_name", "value"), &HFSM::set_integer);
	ClassDB::bind_method(D_METHOD("set_float", "float_name", "value"), &HFSM::set_float);
	ClassDB::bind_method(D_METHOD("set_string", "string_name", "value"), &HFSM::set_string);

	ClassDB::bind_method(D_METHOD("manual_update"), &HFSM::manual_update);
	ClassDB::bind_method(D_METHOD("manual_physics_update"), &HFSM::manual_physics_update);

	ClassDB::bind_method(D_METHOD("get_update_type"), &HFSM::get_update_type);
	ClassDB::bind_method(D_METHOD("set_update_type", "update_type"), &HFSM::set_update_type);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "update_type", PROPERTY_HINT_ENUM, String("Idle And Physics,Idle,Physics,Manual")), "set_update_type", "get_update_type");

	ClassDB::bind_method(D_METHOD("is_active"), &HFSM::is_active);
	ClassDB::bind_method(D_METHOD("set_active", "active"), &HFSM::set_active);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "active"), "set_active", "is_active");
	// ADD_PROPERTY(PropertyInfo(Variant::BOOL, "active", PROPERTY_HINT_NONE,
	// "",
	//                           PROPERTY_USAGE_NONE),
	//              "set_active", "is_active");

	ClassDB::bind_method(D_METHOD("is_debug"), &HFSM::is_debug);
	ClassDB::bind_method(D_METHOD("set_debug", "debug"), &HFSM::set_debug);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "debug"), "set_debug", "is_debug");

	ClassDB::bind_method(D_METHOD("get_root_fsm_res"), &HFSM::get_root_fsm_res);
	ClassDB::bind_method(D_METHOD("set_root_fsm_res", "root_fsm_res"), &HFSM::set_root_fsm_res);

	ClassDB::bind_method(D_METHOD("get_context"), &HFSM::get_context);
	ClassDB::bind_method(D_METHOD("set_context", "context"), &HFSM::set_context);
	ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "context"), "set_context", "get_context");

	ClassDB::bind_method(D_METHOD("get_animation_player"), &HFSM::get_animation_player);
	ClassDB::bind_method(D_METHOD("set_animation_player", "animation_player"), &HFSM::set_animation_player);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "animation_player", PROPERTY_HINT_NODE_PATH_TO_EDITED_NODE, "AnimationPlayer", PROPERTY_USAGE_DEFAULT), "set_animation_player", "get_animation_player");

	ClassDB::bind_method(D_METHOD("___on_tree_entered__"), &HFSM::___on_tree_entered__);
	ClassDB::bind_method(D_METHOD("___on_ready__"), &HFSM::___on_ready__);
	ClassDB::bind_method(D_METHOD("__on_animation_finished"), &HFSM::__on_animation_finished);

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
	static const StringName sn = "tree_entered";
	static const StringName md = "___on_tree_entered__";
	_root_fsm_res.instantiate();
#ifndef DEBUG_IN_EDITOR
	if (Engine::get_singleton()->is_editor_hint()) {
		return;
	}
#endif
	connect(sn, Callable(this, md));
}
HFSM::~HFSM() {
	if (_root_fsm) {
		memdelete(_root_fsm);
	}
}
void HFSM::set_root_fsm_res(const Ref<FsmRes> &root_fsm_res) {
	_root_fsm_res = root_fsm_res;
	notify_property_list_changed();
}
Ref<FsmRes> HFSM::get_root_fsm_res() const { return _root_fsm_res; }

void HFSM::___on_tree_entered__() {
#ifndef DEBUG_IN_EDITOR
	if (Engine::get_singleton()->is_editor_hint()) {
		return;
	}
#endif
	static const StringName sn = "ready";
	static const StringName md = "___on_ready__";
	if (is_inited()) {
		return; // 整个生命周期只初始化一次
	}
	if (get_owner()) {
		get_owner()->connect(sn, Callable(this, md));
	} else {
		connect(sn, Callable(this, md));
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
	auto keys = _agents.keys();
	for (auto i = keys.size() - 1; i >= 0; i--) {
		Node *a;
		if (get_owner()) {
			a = get_owner()->get_node_or_null(_agents[keys[static_cast<int>(i)]]);
		} else {
			a = get_node_or_null(_agents[keys[static_cast<int>(i)]]);
		}
		if (a) {
			_agents[keys[static_cast<int>(i)]] = a;
		} else {
			_agents.erase(keys[static_cast<int>(i)]);
		}
	}

	// 为表达式转换提供必要参数
	_expression_objs_names.resize(HfsmGlobal::name2singleton.size() + _agents.size());
	_expression_objs.resize(_expression_objs_names.size());
	int idx = 0;
	auto arr = HfsmGlobal::name2singleton.get_array();
	for (auto i = 0; i < HfsmGlobal::name2singleton.size(); i++) {
		_expression_objs_names.set(idx, arr[i].key);
		_expression_objs[idx] = arr[i].value;
		idx++;
	}
	auto agent_names = _agents.keys();
	for (auto i = 0; i < agent_names.size(); i++) {
		_expression_objs_names.set(idx, _agents[agent_names[i]]);
		_expression_objs[idx] = _agents[agent_names[i]];
		idx++;
	}

	// 生成hfsm
	generate_hfsm();
	// yield(get_tree(),"idle_frame")
	if (is_active()) {
		restart();
	}
	_inited = true;
	emit_signal(sn);
}

void HFSM::_ready() {
	if (!_root_fsm_res.is_valid()) {
		_root_fsm_res.instantiate();
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
	_trigger_list.clear();
	// 生成根状态机
	Vector<Hfsm::Fsm *> tmp;
	_root_fsm = _root_fsm_res->create_fsm(this, Ref<State>(), tmp);
	// 生成变量列表
	for (auto i = 0; i < _root_fsm_res->_variable_res_list.size(); i++) {
		Ref<HFSMVariableRes> vr = _root_fsm_res->_variable_res_list[i];
		_variable_blackboard.insert(vr->get_variable_name(), vr->create_variable());
	}
	//
	set_debug(_debug);
	//
	if (!Engine::get_singleton()->is_editor_hint()) {
		_root_fsm_res.unref();
	}
}

void HFSM::manual_update() {
	ERR_FAIL_COND(is_active());
	ERR_FAIL_COND(_update_type == UPDATE_TYPE_MANUAL);
	_process(get_process_delta_time());
}

void HFSM::manual_physics_update() {
	ERR_FAIL_COND(is_active());
	ERR_FAIL_COND(_update_type == UPDATE_TYPE_MANUAL);
	_process(get_process_delta_time());
}

void HFSM::restart() {
	_root_fsm->reset();
	set_active(true);
	set_update_type(_update_type);
	_root_fsm->entry();
}

Ref<HFSMVariable> HFSM::get_var(const StringName &variable_name) {
	ERR_FAIL_COND_V(!_variable_blackboard.has(variable_name), Ref<HFSMVariable>());
	return _variable_blackboard[variable_name];
}

Array HFSM::get_vars() {
	Array ret;
	ret.resize(_variable_blackboard.size());
	auto arr = _variable_blackboard.get_array();
	for (auto i = 0; i < ret.size(); i++) {
		ret[i] = arr[i].value;
	}
	return ret;
}

Variant HFSM::get_var_value(const StringName &variable_name) { return get_var(variable_name)->get_value(); }
Dictionary HFSM::get_vars_value() {
	Dictionary r;
	auto arr = _variable_blackboard.get_array();
	for (size_t i = 0; i < _variable_blackboard.size(); i++) {
		r[arr[i].key] = arr[i].value->get_value();
	}
	return r;
}

void HFSM::set_var(const StringName &variable_name, const Variant &value) {
	ERR_FAIL_COND(!_variable_blackboard.has(variable_name));
	_variable_blackboard[variable_name]->set_value(value);
}

void HFSM::set_trigger(const StringName &trigger_name) { _variable_blackboard[trigger_name]->set_value(true); }
void HFSM::set_boolean(const StringName &boolean_name, bool value) { _variable_blackboard[boolean_name]->set_value(value); }
void HFSM::set_integer(const StringName &interger_name, int64_t value) { _variable_blackboard[interger_name]->set_value(value); }
void HFSM::set_float(const StringName &float_name, double value) { _variable_blackboard[float_name]->set_value(value); }
void HFSM::set_string(const StringName &string_name, const String &value) { _variable_blackboard[string_name]->set_value(value); }

void HFSM::set_update_type(UpdateType update_type) {
	_update_type = UpdateType(update_type);
#ifndef DEBUG_IN_EDITOR
	if (Engine::get_singleton()->is_editor_hint()) {
		return;
	}
#endif
	switch (_update_type) {
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

void HFSM::_process(double delta) {
	if (is_active() && is_inited()) {
		switch (_update_type) {
			case UPDATE_TYPE_PHYSICS:
			case UPDATE_TYPE_IDLE_AND_PHYSICS: {
				if (_root_fsm->_running) {
					_root_fsm->check_transit_and_get_update_queue(_active_fsm_list);
				}
				flush_trigger();
			} break;
			default:
				break;
		}

		for (auto &&fsm : *_active_fsm_list) {
			fsm->update(delta);
			if (_update_type == UPDATE_TYPE_IDLE) {
				fsm->physics_update(delta);
			}
		}
	}
}

void HFSM::_physics_process(double delta) {
	if (is_active() && is_inited()) {
		// TODO:: 两次检测？
		if (_root_fsm->_running) {
			_root_fsm->check_transit_and_get_update_queue(_active_fsm_list);
		}
		flush_trigger();

		for (auto &&fsm : *_active_fsm_list) {
			fsm->physics_update(delta);
			if (_update_type == UPDATE_TYPE_IDLE) {
				fsm->update(delta);
			}
		}
	}
}

void HFSM::flush_trigger() {
	for (auto &&t : _trigger_list) {
		t->flush_trigger();
	}
}

// 信号发射器 , 由 fsm 调用
void HFSM::updated(Ref<State> &state, double delta) {
	static const StringName sn = "updated";
	switch (_update_type) {
		case UPDATE_TYPE_IDLE_AND_PHYSICS:
		case UPDATE_TYPE_IDLE:
			emit_signal(sn, state, delta);
			break;
		default:
			break;
	}
}

void HFSM::physic_updated(Ref<State> &state, double delta) {
	static const StringName sn = "physic_updated";
	switch (_update_type) {
		case UPDATE_TYPE_IDLE_AND_PHYSICS:
		case UPDATE_TYPE_PHYSICS:
			emit_signal(sn, state, delta);
			break;
		default:
			break;
	}
}

void HFSM::transited(Ref<State> &from_state, Ref<State> &to_state) {
	static const StringName sn = "transited";
	_previous_state = from_state;
	_current_state = to_state;
	if (_current_state.is_valid() && animation_player) {
		auto anim = _current_state->get_animation_name_for_playing();
		if (animation_player->has_animation(anim)) {
#ifdef FULL_VERSION
			animation_player->play(anim, _current_state->get_animation_blend_time(), _current_state->get_animation_speed(), _current_state->get_animation_reverse());
#else
			animation_player->play(anim);
#endif
			_current_state->_animation_playing = true;
		} else {
			_current_state->_animation_playing = false;
		}
	} else {
		_current_state->_animation_playing = false;
	}
	if (_current_state.is_valid()) {
		// 无论如何，只有最新的状态能播放状态。
		_previous_state->_animation_playing = false;
	}
	emit_signal(sn, from_state, to_state);
}

void HFSM::entered(Ref<State> &state) {
	static const StringName sn = "entered";
	emit_signal(sn, state);
}
void HFSM::exited(Ref<State> &state) {
	static const StringName sn = "exited";
	emit_signal(sn, state);
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
	static const StringName sn = { "animation_finished" };
	static const StringName mn = { "__on_animation_finished" };
	if (animation_player && animation_player->is_connected(sn, Callable(this, mn))) {
		animation_player->disconnect(sn, Callable(this, mn));
	}
	animation_player = p_animtion_player;
	if (animation_player && !animation_player->is_connected(sn, Callable(this, mn))) {
		animation_player->connect(sn, Callable(this, mn));
	}
}

}; // namespace Hfsm
