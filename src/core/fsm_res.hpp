#pragma once

#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/templates/vector.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "hfsm_variable_res.hpp"
#include "state.hpp"
#include "state_res.hpp"
#include "transition_res.hpp"

using namespace godot;

namespace Hfsm {
class HFSM;
class Fsm;
class HFSMVariableRes;

// 状态机资源
class FsmRes : public Resource {
	GDCLASS(FsmRes, Resource)

protected:
	static void _bind_methods();

	String _to_string() const { return vformat("[FsmRes:%d]", get_instance_id()); }

public:
	Fsm *create_fsm(HFSM *p_hfsm, const Ref<State> &p_nested_state, const Vector<Hfsm::Fsm *> &p_nested_fsm_update_queue);

	void set_nested_state_res(const Ref<StateRes> &p_state_res);
	Ref<StateRes> get_nested_state_res() const;

	TypedArray<StateRes> get_state_res_list() const { return state_res_list; }
	// 转换列表
	TypedArray<TransitionRes> get_transition_res_list() const { return transition_res_list; }
	// 变量列表
	TypedArray<HFSMVariableRes> get_variable_res_list() const { return variable_res_list; }

	void add_state_res(const Ref<StateRes> &p_state_res);

	void add_transition_res(const Ref<TransitionRes> &p_transition_res);

	void add_variable_res(const Ref<HFSMVariableRes> &p_variable_res) {
		if (variable_res_list.find(p_variable_res) >= 0) {
			return;
		}
		variable_res_list.push_back(p_variable_res);
	}

	// 未删除相关的 TransitionRes, 需要在编辑器里处理 undoredo
	void remove_state_res(const Ref<StateRes> &p_state_res) {
		state_res_list.erase(p_state_res);
		emit_changed();
	}

	void remove_transition_res(const Ref<TransitionRes> &p_transition_res);

	void remove_variable_res(const Ref<HFSMVariableRes> &p_variable_res) { variable_res_list.erase(p_variable_res); }

	// void set_state_res_list(Array state_res_list){
	//     _state_res_list = Vector<Ref<StateRes>>(state_res_list);
	// }

	// 编辑器方法
	// void set_unique_entry_state(Ref<StateRes> &state_res);
	// Ref<StateRes> get_exist_entry_res();
	// void add_state_res(Ref<StateRes> &new_state_res);
	// void deleted_state_res(Ref<StateRes> &deleted_state_res);
	// void make_state_name_unique(Ref<StateRes> &state_res);
	// void add_transition_res(Ref<TransitionRes> &new_transition_res);
	// void delete_transition_res(Ref<TransitionRes> &deleted_transition_res);

	// TODO:: 这是啥？？？
	// Ref<FsmRes> duplicate_self();

	// VMap<Ref<Script>, Ref<StateRes>> get_state_script_to_state();
	// bool is_deleted_state_script();
	// Vector<Ref<StateRes>> get_all_nested_state_res();

private:
	// 自己的状态列表
	void set_state_res_list(const Array &p_state_res_list) { state_res_list = decltype(state_res_list)(p_state_res_list); }
	void set_transition_res_list(const Array &p_transition_res_list) { transition_res_list = decltype(transition_res_list)(p_transition_res_list); }
	void set_variable_res_list(const Array &p_variable_res_list);

	// 所在的状态
	Ref<StateRes> nested_state_res;

	// 自己的状态列表
	TypedArray<StateRes> state_res_list;
	// 变量列表
	TypedArray<HFSMVariableRes> variable_res_list;
	// 转换列表
	TypedArray<TransitionRes> transition_res_list;

	friend class HFSM;
	friend class HFSMVariableRes;
};

}; // namespace Hfsm
