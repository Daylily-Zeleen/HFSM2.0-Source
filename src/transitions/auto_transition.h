#pragma once

#include "../fsm.h"
#include "../state.h"
#include "transition_base.h"

#ifdef GDEXTENSION_BUILD
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/time.hpp>

#else
#include <core/config/engine.h>
#include <core/os/time.h>

#endif // GDEXTENSION_BUILD

namespace Hfsm {

// 自动转换
class AutoTransition : public TransitionBase {
public:
	void refresh() override;
	bool can_transit() override;

#ifdef ROLLBACK_NET_CODE
	Variant _save_state() override;

	void _load_state(const Variant &p_state) override;
#endif
private:
	TransitionConfig::AuotoTtransitMode mode = TransitionConfig::AUTO_TRANSIT_MODE_ANIMATION_FINISH;
	uint64_t delay_msec = 1000;
	uint64_t times = 5;

	uint64_t next_delay_transit_tick = 0;
	uint64_t update_times = 0;

	friend class TransitionConfig;
};

#pragma region 内联实现

inline void AutoTransition::refresh() {
	switch (mode) {
		case TransitionConfig::AUTO_TRANSIT_MODE_DELAY_TIMER: {
			next_delay_transit_tick = Time::get_singleton()->get_ticks_msec() + delay_msec;
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
			return Time::get_singleton()->get_ticks_msec() > next_delay_transit_tick;
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

} // namespace Hfsm
