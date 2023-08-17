/**************************************************************************/
/*  auto_transition.cpp                                                   */
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
