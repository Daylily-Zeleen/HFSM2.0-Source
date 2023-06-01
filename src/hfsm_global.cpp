#include "hfsm_global.hpp"

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/input.hpp>

namespace Hfsm {

// bool HfsmGlobal::in_editor = false;
HashMap<StringName, Object *> HfsmGlobal::name2singleton = HashMap<StringName, Object *>();

void HfsmGlobal::init_static() {
	// in_editor = Engine::get_singleton()->is_editor_hint();
#define INSERT_GLOBAL_CLASS(m_class) HfsmGlobal::name2singleton.insert(m_class::get_class_static(), m_class::get_singleton())
	// 以此为模板添加全局单例
	INSERT_GLOBAL_CLASS(Input);
}

void HfsmGlobal::deinit_static() {
	HfsmGlobal::name2singleton.clear();
}

} // namespace Hfsm
