#pragma once

#include <godot_cpp/classes/global_constants.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/templates/vector.hpp>

#include <godot_cpp/core/binder_common.hpp>
#include <godot_cpp/templates/vmap.hpp>

#include <hfsm_global.hpp>
// #include "fsm_res.hpp"

using namespace godot;
namespace Hfsm {
class State;
class HFSM;
class FsmRes;

// 考虑状态机是非运行时设计完成的一种东西，在运行时改变状态是不合理的设计
// 因此取消运行时改变状态类型的功能
// TODO:: 添加各种 setget 以及绑定
class Fsm {
	friend class State;
	friend class HFSM;
	friend class FsmRes;

public:
	const Array &get_path() const;
	Ref<State> get_current_state() const;

	Vector<Fsm *> &get_fsm_update_queue() { return fsm_update_queue; }

	bool is_running();

#ifdef ROLLBACK_NET_CODE
	Array _save_state();
	void _load_state(const Array &state);
	void _interpolate_state(const Array &old_state, const Array &new_state,
			real_t weight);
	Array _get_local_input();
	Array _predict_remote_input(const Array &previous_input,
			int64_t ticks_since_real_input);
	void _network_process(Array &input);
	void _network_preprocess(Array &input);
	void _network_postprocess(Array &input);
	Dictionary &_network_spawn_preprocess(Dictionary &data);
	void _network_spawn(Dictionary &data);
	void _network_despawn();
#endif

private:
	HFSM *hfsm = nullptr;
	// PackedStringArray path = PackedsStringArray();
	Vector<Ref<State>> state_list;
	Array path;
	Vector<Fsm *> fsm_update_queue;
	bool running = false;
	bool reset_when_entry = false;

	Ref<State> nested_state;
	// void set_nested_state(const Ref<State> &state);

	Ref<State> current_state;
	Ref<State> current_entry_state;
	// Ref<State> _default_entry_state;
	Vector<Ref<State>> current_exit_state_list = Vector<Ref<State>>();
	// Vector<Ref<State>> _default_exit_state_list = Vector<Ref<State>>();

	// 考虑摒弃资源类
	// void init(hfsm:Node,nested_fsm_res = null,parent_path:Array
	// =[],nested_state = null):
	void reset();
	void entry(const Ref<State> &p_entry_state = Ref<State>());
	void check_transit_and_get_update_queue(Vector<Fsm *> *p_update_queue);
	void update(double p_delta);
	void physics_update(double p_delta);
	bool force_transit(const StringName &p_target_state);
	bool force_transit_state(Ref<State> &p_target_state);
	// void exit();
	void exit_by_state();
	Ref<State> get_state(const StringName &p_state_name);

	// void set_entry_state(String state_name);
	// void set_exit_state(String state_name);
	// void set_normal_state(String state_name);
	// void set_unique_exit_state(String state_name);
	friend class StateRes;
	friend class FsmRes;
};

#pragma region 内联实现

inline const Array &Fsm::get_path() const { return path; }
inline Ref<State> Fsm::get_current_state() const { return current_state; }
inline bool Fsm::is_running() { return running; }

#pragma endregion

}; // namespace Hfsm
