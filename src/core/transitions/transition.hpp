#pragma once

#include "transition_base.hpp"

using namespace godot;

namespace Hfsm {
// class HFSM;
// class State;

class Transition : public RefCounted, public TransitionBase {
	GDCLASS(Transition, RefCounted)
protected:
	static void _bind_methods();

	_TO_STRING()

public:
	// TODO:: call 是否会引发错误？
	// 能否不走 call 调用真正的虚方法？
	void refresh() override { call(SNAME("_refresh")); }
	bool can_transit() override { return bool(call(SNAME("_can_transit"))); }

	virtual void _refresh() {}
	virtual bool _can_transit() { return false; }

	Ref<State> get_from_state() override { return TransitionBase::get_from_state(); }
	Ref<State> get_to_state() override { return TransitionBase::get_to_state(); }

	HFSM *get_hfsm() { return hfsm; }

	// Dictionary get_context() { return hfsm->get_context(); }

private:
	HFSM *hfsm = nullptr;

	friend class TransitionRes;
};
}; // namespace Hfsm
