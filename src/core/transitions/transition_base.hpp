#pragma once

#include <godot_cpp/classes/ref_counted.hpp>

#include <godot_cpp/core/binder_common.hpp>

#include "../../hfsm_global.hpp"
#include "../hfsm.hpp"
#include "../hfsm_variable.hpp"

using namespace godot;

namespace Hfsm {
class HFSM;
class State;
class FsmRes;

enum TransitionType {
	TRANSITION_TYPE_SCRIPT,
	TRANSITION_TYPE_VARIABLE,
	TRANSITION_TYPE_EXPRESSION,
	TRANSITION_TYPE_AUTO,
};

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

	friend class FsmRes;
};

#pragma region 内联实现
inline Ref<State> TransitionBase::get_from_state() { return from_state; }
inline Ref<State> TransitionBase::get_to_state() { return to_state; }

#pragma endregion

}; // namespace Hfsm
