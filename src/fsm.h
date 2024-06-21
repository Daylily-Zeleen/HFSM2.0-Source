/**************************************************************************/
/*  fsm.h                                                                 */
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

#ifdef GDEXTENSION_BUILD
#include <godot_cpp/templates/vector.hpp>
#include <godot_cpp/variant/typed_array.hpp>
using namespace godot;
#else
#include <core/variant/typed_array.h>

#endif // GDEXTENSION_BUILD

namespace HFSM2 {

class HFSM;
// class FSMConfig;

// 考虑状态机是非运行时设计完成的一种东西，在运行时改变状态是不合理的设计
// 因此取消运行时改变状态类型的功能
class FSM {
	friend class FSMConfig;

public:
	const TypedArray<State> &get_path() const;
	Ref<State> get_current_state() const;
	LocalVector<FSM *> &get_fsm_update_queue() { return fsm_update_queue; }

	bool is_running() const;

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

public:
	~FSM();

private:
	HFSM *hfsm = nullptr;
	// PackedStringArray path = PackedsStringArray();
	Vector<Ref<State>> state_list;
	TypedArray<State> path;
	LocalVector<FSM *> fsm_update_queue;
	bool running = false;
	// bool reset_when_entry = false;

	// Ref<State> nested_state;
	// void set_nested_state(const Ref<State> &state);

	Ref<State> current_state;
	Ref<State> current_entry_state;
	// Ref<State> _default_entry_state;
	Vector<Ref<State>> current_exit_state_list = Vector<Ref<State>>();
	// Vector<Ref<State>> _default_exit_state_list = Vector<Ref<State>>();

	// bool force_transit(const StringName &p_target_state);
	// bool force_transit_state(Ref<State> &p_target_state);
	// Ref<State> get_state(const StringName &p_state_name);

	// void set_entry_state(String state_name);
	// void set_exit_state(String state_name);
	// void set_normal_state(String state_name);
	// void set_unique_exit_state(String state_name);

	void set_nested_state(const Ref<State> &p_nested_state, const LocalVector<FSM *> &p_nested_fsm_update_queue);
	friend void State::initialize_state(const StringName &p_name, const StringName &p_anim_name, HFSM *p_hfsm, StateType p_type, const TypedArray<HFSM2::State> &p_path, FSM *p_sub_fsm, const LocalVector<FSM *> &p_nested_fsm_update_queue);

public:
	LocalVector<FSM *> *try_transit_and_get_update_queue();

	// void reset();
	void update(double p_delta);
	void physics_update(double p_delta);
	void entry(); //(const Ref<State> &p_entry_state = Ref<State>());
	void exit_by_state();
};

#pragma region 内联实现

inline const TypedArray<State> &FSM::get_path() const { return path; }
inline Ref<State> FSM::get_current_state() const { return current_state; }
inline bool FSM::is_running() const { return running; }

#pragma endregion

}; // namespace HFSM2
