/**************************************************************************/
/*  variable_config.h                                                     */
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
#include <godot_cpp/classes/resource.hpp>
using namespace godot;
#else
#include <core/io/resource.h>
#endif // GDEXTENSION_BUILD

namespace Hfsm {
class Variable;
class FSMConfig;

class VariableConfig : public Resource {
	GDCLASS(VariableConfig, Resource)

protected:
	static void _bind_methods();

	_TO_STRING()

public:
	bool _set(const StringName &p_name, const Variant &p_property);
	bool _get(const StringName &p_name, Variant &r_property) const;
	void _get_property_list(List<PropertyInfo> *p_list) const;

	void set_variable_name(const StringName &p_name);
	StringName get_variable_name();

	void set_type(Variant::Type p_t);
	Variant::Type get_type() const;

	void set_comment(const String &p_comment);
	String get_comment() const;

	void set_default_value(const Variant &p_default_val);
	Variant get_default_value() const; //  { return _default_val; }

	Ref<Variable> create_variable();

	void set_fsm_config(const Ref<FSMConfig> &p_fsm_config);
	Ref<FSMConfig> get_fsm_config() const;

	static Ref<VariableConfig> create_new(const Ref<FSMConfig> &p_fsm_config);

	String get_type_text() const;

private:
	StringName variable_name = "variable";
	Ref<FSMConfig> fsm_config;
	Variant::Type type = Variant::NIL;
	Variant default_value;
	String comment = "";
};

} // namespace Hfsm
