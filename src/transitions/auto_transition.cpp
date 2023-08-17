
#include "auto_transition.h"

namespace Hfsm {

#pragma region AutoTransition

#ifdef ROLLBACK_NET_CODE
Variant AutoTransition::_save_state() {
	switch (_mode) {
		case AUTO_TRANSIT_MODE_DELAY_TIMER:
			return next_delay_transit_tick;
		case AUTO_TRANSIT_MODE_UPDATE_TIMES:
		case AUTO_TRANSIT_MODE_PHYSICS_UPDATE_TIMES:
			return update_times;
		default:
			return Variant();
	}
}

void AutoTransition::_load_state(const Variant &state) {
	switch (_mode) {
		case AUTO_TRANSIT_MODE_DELAY_TIMER:
			next_delay_transit_tick = state;
			return;
		case AUTO_TRANSIT_MODE_UPDATE_TIMES:
		case AUTO_TRANSIT_MODE_PHYSICS_UPDATE_TIMES:
			update_times = state;
			return;
		default:
			return;
	}
}
#endif
#pragma endregion

} // namespace Hfsm
