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

/*
 * The SNAME macro is used to speed up StringName creation, as it allows caching it after the first usage in a very efficient way.
 * It should NOT be used everywhere, but instead in places where high performance is required and the creation of a StringName
 * can be costly. Places where it should be used are:
 * - Control::get_theme_*(<name> and Window::get_theme_*(<name> functions.
 * - emit_signal(<name>,..) function
 * - call_deferred(<name>,..) function
 * - Comparisons to a StringName in overridden _set and _get methods.
 *
 * Use in places that can be called hundreds of times per frame (or more) is recommended, but this situation is very rare. If in doubt, do not use.
 */

#define SNAME(m_arg) ([]() -> const StringName & { static StringName sname = StringName(m_arg); return sname; })()

}; // namespace Hfsm
