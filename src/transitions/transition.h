#pragma once

#include "transition_base.h"

using namespace godot;

namespace Hfsm {

class Transition : public RefCounted, public TransitionBase {
	GDCLASS(Transition, RefCounted)
protected:
	static void _bind_methods();

	_TO_STRING()

public:
	GDVIRTUAL0(_refresh);
	GDVIRTUAL0R(bool, _can_transit);

	void refresh() override {
		IF_GDE(call(SNAME("_refresh"));)
		IF_GDM(GDVIRTUAL_CALL(_refresh);)
	}

	bool can_transit() override {
		IF_GDE(return bool(call(SNAME("_can_transit")));)
		IF_GDM({
			bool ret;
			if (GDVIRTUAL_CALL(_can_transit, ret)) {
				return ret;
			}
			return false;
		})
	}

	Ref<State> get_from_state() override { return TransitionBase::get_from_state(); }
	Ref<State> get_to_state() override { return TransitionBase::get_to_state(); }

	HFSM *get_hfsm() { return hfsm; }

	Transition() = default;

	Transition(HFSM *p_hfsm, const Ref<Script> &p_script) {
		hfsm = p_hfsm;
		set_script(p_script);
	}

	operator TransitionBase *() { return static_cast<TransitionBase *>(this); }

private:
	HFSM *hfsm = nullptr;
};
}; // namespace Hfsm
