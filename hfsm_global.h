#pragma once
// #define ROLLBACK_NET_CODE
// #define DEBUG_IN_EDITOR

// 以 DEV_ENABLED 宏确定为 完整版
#ifdef DEV_ENABLED

#define FULL_VERSION

#endif


#ifdef DEBUG_ENABLED
#ifndef TOOLS_ENABLED
#define TOOLS_ENABLED
#endif // TOOLS_ENABLED
#endif //  DEBUG_ENABLED




// 字符常量以 utf8 进行编译
#pragma execution_character_set("utf-8")

#ifdef GDEXTENSION_BUILD
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/string_name.hpp>

#include "utils_macros.h"

using namespace godot;
#else
#include <core/templates/hash_map.h>

#include "../utils_macros.h"
#endif // GDEXTENSION_BUILD

namespace Hfsm {

class HfsmGlobal {
private:
	static PackedStringArray &_singleton_names() {
		static PackedStringArray singleton_names;
		return singleton_names;
	}
	static Array &_singletons() {
		static Array singletons;
		return singletons;
	}

public:
	static const PackedStringArray &get_singleton_names() { return _singleton_names(); }
	static const Array &get_singletons() { return _singletons(); }

	static void init_static();
	static void deinit_static();
};

}; // namespace Hfsm

#ifdef GDEXTENSION_BUILD
#define MOUSE_BUTTON(m_btn) MOUSE_BUTTON_##m_btn
#define KEY_MASK(m_btn) KEY_MASK_##m_btn
#define KEY(m_btn) KEY_##m_btn

#else // GDEXTENSION_BUILD

#define MOUSE_BUTTON(m_btn) MouseButton::m_btn
#define KEY_MASK(m_btn) KeyModifierMask::m_btn
#define KEY(m_btn) Key::m_btn

#endif // GDEXTENSION_BUILD

#define IS_PROP(m_prop_usage) !bool(int(m_prop_usage) & (PROPERTY_USAGE_GROUP | PROPERTY_USAGE_CATEGORY | PROPERTY_USAGE_SUBGROUP))

#ifdef FULL_VERSION
#define IF_FULL_VERSION(m_code) m_code
#else // FULL_VERSION
#define IF_FULL_VERSION(m_code)
#endif // FULL_VERSION

#define cb_resource_emit_changed(m_res_ptr) Callable(m_res_ptr, SNAME("emit_changed"))