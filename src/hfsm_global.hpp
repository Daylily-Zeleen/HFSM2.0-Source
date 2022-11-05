// #define ROLLBACK_NET_CODE
// #define DEBUG_IN_EDITOR 

#ifndef HFSM_GLOBAL_H
#define HFSM_GLOBAL_H

// 字符常量以 utf8 进行编译
#pragma execution_character_set("utf-8")

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/templates/vmap.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/string_name.hpp>


using namespace godot;
namespace Hfsm {
class HfsmGlobal {
public:
    // 插件内唯一识别是否再编辑器内
    static VMap<StringName, Object *> name2singleton;
    static void init_static();
    static void deinit_static();
};
}; // namespace Hfsm

#endif