#pragma once

#include "variable_config.h"
#include "state.h"
#include "state_config.h"
#include "transition_config.h"

#ifdef GDEXTENSION_BUILD
using namespace godot;
#else

#endif // GDEXTENSION_BUILD
namespace Hfsm {
// class HFSM;
// class Fsm;
// class VariableConfig;

// 状态机资源
class FSMConfig : public Resource {
	GDCLASS(FSMConfig, Resource)

protected:
	static void _bind_methods();

	_TO_STRING()

public:
	Fsm *create_fsm(class HFSM *p_hfsm, const Ref<State> &p_nested_state, const Vector<class Fsm *> &p_nested_fsm_update_queue);

	void set_nested_state_config(const Ref<StateConfig> &p_state_config);
	Ref<StateConfig> get_nested_state_config() const;

	TypedArray<StateConfig> get_state_config_list() const { return state_config_list; }
	// 转换列表
	TypedArray<TransitionConfig> get_transition_config_list() const { return transition_config_list; }
	// 变量列表
	TypedArray<VariableConfig> get_variable_config_list() const { return variable_config_list; }

	void add_state_config(const Ref<StateConfig> &p_state_config);

	void add_transition_config(const Ref<TransitionConfig> &p_transition_config);

	void add_variable_config(const Ref<VariableConfig> &p_variable_config);

	// 未删除相关的 TransitionConfig, 需要在编辑器里处理 undoredo
	void remove_state_config(const Ref<StateConfig> &p_state_config);

	void remove_transition_config(const Ref<TransitionConfig> &p_transition_config);

	void remove_variable_config(const Ref<VariableConfig> &p_variable_config);

	// void set_state_config_list(Array state_config_list){
	//     _state_config_list = Vector<Ref<StateConfig>>(state_config_list);
	// }

	// 编辑器方法
	// void set_unique_entry_state(Ref<StateConfig> &state_config);
	// Ref<StateConfig> get_exist_entry_config();
	// void add_state_config(Ref<StateConfig> &new_state_config);
	// void deleted_state_config(Ref<StateConfig> &deleted_state_config);
	// void make_state_name_unique(Ref<StateConfig> &state_config);
	// void add_transition_config(Ref<TransitionConfig> &new_transition_config);
	// void delete_transition_config(Ref<TransitionConfig> &deleted_transition_config);

	// VMap<Ref<Script>, Ref<StateConfig>> get_state_script_to_state();
	// Vector<Ref<StateConfig>> get_all_nested_state_config();

	void set_variable_config_list(const Array &p_variable_config_list);

private:
	// 自己的状态列表
	void set_state_config_list(const Array &p_state_config_list);
	void set_transition_config_list(const Array &p_transition_config_list);

	// 所在的状态
	Ref<StateConfig> nested_state_config;

	// 自己的状态列表
	TypedArray<StateConfig> state_config_list;
	// 变量列表
	TypedArray<VariableConfig> variable_config_list;
	// 转换列表
	TypedArray<TransitionConfig> transition_config_list;
};

}; // namespace Hfsm
