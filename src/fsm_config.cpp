#include "fsm_config.h"

#include "fsm.h"
#include "state_config.h"
#include "transition_config.h"
#include "transitions/transition_base.h"

namespace Hfsm {

#pragma region FSMConfig

void FSMConfig::_bind_methods() {
	GDBIND_BEGIN(FSMConfig);
	GDADD_PROPERTY_RESOURCE(nested_state_config, PROPERTY_USAGE_NONE);

	GDADD_PROPERTY_TYPED_ARRAY(state_config_list, StateConfig);
	GDADD_PROPERTY_TYPED_ARRAY(transition_config_list, TransitionConfig);
	GDADD_PROPERTY_TYPED_ARRAY(variable_config_list, VariableConfig, PROPERTY_USAGE_STORAGE);

	GDBIND_METHOD(add_state_config, "new_state_config");
	GDBIND_METHOD(add_transition_config, "new_transition_config");
	GDBIND_METHOD(add_variable_config, "new_variable_config");

	GDBIND_METHOD(remove_state_config, "to_remove_state_config");
	GDBIND_METHOD(remove_transition_config, "to_remove_transition_config");
	GDBIND_METHOD(remove_variable_config, "remove_variable_config");
}

void FSMConfig::set_nested_state_config(const Ref<StateConfig> &p_state_config) {
	nested_state_config = p_state_config;
	emit_changed();
}
Ref<StateConfig> FSMConfig::get_nested_state_config() const { return nested_state_config; }

void FSMConfig::add_state_config(const Ref<StateConfig> &p_state_config) {
	if (state_config_list.find(p_state_config) >= 0) {
		return;
	}
	bool ununique = false;
	do {
		ununique = false;
		for (auto i = 0; i < state_config_list.size(); i++) {
			Ref<StateConfig> sc = state_config_list[i];
			StringName state_name = p_state_config->get_state_name();
			StringName be_check = sc->get_state_name();
			if (state_name == be_check) {
				state_name = String("@") + String(state_name);
				p_state_config->set_state_name(state_name);
				ununique = true;
				break;
			}
		}
	} while (ununique);

	state_config_list.push_back(p_state_config);

	if (p_state_config.is_valid() && !p_state_config->is_connected(s_changed, cb_resource_emit_changed(this))) {
		p_state_config->connect(s_changed, cb_resource_emit_changed(this));
	}
	emit_changed();
}

void FSMConfig::add_transition_config(const Ref<TransitionConfig> &p_transition_config) {
	if (transition_config_list.find(p_transition_config) >= 0) {
		return;
	}
	Ref<StateConfig> add_from_state = p_transition_config->get_from_state_config();
	Ref<StateConfig> add_to_state = p_transition_config->get_to_state_config();
	for (size_t i = 0; i < transition_config_list.size(); i++) {
		Ref<TransitionConfig> tc = transition_config_list[i];
		Ref<StateConfig> existed_from_config = tc->get_from_state_config();
		Ref<StateConfig> existed_to_config = tc->get_to_state_config();
		ERR_FAIL_COND_MSG(add_from_state == add_to_state && existed_from_config == existed_to_config, "不应发生:存在相同的转换");
	}

	transition_config_list.push_back(p_transition_config);
	if (p_transition_config.is_valid() && !p_transition_config->is_connected(s_changed, cb_resource_emit_changed(this))) {
		p_transition_config->connect(s_changed, cb_resource_emit_changed(this));
	}
	emit_changed();
}

void FSMConfig::add_variable_config(const Ref<VariableConfig> &p_variable_config) {
	if (variable_config_list.find(p_variable_config) >= 0) {
		return;
	}
	variable_config_list.push_back(p_variable_config);

	if (p_variable_config.is_valid() && !p_variable_config->is_connected(s_changed, cb_resource_emit_changed(this))) {
		p_variable_config->connect(s_changed, cb_resource_emit_changed(this));
	}

	emit_changed();
}

// 未删除相关的 TransitionConfig, 需要在编辑器里处理 undoredo
void FSMConfig::remove_state_config(const Ref<StateConfig> &p_state_config) {
	state_config_list.erase(p_state_config);

	if (p_state_config.is_valid() && p_state_config->is_connected(s_changed, cb_resource_emit_changed(this))) {
		p_state_config->disconnect(s_changed, cb_resource_emit_changed(this));
	}

	emit_changed();
}

void FSMConfig::remove_variable_config(const Ref<VariableConfig> &p_variable_config) {
	variable_config_list.erase(p_variable_config);

	if (p_variable_config.is_valid() && p_variable_config->is_connected(s_changed, cb_resource_emit_changed(this))) {
		p_variable_config->disconnect(s_changed, cb_resource_emit_changed(this));
	}

	emit_changed();
}

void FSMConfig::remove_transition_config(const Ref<TransitionConfig> &p_transition_config) {
	auto cb = cb_resource_emit_changed(this);
	if (transition_config_list.find(p_transition_config) >= 0) {
		transition_config_list.erase(p_transition_config);
		if (p_transition_config.is_valid() && p_transition_config->is_connected(s_changed, cb)) {
			p_transition_config->disconnect(s_changed, cb);
		}
		emit_changed();
	} else {
		Ref<StateConfig> add_from_state = p_transition_config->get_from_state_config();
		Ref<StateConfig> add_to_state = p_transition_config->get_to_state_config();
		for (size_t i = 0; i < transition_config_list.size(); i++) {
			Ref<TransitionConfig> tc = transition_config_list[i];
			Ref<StateConfig> existed_from_config = tc->get_from_state_config();
			Ref<StateConfig> existed_to_config = tc->get_to_state_config();

			if (add_from_state == add_to_state && existed_from_config == existed_to_config) {
				Ref<TransitionConfig> tc = transition_config_list[i];
				transition_config_list.erase(tc);
				if (tc.is_valid() && tc->is_connected(s_changed, cb)) {
					tc->disconnect(s_changed, cb);
				}

				emit_changed();
				ERR_FAIL_MSG("不应发生: 不存在要移除的转换，但存在相同的连接方式，已将其移除。");
			}
		}
	}
}

FSM *FSMConfig::create_fsm(HFSM *p_hfsm) {
	FSM *ret = memnew(FSM);
	ret->hfsm = p_hfsm;

	ret->fsm_update_queue.push_back(ret);

	// 构造状态列表
	auto state_config2state = VMap<Ref<StateConfig>, Ref<State>>();
	for (size_t i = 0; i < state_config_list.size(); i++) {
		Ref<StateConfig> state_config = state_config_list[i];

		auto state = state_config->create_state(p_hfsm, ret);
		state_config2state.insert(state_config, state);

		ret->state_list.push_back(state);
	}

	// 构造转换列表
	for (size_t i = 0; i < transition_config_list.size(); i++) {
		Ref<TransitionConfig> transition_config = transition_config_list[i];
		Ref<StateConfig> from_config = transition_config->get_from_state_config();
		Ref<StateConfig> to_config = transition_config->get_to_state_config();

		TransitionBase *transition = transition_config->create_transition(p_hfsm);
		//  添加到起始状态的转换列表中
		auto from_state = state_config2state[from_config];
		from_state->_add_transition(transition);
		// 设置转换的起始与目标状态
		transition->from_state = from_state;
		transition->to_state = state_config2state[to_config];
	}

	// 整理起始与结束状态
	for (auto &&state : ret->state_list) {
		if (state->get_state_type() == State::STATE_TYPE_ENTRY) {
			ret->current_entry_state = state;
		} else if (state->get_state_type() == State::STATE_TYPE_EXIT) {
			ret->current_exit_state_list.append(state);
		}
	}

	return ret;
}

void FSMConfig::set_variable_config_list(const Array &p_variable_config_list) {
	auto cb = cb_resource_emit_changed(this);
	for (auto i = 0; i < variable_config_list.size(); ++i) {
		Ref<VariableConfig> vc = variable_config_list[i];
		if (vc.is_valid() && vc->is_connected(s_changed, cb)) {
			vc->disconnect(s_changed, cb);
		}
	}

	variable_config_list = decltype(variable_config_list)(p_variable_config_list);
	for (size_t i = 0; i < variable_config_list.size(); i++) {
		if (auto vc = Object::cast_to<VariableConfig>(variable_config_list[i])) {
			if (vc->get_fsm_config() != this) {
				vc->set_fsm_config(this);
			}
		} else {
			variable_config_list[i] = VariableConfig::create_new(this);
		}
	}

	for (auto i = 0; i < variable_config_list.size(); ++i) {
		Ref<VariableConfig> vc = variable_config_list[i];
		if (vc.is_valid() && !vc->is_connected(s_changed, cb)) {
			vc->connect(s_changed, cb);
		}
	}

	emit_changed();
}

void FSMConfig::set_state_config_list(const Array &p_state_config_list) {
	auto cb = cb_resource_emit_changed(this);
	for (auto i = 0; i < state_config_list.size(); ++i) {
		Ref<StateConfig> sc = state_config_list[i];
		if (sc.is_valid() && sc->is_connected(s_changed, cb)) {
			sc->disconnect(s_changed, cb);
		}
	}
	state_config_list = decltype(state_config_list)(p_state_config_list);
	for (auto i = 0; i < state_config_list.size(); ++i) {
		Ref<StateConfig> sc = state_config_list[i];
		if (sc.is_valid() && !sc->is_connected(s_changed, cb)) {
			sc->connect(s_changed, cb);
		}
	}

	emit_changed();
}

void FSMConfig::set_transition_config_list(const Array &p_transition_config_list) {
	auto cb = cb_resource_emit_changed(this);
	for (auto i = 0; i < transition_config_list.size(); ++i) {
		Ref<TransitionConfig> tc = transition_config_list[i];
		if (tc.is_valid() && tc->is_connected(s_changed, cb)) {
			tc->disconnect(s_changed, cb);
		}
	}
	transition_config_list = decltype(transition_config_list)(p_transition_config_list);
	for (auto i = 0; i < transition_config_list.size(); ++i) {
		Ref<TransitionConfig> tc = transition_config_list[i];
		if (tc.is_valid() && !tc->is_connected(s_changed, cb)) {
			tc->connect(s_changed, cb);
		}
	}

	emit_changed();
}

#pragma endregion

} // namespace Hfsm
