/**************************************************************************/
/*  transition.h                                                          */
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

#include "transition_base.h"

using namespace godot;

namespace HFSM2 {

class Transition : public RefCounted, public TransitionBase {
	GDCLASS(Transition, RefCounted)
protected:
	static void _bind_methods();

	_TO_STRING()

public:
	GDVIRTUAL0(_refresh);
	GDVIRTUAL0R(bool, _can_transit);

	void refresh() override {
		GDVIRTUAL_CALL(_refresh);
	}

	bool can_transit() override {
		IF_GDE(return bool(GDVIRTUAL_CALL(_can_transit));)
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

	Transition(HFSM *p_hfsm) :
			hfsm(p_hfsm) {}

	operator TransitionBase *() { return static_cast<TransitionBase *>(this); }

private:
	HFSM *hfsm = nullptr;
};

}; // namespace HFSM2
