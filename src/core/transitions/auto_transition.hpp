#ifndef AUTO_TRANSITION_H
#define AUTO_TRANSITION_H

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/time.hpp>
#include "transition_base.hpp"
#include "../state.hpp"
#include "../fsm.hpp"

namespace Hfsm {
enum AuotoTtransitMode {
    AUTO_TRANSIT_MODE_DELAY_TIMER,
    AUTO_TRANSIT_MODE_FSM_EXIT,
    AUTO_TRANSIT_MODE_MANUAL,
    AUTO_TRANSIT_MODE_UPDATE_TIMES,
    AUTO_TRANSIT_MODE_PHYSICS_UPDATE_TIMES,
    AUTO_TRANSIT_MODE_MAX,
};

// 自动转换
class AutoTransition : public TransitionBase {
public:

    void refresh() override;
    bool can_transit() override;

#ifdef ROLLBACK_NET_CODE
    Variant _save_state() override;

    void _load_state(const Variant &state) override;
#endif
private:
    uint8_t _mode = AUTO_TRANSIT_MODE_DELAY_TIMER;
    uint64_t _delay_msec = 1000;
    uint64_t _times = 5;

    uint64_t _next_delay_transit_tick = 0;
    uint64_t _update_times = 0;
    
    friend class TransitionRes;
};


#pragma region 内联实现


inline void AutoTransition::refresh() {
    switch (_mode) {
    case AUTO_TRANSIT_MODE_DELAY_TIMER:
        _next_delay_transit_tick = Time::get_singleton()->get_ticks_msec() + _delay_msec;
        break;
    case AUTO_TRANSIT_MODE_UPDATE_TIMES:
    case AUTO_TRANSIT_MODE_PHYSICS_UPDATE_TIMES:
        _update_times = _update_times;
        break;
    }
}
inline bool AutoTransition::can_transit() {
    switch (_mode) {
        case AUTO_TRANSIT_MODE_DELAY_TIMER:
            return Time::get_singleton()->get_ticks_msec() > _next_delay_transit_tick;
        case AUTO_TRANSIT_MODE_FSM_EXIT: {
            auto _fsm = get_from_state()->get_fsm();
            if (_fsm && !_fsm->is_running()) {
                return true;
            }
            return false;
        }
        case AUTO_TRANSIT_MODE_MANUAL:
            return get_from_state()->is_exited();
        case AUTO_TRANSIT_MODE_UPDATE_TIMES:
            if (!Engine::get_singleton()->is_in_physics_frame()) {
                _update_times--;
            }
            return _update_times == 0;
        case AUTO_TRANSIT_MODE_PHYSICS_UPDATE_TIMES:
            if (Engine::get_singleton()->is_in_physics_frame()) {
                _update_times--;
            }
            return _update_times == 0;
        default:
            return false;
    }
}



#pragma endregion
} // namespace Hfsm

#endif