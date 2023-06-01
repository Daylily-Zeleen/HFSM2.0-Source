#pragma once
// #define ROLLBACK_NET_CODE
// #define DEBUG_IN_EDITOR

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

// Type safe macros.

#define _DECLTYPE_METHOD_TYPE(t_prefix, m_obj_ptr, m_method) \
	using t_prefix##m_method = decltype(&decltype(std::remove_reference_t<decltype(*(m_obj_ptr))>(*(m_obj_ptr)))::m_method)

#define _DECLTYPE_METHOD_RETURN_TYPE(t_prefix, m_obj_ptr, m_method, ...) \
	_DECLTYPE_METHOD_TYPE(t_prefix, m_obj_ptr, m_method);                \
	using t_prefix##m_method##_r = decltype((m_obj_ptr->*((std::remove_reference_t<t_prefix##m_method>(t_prefix##m_method()))))(__VA_ARGS__))

#define CALLABLE(m_obj_ptr, m_method)                      \
	[m_obj_ptr]() {                                        \
		_DECLTYPE_METHOD_TYPE(MT_, m_obj_ptr, m_method);   \
		return Callable(m_obj_ptr, StringName(#m_method)); \
	}()

#define NAMEOF(m_obj_ptr, m_method)                      \
	[m_obj_ptr]() {                                      \
		_DECLTYPE_METHOD_TYPE(MT_, m_obj_ptr, m_method); \
		return StringName(#m_method);                    \
	}()

#define TNAMEOF(m_method) NAMEOF(this, m_method)

#define TCALLABLE(m_method) CALLABLE(this, m_method)

#define CALLABLE_BIND(m_obj_ptr, m_method, ...) CALLABLE(m_obj_ptr, m_method).bindv(Array::make(__VA_ARGS__))
#define TCALLABLE_BINDV(m_method, ...) TCALLABLE(m_method).bindv(Array::make(__VA_ARGS__))

#define IS_CONNECTED(m_signal, m_obj_ptr, m_method) is_connected(m_signal, CALLABLE(m_obj_ptr, m_method))
#define DISCONNECT(m_signal, m_obj_ptr, m_method) disconnect(m_signal, CALLABLE(m_obj_ptr, m_method))

#define TIS_CONNECTED(m_signal, m_method) IS_CONNECTED(m_signal, this, m_method)
#define TDISCONNECT(m_signal, m_method) DISCONNECT(m_signal, this, m_method)

#define changed StringName("changed")

#define GDBIND_BEGIN(m_class) using T_BIND = m_class
#define GDBIND_METHOD(m_method, ...)                       \
	{                                                      \
		using TM_##m_method = decltype(&T_BIND::m_method); \
	}                                                      \
	ClassDB::bind_method(D_METHOD(#m_method, ##__VA_ARGS__), &T_BIND::m_method)

#define GDBIND_SETGET_BOOL(m_property)                   \
	using T_##m_property = decltype(T_BIND::m_property); \
	GDBIND_METHOD(is_##m_property);                      \
	GDBIND_METHOD(set_##m_property, #m_property);

#define GDBIND_SETGET(m_property)                        \
	using T_##m_property = decltype(T_BIND::m_property); \
	GDBIND_METHOD(get_##m_property);                     \
	GDBIND_METHOD(set_##m_property, #m_property);

#define GDADD_PROPERTY_ORIGIN(m_variant_type, m_property, ...)                      \
	GDBIND_SETGET(m_property);                                                      \
	ADD_PROPERTY(PropertyInfo(Variant::m_variant_type, #m_property, ##__VA_ARGS__), \
			"set_" #m_property, "get_" #m_property)

#define GDADD_PROPERTY(m_variant_type, m_property, ...) GDADD_PROPERTY_ORIGIN(m_variant_type, m_property, ##__VA_ARGS__)

#define GDADD_PROPERTY_BOOL(m_property, ...)                              \
	GDBIND_SETGET_BOOL(m_property);                                       \
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, #m_property, ##__VA_ARGS__), \
			"set_" #m_property, "is_" #m_property)

#define GDADD_PROPERTY_RESOURCE(m_property, ...)                             \
	GDADD_PROPERTY(Variant::OBJECT, m_property, PROPERTY_HINT_RESOURCE_TYPE, \
			decltype(std::remove_reference_t<decltype(*(T_##m_property().ptr()))>(*(T_##m_property().ptr())))::get_class_static(), ##__VA_ARGS__)

#define GDADD_PROPERTY_TYPED_ARRAY(m_property, m_class, ...)              \
	GDADD_PROPERTY(Variant::ARRAY, m_property, PROPERTY_HINT_TYPE_STRING, \
			vformat("%d/%d:%s", Variant::OBJECT, PROPERTY_HINT_RESOURCE_TYPE, m_class::get_class_static()), ##__VA_ARGS__)

// Undoredo
#define CREATE_ACTION(m_action_name) auto undo_redo = HfsmEditorPlugin::create_action(m_action_name)
#define COMMIT_ACTION() undo_redo->commit_action()

#define ADD_DO_METHOD(m_obj_ptr, m_method, ...)                                 \
	{                                                                           \
		_DECLTYPE_METHOD_RETURN_TYPE(ADM_, m_obj_ptr, m_method, ##__VA_ARGS__); \
	}                                                                           \
	undo_redo->add_do_method(m_obj_ptr, #m_method, ##__VA_ARGS__)
#define ADD_UNDO_METHOD(m_obj_ptr, m_method, ...)                                \
	{                                                                            \
		_DECLTYPE_METHOD_RETURN_TYPE(AUDM_, m_obj_ptr, m_method, ##__VA_ARGS__); \
	}                                                                            \
	undo_redo->add_undo_method(m_obj_ptr, #m_method, ##__VA_ARGS__)

#define ADD_DO_METHOD_UNCHECK_ARGS(m_obj_ptr, m_method, ...) \
	{                                                        \
		_DECLTYPE_METHOD_TYPE(ADM_, m_obj_ptr, m_method);    \
	}                                                        \
	undo_redo->add_do_method(m_obj_ptr, #m_method, ##__VA_ARGS__)
#define ADD_UNDO_METHOD_UNCHECK_ARGS(m_obj_ptr, m_method, ...) \
	{                                                          \
		_DECLTYPE_METHOD_TYPE(AUDM_, m_obj_ptr, m_method);     \
	}                                                          \
	undo_redo->add_undo_method(m_obj_ptr, #m_method, ##__VA_ARGS__)
#define ADD_DO_REFERENCE(m_obj) undo_redo->add_do_reference(m_obj)
#define ADD_UNDO_REFERENCE(m_obj) undo_redo->add_undo_reference(m_obj)

// #define TADD_DO_METHOD(m_method, ...)                                      \
// 	{                                                                      \
// 		_DECLTYPE_METHOD_RETURN_TYPE(ADM_, this, m_method, ##__VA_ARGS__); \
// 	}                                                                      \
// 	undo_redo->add_do_method(this, #m_method, ##__VA_ARGS__)
// #define TADD_UNDO_METHOD(m_method, ...)                                     \
// 	{                                                                       \
// 		_DECLTYPE_METHOD_RETURN_TYPE(AUDM_, this, m_method, ##__VA_ARGS__); \
// 	}                                                                       \
// 	undo_redo->add_undo_method(this, #m_method, ##__VA_ARGS__)

// #define TADD_DO_METHOD_UNCHECK_ARGS(m_method, ...)   \
// 	{                                                \
// 		_DECLTYPE_METHOD_TYPE(ADM_, this, m_method); \
// 	}                                                \
// 	undo_redo->add_do_method(this, #m_method, ##__VA_ARGS__)
// #define TADD_UNDO_METHOD_UNCHECK_ARGS(m_method, ...)  \
// 	{                                                 \
// 		_DECLTYPE_METHOD_TYPE(AUDM_, this, m_method); \
// 	}                                                 \
// 	undo_redo->add_undo_method(this, #m_method, ##__VA_ARGS__)

}; // namespace Hfsm
