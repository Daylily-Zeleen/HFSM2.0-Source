/**************************************************************************/
/*  auto_transition.h                                                     */
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

#include "../fsm.h"
#include "../state.h"
#include "../transition_config.h"
#include "transition_base.h"

#ifdef GDEXTENSION_BUILD
#include <chrono>
#include <godot_cpp/classes/engine.hpp>

#else
#include <core/config/engine.h>
#include <core/os/time.h>

#endif // GDEXTENSION_BUILD

namespace HFSM2 {

// 自动转换
class AutoTransition : public TransitionBase {
	inline static uint64_t get_ticks_msec() {
#ifdef GDEXTENSION_BUILD
		using namespace std::chrono;
		return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
#else
		return Time::get_singleton()->get_ticks_msec();
#endif // GDEXTENSION_BUILD
	}

public:
	void refresh() override;
	bool can_transit() override;

#ifdef ROLLBACK_NET_CODE
	Variant _save_state() override;

	void _load_state(const Variant &p_state) override;
#endif
	AutoTransition(TransitionConfig::AuotoTtransitMode p_mode, uint64_t p_delay_msec, uint64_t p_times) :
			mode(p_mode), delay_msec(p_delay_msec), times(p_times) {}

private:
	TransitionConfig::AuotoTtransitMode mode = TransitionConfig::AUTO_TRANSIT_MODE_ANIMATION_FINISH;
	uint64_t delay_msec = 1000;
	uint64_t times = 5;

	uint64_t next_delay_transit_tick = 0;
	uint64_t update_times = 0;
};

#pragma region 内联实现

inline void AutoTransition::refresh() {
	switch (mode) {
		case TransitionConfig::AUTO_TRANSIT_MODE_DELAY_TIMER: {
			next_delay_transit_tick = get_ticks_msec() + delay_msec;
		} break;
		case TransitionConfig::AUTO_TRANSIT_MODE_UPDATE_TIMES:
		case TransitionConfig::AUTO_TRANSIT_MODE_PHYSICS_UPDATE_TIMES: {
			update_times = times;
		} break;
		default: {
		} break;
	}
}
inline bool AutoTransition::can_transit() {
	switch (mode) {
		case TransitionConfig::AUTO_TRANSIT_MODE_ANIMATION_FINISH: {
			return !get_from_state()->is_animation_playing();
		} break;
		case TransitionConfig::AUTO_TRANSIT_MODE_DELAY_TIMER: {
			return get_ticks_msec() > next_delay_transit_tick;
		} break;
		case TransitionConfig::AUTO_TRANSIT_MODE_FSM_EXIT: {
			auto fsm = get_from_state()->get_sub_fsm();
			if (fsm && !fsm->is_running()) {
				return true;
			}
			return false;
		} break;
		case TransitionConfig::AUTO_TRANSIT_MODE_MANUAL: {
			return get_from_state()->is_exited();
		} break;
		case TransitionConfig::AUTO_TRANSIT_MODE_UPDATE_TIMES: {
			if (!Engine::get_singleton()->is_in_physics_frame()) {
				update_times--;
			}
			return update_times == 0;
		} break;
		case TransitionConfig::AUTO_TRANSIT_MODE_PHYSICS_UPDATE_TIMES: {
			if (Engine::get_singleton()->is_in_physics_frame()) {
				update_times--;
			}
			return update_times == 0;
		} break;
		default: {
			return false;
		} break;
	}
}

#pragma endregion

} // namespace HFSM2
