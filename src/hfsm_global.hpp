#pragma once
// #define ROLLBACK_NET_CODE
// #define DEBUG_IN_EDITOR

#define DEV_ENABLED
// 以 DEV_ENABLED 宏确定为 完整版
#ifdef DEV_ENABLED

#define FULL_VERSION

#endif

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
