#include "comparation_expression.h"

#ifdef TOOLS_ENABLED
#ifdef GDEXTENSION_BUILD
#include <godot_cpp/classes/engine.hpp>
#else
#include <core/config/engine.h>
#endif // GDEXTENSION_BUILD
#endif // TOOLS_ENABLED
namespace Hfsm {

#define _type_convertable_check(m_variable, m_value_type) \
	CRASH_COND(!Variant::can_convert(m_value_type, (m_variable)->get_variable_type()))

#ifdef TOOLS_ENABLED
#define type_convertable_check(m_variable, m_value_type)   \
	if (Engine::get_singleton()->is_editor_hint()) {       \
		_type_convertable_check(m_variable, m_value_type); \
	}
#else
#define type_convertable_check(m_variable, m_value_type) _type_convertable_check(m_variable, m_value_type)
#endif // TOOLS_ENABLED

// ComparationExpression
ComparationExpression::ComparationExpression(const Ref<Variable> &p_variable, Comparator p_comparator) :
		VariableExpression(p_variable) {
	ERR_FAIL_COND(!(p_comparator >= 0 && p_comparator < 6));
	comparator = p_comparator;
}

// ConstantComparationExpression
ConstantComparationExpression::ConstantComparationExpression(
		const Ref<Variable> &p_variable, Comparator p_comparator, const Variant &p_value) :
		ComparationExpression(p_variable, p_comparator) {
	type_convertable_check(variable, p_value.get_type());
	value = p_value;
}

bool ConstantComparationExpression::get_result(bool p_and_mode,
		bool &r_result) {
	r_result = compare_with(variable, comparator, value);
	// 与 + 假  or 或 + 真
	if ((p_and_mode && !r_result) || (!p_and_mode && r_result)) {
		return true;
	} else {
		return false;
	}
}

// VariableComparationExpression
VariableComparationExpression::VariableComparationExpression(
		const Ref<Variable> &p_variable, Comparator p_comparator, const Ref<Variable> &p_value) :
		ComparationExpression(p_variable, p_comparator) {
	type_convertable_check(variable, p_value->get_variable_type());
	value = p_value;
}

bool VariableComparationExpression::get_result(bool p_and_mode,
		bool &r_result) {
	r_result = compare_with(variable, comparator, value.ptr());
	// 与 + 假  or 或 + 真
	if ((p_and_mode && !r_result) || (!p_and_mode && r_result)) {
		return true;
	} else {
		return false;
	}
}

} // namespace Hfsm
