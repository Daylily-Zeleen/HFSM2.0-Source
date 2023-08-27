/**************************************************************************/
/*  utils_macros.h                                                        */
/**************************************************************************/
/*                         This file is part of:                          */
/*                   Hierarchical Finite State Machine                    */
/*            https://github.com/Daylily-Zeleen/HFSM2.0-Source            */
/**************************************************************************/
/* Copyright (c) 2023-present Daylily Zeleen.                             */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#pragma once

/*
	GDEXTENSION_BUILD
	GODOT_MODULE_BUILD
	IDE_TYPE_SAFE
*/

#ifdef GODOT_MODULE_BUILD
#undef GDEXTENSION_BUILD
#endif

#ifdef GDEXTENSION_BUILD
#undef GODOT_MODULE_BUILD
#endif

#ifndef GDEXTENSION_BUILD
#include <array>
#endif

#define s_changed StringName("changed")

#ifdef GDEXTENSION_BUILD
#define _TO_STRING() \
	String _to_string() { return vformat("[%s:%d]", get_class_static(), get_instance_id()); }

#define SNAME(m_arg) ([]() -> const StringName & { static StringName sname = StringName(m_arg); return sname; })()
#else
#define _TO_STRING() // Godot module no need to override _to_string()
#endif // GDEXTENSION_BUILD

// Type safe macros.
#ifdef IDE_TYPE_SAFE

#define _DECLTYPE_PTR_MEMBER(t_prefix, m_obj_ptr, m_member) \
	using t_prefix##m_member = decltype(&std::remove_pointer_t<decltype(m_obj_ptr)>::m_member)

#define DECLTYPE_METHOD_RETURN_TYPE(m_obj_ptr, m_method, ...) \
	{ using t_prefix##m_method##_r = decltype(m_obj_ptr->m_method(__VA_ARGS__)); }

#ifdef GDEXTENSION_BUILD
#define CALLABLE(m_obj_ptr, m_method)                      \
	[m_obj_ptr]() {                                        \
		_DECLTYPE_PTR_MEMBER(MT_, m_obj_ptr, m_method);    \
		return Callable(m_obj_ptr, StringName(#m_method)); \
	}()
#else // GDEXTENSION_BUILD
#define CALLABLE(m_obj_ptr, m_method) callable_mp(m_obj_ptr, &std::remove_pointer_t<decltype(m_obj_ptr)>::m_method)
#endif // GDEXTENSION_BUILD

#define NAMEOF(m_obj_ptr, m_property)                     \
	[m_obj_ptr]() {                                       \
		_DECLTYPE_PTR_MEMBER(MT_, m_obj_ptr, m_property); \
		return StringName(#m_property);                   \
	}()

#define GDBIND_METHOD_DIFF(m_method_name, m_method, ...)   \
	{ using TM_##m_method = decltype(&T_BIND::m_method); } \
	ClassDB::bind_method(D_METHOD(m_method_name, ##__VA_ARGS__), &T_BIND::m_method)

#define GDBIND_METHOD(m_method, ...)                       \
	{ using TM_##m_method = decltype(&T_BIND::m_method); } \
	ClassDB::bind_method(D_METHOD(#m_method, ##__VA_ARGS__), &T_BIND::m_method)

#ifdef GDEXTENSION_BUILD
#define GDBIND_CALBACK(m_method, ...) GDBIND_METHOD(m_method, ##__VA_ARGS__)
#else // GDEXTENSION_BUILD
#define GDBIND_CALBACK(m_method, ...) \
	{ using TM_##m_method = decltype(&T_BIND::m_method); }

#endif // GDEXTENSION_BUILD

#define GDBIND_SETGET_BOOL(m_property)                   \
	using T_##m_property = decltype(T_BIND::m_property); \
	GDBIND_METHOD(is_##m_property);                      \
	GDBIND_METHOD(set_##m_property, #m_property)

#define GDBIND_SETGET(m_property)                        \
	using T_##m_property = decltype(T_BIND::m_property); \
	GDBIND_METHOD(get_##m_property);                     \
	GDBIND_METHOD(set_##m_property, #m_property)

// Undoredo
#define ADD_DO_METHOD(m_obj_ptr, m_method, ...)                      \
	DECLTYPE_METHOD_RETURN_TYPE(m_obj_ptr, m_method, ##__VA_ARGS__); \
	undo_redo->add_do_method(m_obj_ptr, #m_method, ##__VA_ARGS__)
#define ADD_UNDO_METHOD(m_obj_ptr, m_method, ...)                    \
	DECLTYPE_METHOD_RETURN_TYPE(m_obj_ptr, m_method, ##__VA_ARGS__); \
	undo_redo->add_undo_method(m_obj_ptr, #m_method, ##__VA_ARGS__)

#else // IDE_TYPE_SAFE

#ifdef GDEXTENSION_BUILD
#define CALLABLE(m_obj_ptr, m_method) Callable(m_obj_ptr, StringName(#m_method))
#else // GDEXTENSION_BUILD
#define CALLABLE(m_obj_ptr, m_method) callable_mp(m_obj_ptr, &std::remove_pointer_t<decltype(m_obj_ptr)>::m_method)
#endif // GDEXTENSION_BUILD

#define NAMEOF(m_obj_ptr, m_property) StringName(#m_property)

#define GDBIND_BEGIN(m_class) using T_BIND = m_class

#define GDBIND_METHOD_DIFF(m_method_name, m_method, ...) \
	ClassDB::bind_method(D_METHOD(m_method_name, ##__VA_ARGS__), &T_BIND::m_method)
#define GDBIND_METHOD(m_method, ...) ClassDB::bind_method(D_METHOD(#m_method, ##__VA_ARGS__), &T_BIND::m_method)

#ifdef GDEXTENSION_BUILD
#define GDBIND_CALBACK(m_method, ...) GDBIND_METHOD(m_method, ##__VA_ARGS__)
#else // GDEXTENSION_BUILD
#define GDBIND_CALBACK(m_method, ...)
#endif // GDEXTENSION_BUILD

#define GDBIND_SETGET_BOOL(m_property) \
	GDBIND_METHOD(is_##m_property);    \
	GDBIND_METHOD(set_##m_property, #m_property)

#define GDBIND_SETGET(m_property)    \
	GDBIND_METHOD(get_##m_property); \
	GDBIND_METHOD(set_##m_property, #m_property)

// Undoredo
#define ADD_DO_METHOD(m_obj_ptr, m_method, ...) undo_redo->add_do_method(m_obj_ptr, #m_method, ##__VA_ARGS__)
#define ADD_UNDO_METHOD(m_obj_ptr, m_method, ...) undo_redo->add_undo_method(m_obj_ptr, #m_method, ##__VA_ARGS__)

#endif // IDE_TYPE_SAFE

//

#define TNAMEOF(m_property) NAMEOF(this, m_property)
#define TCALLABLE(m_method) CALLABLE(this, m_method)

#ifdef GDEXTENSION_BUILD
#define CALLABLE_BIND(m_obj_ptr, m_method, ...) CALLABLE(m_obj_ptr, m_method).bindv(Array::make(__VA_ARGS__))
#define TCALLABLE_BIND(m_method, ...) TCALLABLE(m_method).bindv(Array::make(__VA_ARGS__))
#else // GDEXTENSION_BUILD
#define CALLABLE_BIND(m_obj_ptr, m_method, ...) CALLABLE(m_obj_ptr, m_method).bind(__VA_ARGS__)
#define TCALLABLE_BIND(m_method, ...) TCALLABLE(m_method).bind(__VA_ARGS__)
#endif // GDEXTENSION_BUILD

#define IS_CONNECTED(m_signal, m_obj_ptr, m_method) is_connected(m_signal, CALLABLE(m_obj_ptr, m_method))
#define DISCONNECT(m_signal, m_obj_ptr, m_method) disconnect(m_signal, CALLABLE(m_obj_ptr, m_method))

#define TIS_CONNECTED(m_signal, m_method) IS_CONNECTED(m_signal, this, m_method)
#define TDISCONNECT(m_signal, m_method) DISCONNECT(m_signal, this, m_method)

//
#define GDBIND_BEGIN(m_class) using T_BIND = m_class

#ifdef GDEXTENSION_BUILD
#define GDADD_PROPERTY_ORIGIN(m_variant_type, m_property, ...)                                                          \
	GDBIND_SETGET(m_property);                                                                                          \
	godot::ClassDB::add_property(get_class_static(), PropertyInfo(Variant::m_variant_type, #m_property, ##__VA_ARGS__), \
			"set_" #m_property, "get_" #m_property)

#define GDADD_PROPERTY_BOOL(m_property, ...)                                                                  \
	GDBIND_SETGET_BOOL(m_property);                                                                           \
	godot::ClassDB::add_property(get_class_static(), PropertyInfo(Variant::BOOL, #m_property, ##__VA_ARGS__), \
			"set_" #m_property, "is_" #m_property)

#else // GDEXTENSION_BUILD
#define GDADD_PROPERTY_ORIGIN(m_variant_type, m_property, ...)                                                     \
	GDBIND_SETGET(m_property);                                                                                     \
	::ClassDB::add_property(get_class_static(), PropertyInfo(Variant::m_variant_type, #m_property, ##__VA_ARGS__), \
			"set_" #m_property, "get_" #m_property)

#define GDADD_PROPERTY_BOOL(m_property, ...)                                                             \
	GDBIND_SETGET_BOOL(m_property);                                                                      \
	::ClassDB::add_property(get_class_static(), PropertyInfo(Variant::BOOL, #m_property, ##__VA_ARGS__), \
			"set_" #m_property, "is_" #m_property)

#endif // GDEXTENSION_BUILD

#define GDADD_PROPERTY(m_variant_type, m_property, ...) GDADD_PROPERTY_ORIGIN(m_variant_type, m_property, ##__VA_ARGS__)

#define GDADD_PROPERTY_RESOURCE(m_property, ...)                             \
	GDADD_PROPERTY(Variant::OBJECT, m_property, PROPERTY_HINT_RESOURCE_TYPE, \
			std::remove_pointer_t<decltype(m_property.ptr())>::get_class_static(), ##__VA_ARGS__)

#define GDADD_PROPERTY_TYPED_ARRAY(m_property, m_class, ...)              \
	GDADD_PROPERTY(Variant::ARRAY, m_property, PROPERTY_HINT_TYPE_STRING, \
			vformat("%d/%d:%s", Variant::OBJECT, PROPERTY_HINT_RESOURCE_TYPE, m_class::get_class_static()), ##__VA_ARGS__)

// Dynamic bind
#include <type_traits>
template <class Var, typename T>
static T _convert_to(const Var &p_var) {
	if constexpr (std::is_enum<T>().value) {
		return T(int(p_var));
	} else {
		return T(p_var);
	}
}

#define _TRY_SET_PROP_ORIGIN(m_name_id, m_prop_id, m_prop)               \
	if ((m_name_id) == TNAMEOF(m_prop)) {                                \
		set_##m_prop(_convert_to<Variant, decltype(m_prop)>(m_prop_id)); \
		return true;                                                     \
	}
#define _TRY_SET_PROP(m_prop) _TRY_SET_PROP_ORIGIN(p_name, p_property, m_prop)

#define _TRY_GET_PROP_ORIGIN(getter_prefix, m_name_id, m_prop_id, m_prop) \
	if ((m_name_id) == TNAMEOF(m_prop)) {                                 \
		(m_prop_id) = getter_prefix##m_prop();                            \
		return true;                                                      \
	}
#define _TRY_GET_PROP(m_prop) _TRY_GET_PROP_ORIGIN(get_, p_name, r_property, m_prop)
#define _TRY_GET_PROPB(m_prop) _TRY_GET_PROP_ORIGIN(is_, p_name, r_property, m_prop)

#define _PUSH_PROP_ORIGIN(m_list_id, m_typ, m_prop, ...) m_list_id->push_back(PropertyInfo(Variant::m_typ, TNAMEOF(m_prop), ##__VA_ARGS__))
#define _PUSH_PROP(m_typ, m_prop, ...) _PUSH_PROP_ORIGIN(p_list, m_typ, m_prop, ##__VA_ARGS__)
#define _PUSH_PROP_RESOURCE(m_prop, ...) _PUSH_PROP_ORIGIN(p_list, OBJECT, m_prop, PROPERTY_HINT_RESOURCE_TYPE, \
		std::remove_pointer_t<decltype(m_prop.ptr())>::get_class_static(), ##__VA_ARGS__)
#define _PUSH_PROP_TYPED_ARRAY(m_prop, m_class, ...) _PUSH_PROP_ORIGIN(p_list, ARRAY, m_prop, PROPERTY_HINT_TYPE_STRING, \
		vformat("%d/%d:%s", Variant::OBJECT, PROPERTY_HINT_RESOURCE_TYPE, m_class::get_class_static()), ##__VA_ARGS__)

// UNDO REDO
#define CREATE_ACTION(m_action_name) auto undo_redo = []() {              \
	EditorUndoRedoManager::get_singleton()->create_action(m_action_name); \
	return EditorUndoRedoManager::get_singleton();                        \
}()
#define COMMIT_ACTION() undo_redo->commit_action()
#define ADD_DO_REFERENCE(m_obj) undo_redo->add_do_reference(m_obj)
#define ADD_UNDO_REFERENCE(m_obj) undo_redo->add_undo_reference(m_obj)

// Log =============================================
#ifdef GDEXTENSION_BUILD

#ifdef TOOLS_ENABLED
#include <godot_cpp/variant/utility_functions.hpp>
#endif // TOOLS_ENABLED

#define ED_MSG(fmt, ...) IF_TOOLS(UtilityFunctions::printerr(vformat(fmt, ##__VA_ARGS__)))

#define VLog(fmt, ...) IF_DEBUG(UtilityFunctions::print_verbose(vformat(fmt, ##__VA_ARGS__)))
#define DLog(fmt, ...) IF_DEBUG(WARN_PRINT(vformat(fmt, ##__VA_ARGS__)))
#define WLog(fmt, ...) IF_DEBUG(WARN_PRINT(vformat("[%s:%d]", __FILE__, __LINE__) + vformat(fmt, ##__VA_ARGS__)))
#define ELog(fmt, ...) IF_DEBUG(UtilityFunctions::printerr(vformat("[%s:%d]", __FILE__, __LINE__) + vformat(fmt, ##__VA_ARGS__)))

#else // GDEXTENSION_BUILD
#ifdef TOOLS_ENABLED
#include "core/string/print_string.h"
#endif // TOOLS_ENABLED
// #include "core/variant/variant.h"

#define ED_MSG(fmt, ...) IF_TOOLS(print_error(vformat(fmt, ##__VA_ARGS__)))

#define VLog(fmt, ...) IF_DEBUG(print_verbose(vformat(fmt, ##__VA_ARGS__)))
#define DLog(fmt, ...) IF_DEBUG(print_line(vformat(fmt, ##__VA_ARGS__)))
#define WLog(fmt, ...) IF_DEBUG(print_line(vformat("[%s:%d]", __FILE__, __LINE__) + vformat(fmt, ##__VA_ARGS__)))
#define ELog(fmt, ...) IF_DEBUG(print_error(vformat("[%s:%d]", __FILE__, __LINE__) + vformat(fmt, ##__VA_ARGS__)))

#endif // GDEXTENSION_BUILD

// Arr

#ifdef GDEXTENSION_BUILD
#include <godot_cpp/variant/array.hpp>
#include <type_traits>

using Array = godot::Array;
#endif // GDEXTENSION_BUILD

template <typename TArr, class = typename std::enable_if<!std::is_same_v<TArr, Array>, TArr>::type, class... Args>
static TArr make_arr(Args... args) {
#ifdef GDEXTENSION_BUILD
	return godot::helpers::append_all(TArr(), args...);
#else
	return { args... };
#endif // GDEXTENSION_BUILD
}

template <typename TArr, class = typename std::enable_if<std::is_same_v<TArr, Array>, Array>::type, class... Args>
static Array make_arr(Args... args) {
#ifdef GDEXTENSION_BUILD
	return Array::make(args...);
#else
	Array ret;
	std::array<Variant, sizeof...(Args)> elements{ Variant(args)... };
	ret.resize(sizeof...(Args));
	for (auto i = 0; i < sizeof...(args); ++i) {
		ret.set(i, elements[i]);
	}
	return ret;
#endif // GDEXTENSION_BUILD
}

//============================================
#ifdef GDEXTENSION_BUILD
#define GDVIRTUAL0(m_method)
#define GDVIRTUAL0R(r_type, m_method)
#define GDVIRTUAL1(m_method, m_type1)
#define GDVIRTUAL1R(r_type, m_method, m_type1)
#define GDVIRTUAL_BIND(m_method, ...) BIND_VIRTUAL_METHOD(T_BIND, m_method) // TODO:: 等待GDE实现自定义的虚方法绑定
#define GDVIRTUAL_CALL(m_method, ...) call(SNAME(#m_method), ##__VA_ARGS__)

#endif //GDEXTENSION_BUILD

#ifdef GDEXTENSION_BUILD
#define GD_(m_method_name) _##m_method_name
#else // GDEXTENSION_BUILD
#define GD_(m_method_name) m_method_name
#endif // GDEXTENSION_BUILD

// ==========================

#if defined(DEBUG_ENABLED) && defined(TOOLS_ENABLED)
#define IF_DEBUG_TOOL(m_code) m_code
#define IF_NOT_DEBUG_TOOL(m_code)
#else // defined (DEBUG_ENABLED) && defined (TOOLS_ENABLED)
#define IF_DEBUG_TOOL(m_code)
#define IF_NOT_DEBUG_TOOL(m_code) m_code
#endif //defined (DEBUG_ENABLED) && defined (TOOLS_ENABLED)

#ifdef DEBUG_ENABLED
#define IF_DEBUG(m_debug_code) m_debug_code
#define IF_NOT_DEBUG(m_not_debug_code)
#else // DEBUG_ENABLED
#define IF_DEBUG(m_debug_code)
#define IF_NOT_DEBUG(m_not_debug_code) m_not_debug_code
#endif // DEBUG_ENABLED

#ifdef TOOLS_ENABLED
#define IF_TOOLS(m_tools_code) m_tools_code
#define IF_NOT_TOOLS(m_not_tools_code)
#else // TOOLS_ENABLED
#define IF_TOOLS(m_tools_code)
#define IF_NOT_TOOLS(m_not_tools_code) m_not_tools_code
#endif // TOOLS_ENABLED

#ifdef GDEXTENSION_BUILD
#define IF_GDE(m_gde_code) m_gde_code
#define IF_GDM(m_gdm_code)
#else // GDEXTENSION_BUILD
#define IF_GDE(m_gde_code)
#define IF_GDM(m_gdm_code) m_gdm_code
#endif // GDEXTENSION_BUILD

#ifdef DEV_ENABLED
#define IF_DEV(m_code) m_code
#define IF_NOT_DEV(m_code)
#else // DEV_ENABLED
#define IF_DEV(m_code)
#define IF_NOT_DEV(m_code) m_code
#endif // DEV_ENABLED

#ifdef GDEXTENSION_BUILD
#include <godot_cpp/classes/weak_ref.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;
#else // GDEXTENSION_BUILD
#include <core/object/ref_counted.h>
#include <core/variant/variant_utility.h>
#endif // GDEXTENSION_BUILD
template <typename T, std::enable_if<std::is_convertible_v<T, Object *> || std::is_convertible_v<T, Ref<RefCounted>>> *valve = nullptr>
inline static Ref<WeakRef> weakref(T p_obj) {
	IF_GDE(return UtilityFunctions::weakref(p_obj);)
	IF_GDM({
		Callable::CallError err;
		return VariantUtilityFunctions::weakref(p_obj, err);
	})
}
inline static Ref<WeakRef> weakref(Object *p_obj) { return weakref(p_obj); }
inline static Ref<WeakRef> weakref(const Ref<RefCounted> &p_obj) { return weakref(p_obj); }
template <typename T, std::enable_if<std::is_convertible_v<T, Object *> || std::is_convertible_v<T, Ref<RefCounted>>> *valve = nullptr>
inline static T get_ref(const Ref<WeakRef> &p_weak_ref) {
	if (p_weak_ref.is_valid()) {
		return p_weak_ref->get_ref();
	}
	return {};
}
// inline static Variant get_ref(const Ref<WeakRef> &p_weak_ref) { return get_ref(p_weak_ref); }