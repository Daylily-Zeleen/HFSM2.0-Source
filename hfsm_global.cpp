/**************************************************************************/
/*  hfsm_global.cpp                                                       */
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

#include "hfsm_global.h"

#ifdef GDEXTENSION_BUILD
#include <godot_cpp/classes/engine.hpp>
#else // GDEXTENSION_BUILD

#endif // GDEXTENSION_BUILD

#ifdef TOOLS_ENABLED
#include "editor/state_node.h"
#endif // TOOLS_ENABLED

#include "src/state.h"

namespace Hfsm {

void HfsmGlobal::init_static() {
	auto &singleton_names = _singleton_names();
	auto &singletons = _singletons();

	IF_GDE({
		auto sigleton_list = Engine::get_singleton()->get_singleton_list();
		// TODO:: Waiting for ClassDB singleton.
		sigleton_list.remove_at(sigleton_list.find("ClassDB"));

		singleton_names.resize(sigleton_list.size());
		singletons.resize(sigleton_list.size());
		for (auto i = 0; i < sigleton_list.size(); ++i) {
			singleton_names[i] = sigleton_list[i];
			singletons[i] = Engine::get_singleton()->get_singleton({ sigleton_list[i] });
		}
	})

	IF_GDM({
		List<Engine::Singleton> singleton_list;
		Engine::get_singleton()->get_singletons(&singleton_list);
		singleton_names.resize(singleton_list.size());
		singletons.resize(singleton_list.size());
		int idx = 0;
		for (const auto &singleton : singleton_list) {
			singleton_names.set(idx, singleton.name);
			singletons.set(idx, singleton.ptr);
			idx++;
		}
	})
}

void HfsmGlobal::deinit_static() {
	_singleton_names().clear();
	_singletons().clear();
}

} // namespace Hfsm
