/**************************************************************************/
/*  compare_expression.cpp                                                */
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

#include "compare_expression.h"

#ifdef TOOLS_ENABLED
#ifdef GDEXTENSION_BUILD
#include <godot_cpp/classes/engine.hpp>
#else
#include <core/config/engine.h>
#endif // GDEXTENSION_BUILD
#endif // TOOLS_ENABLED
namespace HFSM2 {

#define _type_convertible_check(m_variable, m_value_type) \
	CRASH_COND(!Variant::can_convert(m_value_type, (m_variable)->get_variable_type()))

#ifdef TOOLS_ENABLED
#define type_convertible_check(m_variable, m_value_type)   \
	if (Engine::get_singleton()->is_editor_hint()) {       \
		_type_convertible_check(m_variable, m_value_type); \
	}
#else
#define type_convertible_check(m_variable, m_value_type) _type_convertible_check(m_variable, m_value_type)
#endif // TOOLS_ENABLED

// CompareExpression
CompareExpression::CompareExpression(const Ref<Variable> &p_variable, Comparator p_comparator) :
		VariableExpression(p_variable) {
	ERR_FAIL_COND(!(p_comparator >= 0 && p_comparator < 6));
	comparator = p_comparator;
}

// ConstantCompareExpression
ConstantCompareExpression::ConstantCompareExpression(
		const Ref<Variable> &p_variable, Comparator p_comparator, const Variant &p_value) :
		CompareExpression(p_variable, p_comparator) {
	type_convertible_check(variable, p_value.get_type());
	value = p_value;
}

bool ConstantCompareExpression::get_result(bool p_and_mode,
		bool &r_result) {
	r_result = compare_with(variable, comparator, value);
	// 与 + 假  or 或 + 真
	if ((p_and_mode && !r_result) || (!p_and_mode && r_result)) {
		return true;
	} else {
		return false;
	}
}

// VariableCompareExpression
VariableCompareExpression::VariableCompareExpression(
		const Ref<Variable> &p_variable, Comparator p_comparator, const Ref<Variable> &p_value) :
		CompareExpression(p_variable, p_comparator) {
	type_convertible_check(variable, p_value->get_variable_type());
	value = p_value;
}

bool VariableCompareExpression::get_result(bool p_and_mode,
		bool &r_result) {
	r_result = compare_with(variable, comparator, value.ptr());
	// 与 + 假  or 或 + 真
	if ((p_and_mode && !r_result) || (!p_and_mode && r_result)) {
		return true;
	} else {
		return false;
	}
}

} // namespace HFSM2
