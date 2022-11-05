#include "hfsm_global.hpp"

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/input.hpp>

namespace Hfsm {

// bool HfsmGlobal::in_editor = false;
VMap<StringName, Object *> HfsmGlobal::name2singleton = VMap<StringName, Object *>();

void HfsmGlobal::init_static() {
    // in_editor = Engine::get_singleton()->is_editor_hint();

    // 以此为模板添加全局单例
    HfsmGlobal::name2singleton.insert("Input", Input::get_singleton());
}

void HfsmGlobal::deinit_static(){
}

} // namespace Hfsm
