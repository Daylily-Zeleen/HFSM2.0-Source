/**************************************************************************/
/*  variable_expression_config.h                                          */
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

#ifdef GDEXTENSION_BUILD
#include <godot_cpp/classes/resource.hpp>

using namespace godot;
#else // GDEXTENSION_BUILD
#include <core/io/resource.h>

#endif // GDEXTENSION_BUILD

#include <src/variable_config.h>

#include "compare_expression.h"

namespace HFSM2 {

class HFSM;
class VariableExpression;

class VariableExpressionConfig : public Resource {
	GDCLASS(VariableExpressionConfig, Resource)

protected:
	static void _bind_methods();

	_TO_STRING()
public:
	bool _set(const StringName &p_name, const Variant &p_property);
	bool _get(const StringName &p_name, Variant &r_property) const;
	void _get_property_list(List<PropertyInfo> *p_list) const;

	enum TriggerType {
		TRIGGER_TYPE_SOLO,
		TRIGGER_TYPE_UNION,
		TRIGGER_TYPE_NORMAL,
		TRIGGER_TYPE_MAX,
	};

	using Comparator = CompareExpression::Comparator;

	void set_variable_config(const Ref<VariableConfig> &p_variable_config);
	Ref<VariableConfig> get_variable_config() const;

	void set_value(const Variant &p_value);
	Variant get_value() const;

	void set_comparator(Comparator p_cmp);
	Comparator get_comparator() const;

	void set_trigger_type(TriggerType p_trigger_type);
	TriggerType get_trigger_type() const;

	void set_variable_as_value(bool p_variable_as_value);
	bool is_variable_as_value() const;

	VariableExpression *create_variable_expression(HFSM *p_hfsm);

#if TOOLS_ENABLED
	Array debug_serialize(const Ref<FSMConfig> &p_root_config) const;
	static Ref<VariableExpressionConfig> debug_deserialize(const Array &p_data, const Ref<FSMConfig> &p_root_config);
#endif // TOOLS_ENABLED

private:
	Ref<VariableConfig> variable_config;
	Variant value;
	Comparator comparator = Comparator::COMPARATOR_EQUAL;
	// trigger
	TriggerType trigger_type = TRIGGER_TYPE_SOLO;

	// 是否使用另一个 变量资源作为 比较值
	bool variable_as_value = false;
};

} // namespace HFSM2

VARIANT_ENUM_CAST(HFSM2::VariableExpressionConfig::TriggerType);
VARIANT_ENUM_CAST(HFSM2::VariableExpressionConfig::Comparator);
