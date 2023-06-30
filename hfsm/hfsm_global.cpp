#include "hfsm_global.h"

#ifdef GDEXTENSION_BUILD
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/input.hpp>
#else
#include <core/input/input.h>

#endif // GDEXTENSION_BUILD

#ifdef TOOLS_ENABLED
#include "editor/state_node.h"
#endif // TOOLS_ENABLED

#include "src/state.h"

namespace Hfsm {

PackedStringArray HfsmGlobal::singleton_names = {};
Array HfsmGlobal::singletons = {};

#define ADD_SINGLETON(m_class)                                          \
	HfsmGlobal::singleton_names.push_back(m_class::get_class_static()); \
	HfsmGlobal::singletons.push_back(m_class::get_singleton())

void HfsmGlobal::init_static() {
	IF_TOOLS({
		StateNode::IN_COLOR = Color::named("ORANGE");
		StateNode::OUT_COLOR = Color::named("GREEN");
	})

	// 以此为模板添加全局单例
	ADD_SINGLETON(Input);
}

void HfsmGlobal::deinit_static() {
	HfsmGlobal::singleton_names.clear();
	HfsmGlobal::singletons.clear();
}

} // namespace Hfsm
