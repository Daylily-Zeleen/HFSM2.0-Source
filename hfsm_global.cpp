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
