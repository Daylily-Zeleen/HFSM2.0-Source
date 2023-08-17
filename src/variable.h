/**************************************************************************/
/*  variable.h                                                            */
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

#include "../hfsm_global.h"

#ifdef GDEXTENSION_BUILD
#ifdef TOOLS_ENABLED
#include <godot_cpp/classes/engine.hpp>
#endif // TOOLS_ENABLED
#include <godot_cpp/classes/ref_counted.hpp>
using namespace godot;
#else
#ifdef TOOLS_ENABLED
#include <core/config/engine.h>
#endif // TOOLS_ENABLED
#include <core/object/ref_counted.h>

#endif // GDEXTENSION_BUILD

namespace Hfsm {

// 变量类
class Variable : public RefCounted {
	GDCLASS(Variable, RefCounted)
protected:
	static void _bind_methods();

public:
	StringName get_variable_name() const;

	Variant::Type get_variable_type() const;

	Variant get_value() const;
	void set_value(const Variant &p_value);

	bool is_trigger() const;
	// 触发器专用
	void trigger();
	void flush_trigger();

	Variable() = default;
	Variable(const StringName &p_variable_name, Variant::Type p_type, const Variant &p_default_value) :
			variable_name(p_variable_name), type(p_type), value(p_default_value) {}

private:
	StringName variable_name = "";
	Variant::Type type = Variant::NIL;
	Variant value;
};

#pragma region 内联实现
inline StringName Variable::get_variable_name() const { return variable_name; }

inline Variant Variable::get_value() const { return value; }
inline void Variable::set_value(const Variant &p_value) {
	// 触发器特殊处理
	if (type == Variant::NIL) {
		value = Variant(true);
		return;
	}

#ifdef TOOLS_ENABLED
	if (!Engine::get_singleton()->is_editor_hint()) {
		CRASH_COND(!Variant::can_convert(p_value.get_type(), type));
	}
#endif // TOOLS_ENABLED
	value = p_value;
}

inline bool Variable::is_trigger() const { return get_variable_type() == Variant::NIL; }
inline void Variable::trigger() {
	ERR_FAIL_COND(type != Variant::NIL);
	value = Variant(true);
}

inline Variant::Type Variable::get_variable_type() const { return type; }

// 触发器专用
inline void Variable::flush_trigger() { value = false; }

#pragma endregion
}; // namespace Hfsm
