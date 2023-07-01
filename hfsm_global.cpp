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

PackedStringArray HfsmGlobal::singleton_names = {};
Array HfsmGlobal::singletons = {};

void HfsmGlobal::init_static() {
	IF_TOOLS({
		StateNode::IN_COLOR = Color::named("ORANGE");
		StateNode::OUT_COLOR = Color::named("GREEN");
	})

	IF_GDE({
		auto sigleton_list = Engine::get_singleton()->get_singleton_list();
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
	HfsmGlobal::singleton_names.clear();
	HfsmGlobal::singletons.clear();
}

} // namespace Hfsm
