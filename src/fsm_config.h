/**************************************************************************/
/*  fsm_config.h                                                          */
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

#pragma once

#include "state.h"
#include "state_config.h"
#include "transition_config.h"

#ifdef GDEXTENSION_BUILD
using namespace godot;
#else

#endif // GDEXTENSION_BUILD
namespace HFSM2 {
// class HFSM;
// class FSM;
class VariableConfig;

// 状态机资源
class FSMConfig : public Resource {
	GDCLASS(FSMConfig, Resource)

protected:
	static void _bind_methods();

	_TO_STRING()

public:
	FSM *create_fsm(class HFSM *p_hfsm);

	void set_nested_state_config(const Ref<StateConfig> &p_state_config);
	Ref<StateConfig> get_nested_state_config() const;

	TypedArray<StateConfig> get_state_config_list() const { return state_config_list; }
	// 转换列表
	TypedArray<TransitionConfig> get_transition_config_list() const { return transition_config_list; }
	// 变量列表
	TypedArray<class VariableConfig> get_variable_config_list() const { return variable_config_list; }

	void add_state_config(const Ref<StateConfig> &p_state_config);

	void add_transition_config(const Ref<TransitionConfig> &p_transition_config);

	void add_variable_config(const Ref<class VariableConfig> &p_variable_config);

	// 未删除相关的 TransitionConfig, 需要在编辑器里处理 Undo Redo
	void remove_state_config(const Ref<StateConfig> &p_state_config);

	void remove_transition_config(const Ref<TransitionConfig> &p_transition_config);

	void remove_variable_config(const Ref<class VariableConfig> &p_variable_config);

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

#if TOOLS_ENABLED
	Array debug_serialize(const Ref<FSMConfig> &p_root = {}) const;
	static Ref<FSMConfig> debug_deserialize(const Array &p_data, const Ref<FSMConfig> &p_root = {});
#endif // TOOLS_ENABLED

private:
	// 自己的状态列表
	void set_state_config_list(const Array &p_state_config_list);
	void set_transition_config_list(const Array &p_transition_config_list);

	// 所在的状态
	Ref<WeakRef> nested_state_config;

	// 自己的状态列表
	TypedArray<StateConfig> state_config_list;
	// 变量列表
	TypedArray<class VariableConfig> variable_config_list;
	// 转换列表
	TypedArray<TransitionConfig> transition_config_list;
};

}; // namespace HFSM2
