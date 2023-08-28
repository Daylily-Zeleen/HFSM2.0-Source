/**************************************************************************/
/*  transition.cpp                                                        */
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

#include "transition.h"

namespace HFSM2 {

void Transition::_bind_methods() {
	GDBIND_BEGIN(Transition);
	GDVIRTUAL_BIND(_refresh);
	GDVIRTUAL_BIND(_can_transit);

	GDBIND_BEGIN(Transition);
	GDBIND_METHOD(get_from_state);
	GDBIND_METHOD(get_to_state);
	GDBIND_METHOD(get_hfsm);

#ifdef ROLLBACK_NET_CODE
	BIND_VIRTUAL_METHOD(Transition, _save_state);
	BIND_VIRTUAL_METHOD(Transition, _load_state);
	BIND_VIRTUAL_METHOD(Transition, _interpolate_state);
	BIND_VIRTUAL_METHOD(Transition, _get_local_input);
	BIND_VIRTUAL_METHOD(Transition, _predict_remote_input);
	BIND_VIRTUAL_METHOD(Transition, _network_process);
	BIND_VIRTUAL_METHOD(Transition, _network_preprocess);
	BIND_VIRTUAL_METHOD(Transition, _network_postprocess);
	BIND_VIRTUAL_METHOD(Transition, _network_spawn_preprocess);
	BIND_VIRTUAL_METHOD(Transition, _network_spawn);
	BIND_VIRTUAL_METHOD(Transition, _network_despawn);
#endif
}

}; // namespace HFSM2
