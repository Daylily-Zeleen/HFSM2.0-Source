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
	CRASH_COND(!Variant::can_convert(m_value_type, (m_variable)->get_type()))

#ifdef TOOLS_ENABLED
#define type_convertable_check(m_variable, m_value_type)   \
	if (Engine::get_singleton()->is_editor_hint()) {       \
		_type_convertable_check(m_variable, m_value_type); \
	}
#else
#define type_convertable_check(m_variable, m_value_type) _type_convertable_check(m_variable, m_value_type)
#endif // TOOLS_ENABLED

// ComparationExpression
ComparationExpression::ComparationExpression(const Ref<Variable> &p_variable, uint8_t p_op) :
		VariableExpression(p_variable) {
	ERR_FAIL_COND(!(p_op >= 0 && p_op < 6));
	op = p_op;
}

// ConstantComparationExpression
ConstantComparationExpression::ConstantComparationExpression(
		const Ref<Variable> &p_variable, uint8_t p_op, const Variant &p_value) :
		ComparationExpression(p_variable, p_op) {
	type_convertable_check(variable, p_value.get_type());
	value = p_value;
}

// VariableComparationExpression
VariableComparationExpression::VariableComparationExpression(
		const Ref<Variable> &p_variable, uint8_t p_op, const Ref<Variable> &p_value) :
		ComparationExpression(p_variable, p_op) {
	type_convertable_check(variable, p_value->get_type());
	value = p_value;
}

} // namespace Hfsm
