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
