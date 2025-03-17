/**************************************************************************/
/*  transition_base.h                                                     */
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
#include <godot_cpp/classes/ref_counted.hpp>

using namespace godot;
#else // GDEXTENSION_BUILD
#include <core/object/ref_counted.h>

#endif // GDEXTENSION_BUILD

namespace HFSM2 {
class HFSM;
class State;

// 基类
class TransitionBase {
public:
	virtual void refresh() {}
	virtual bool can_transit() { return false; }

	virtual Ref<State> get_from_state();
	virtual Ref<State> get_to_state();

	// void set_from_state(Ref<RefCounted> &from);
	// void set_to_state(Ref<RefCounted> &to);

#ifdef ROLLBACK_NET_CODE
	virtual Variant _save_state();
	virtual void _load_state(const Variant &state);
	virtual void _interpolate_state(const Variant &old_state,
			const Variant &new_state, real_t weight);
	virtual Variant _get_local_input();
	virtual Variant _predict_remote_input(const Variant &previous_input,
			int64_t ticks_since_real_input);
	virtual void _network_process(Variant &input);
	virtual void _network_preprocess(Variant &input);
	virtual void _network_postprocess(Variant &input);
	virtual Dictionary &_network_spawn_preprocess(Dictionary &data);
	virtual void _network_spawn(Dictionary &data);
	virtual void _network_despawn();
#endif

private:
	Ref<State> from_state;
	Ref<State> to_state;

	friend class FSMConfig;
	friend class State;
};

#pragma region 内联实现
inline Ref<State> TransitionBase::get_from_state() { return from_state; }
inline Ref<State> TransitionBase::get_to_state() { return to_state; }

#pragma endregion

}; // namespace HFSM2
