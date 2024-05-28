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

// #include "hfsm_global.gen.h"
#else // GDEXTENSION_BUILD

#endif // GDEXTENSION_BUILD

#ifdef TOOLS_ENABLED
#include "editor/state_node.h"
#endif // TOOLS_ENABLED

#include "src/state.h"

namespace HFSM2 {

#ifdef GDE_COMPATIBILITY_ENABLED
HFSMGlobal::GodotVersion HFSMGlobal::godot_version;
#endif // GDE_COMPATIBILITY_ENABLED

void HFSMGlobal::init_static() {
#ifdef GDE_COMPATIBILITY_ENABLED
	Dictionary version_info = Engine::get_singleton()->get_version_info();
	godot_version = { version_info["major"], version_info["minor"], version_info["patch"] };
#endif // GDE_COMPATIBILITY_ENABLED

	auto &singleton_names = _singleton_names();
	auto &singletons = _singletons();

	IF_GDE({
		auto singleton_list = Engine::get_singleton()->get_singleton_list();

		// Specially, these wrong singletons.
		// TODO::
		// "GDExtensionManager", "ResourceUID", "IP", "Time": https://github.com/godotengine/godot/issues/81030
		// Crash at exit editor.
		for (const String &klass : {
					 "GDExtensionManager",
					 "ResourceUID",
					 "IP",
					 "Time",
			 }) {
			auto idx = singleton_list.find(klass);
			if (idx >= 0) {
				singleton_list.remove_at(idx);
			}
		}

		singleton_names.resize(singleton_list.size());
		singletons.resize(singleton_list.size());
		for (auto i = 0; i < singleton_list.size(); ++i) {
			singleton_names[i] = singleton_list[i];
			singletons[i] = Engine::get_singleton()->get_singleton({ singleton_list[i] });
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

void HFSMGlobal::deinit_static() {
	_singleton_names().clear();
	_singletons().clear();
}

#ifdef GDE_COMPATIBILITY_ENABLED
bool HFSMGlobal::is_4_point_2_or_later() {
	if (godot_version.major > 4) {
		return true;
	}
	if (godot_version.minor >= 2) {
		return true;
	}
	return false;
}

bool HFSMGlobal::is_4_point_3_or_later() {
	if (godot_version.major > 4) {
		return true;
	}
	if (godot_version.minor >= 3) {
		return true;
	}
	return false;
}
#endif // GDE_COMPATIBILITY_ENABLED

} // namespace HFSM2
