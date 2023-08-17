/**************************************************************************/
/*  trigger_expression.cpp                                                */
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

#include "trigger_expression.h"

#ifdef TOOLS_ENABLED
#ifdef GDEXTENSION_BUILD
#include <godot_cpp/classes/engine.hpp>
#else
#include <core/config/engine.h>
#endif // GDEXTENSION_BUILD
#endif // TOOLS_ENABLED

namespace Hfsm {

#pragma region TriggerExpression
TriggerExpression::TriggerExpression(const Ref<Variable> &variable) :
		VariableExpression(variable) {
#ifdef TOOLS_ENABLED
	if (!Engine::get_singleton()->is_editor_hint()) {
		CRASH_COND(variable->get_variable_type() != Variant::NIL);
	}
#endif // TOOLS_ENABLED
}

bool TriggerExpression::get_result(bool p_and_mode, bool &r_result) {
	r_result = variable->get_value();
	// 与 + 假  or 或 + 真
	if ((p_and_mode && !r_result) || (!p_and_mode && r_result)) {
		return true;
	} else {
		return false;
	}
}

#pragma endregion

#pragma region SoloTriggerExpression
SoloTriggerExpression::SoloTriggerExpression(const Ref<Variable> &variable) :
		TriggerExpression(variable) {}

//  独立触发器只关注自己
bool SoloTriggerExpression::get_result(bool p_and_mode, bool &r_result) {
	r_result = variable->get_value();
	if (r_result) {
		return true;
	} else {
		return false;
	}
}

VariableExpression::ExpressionType SoloTriggerExpression::get_expression_type() {
	return ExpressionType::EXPRESSION_TYPE_SOLO_TRIGGER;
}
#pragma endregion

#pragma region UnionTrigger
UnionTriggerExpression::UnionTriggerExpression(const Ref<Variable> &variable) :
		TriggerExpression(variable) {}

bool UnionTriggerExpression::get_result(bool p_and_mode, bool &r_result) {
	r_result = variable->get_value();
	if (!r_result) {
		return true;
	}
	return false;
}

VariableExpression::ExpressionType UnionTriggerExpression::get_expression_type() {
	return ExpressionType::EXPRESSION_TYPE_UNION_TRIGGER;
}
#pragma endregion

} // namespace Hfsm
