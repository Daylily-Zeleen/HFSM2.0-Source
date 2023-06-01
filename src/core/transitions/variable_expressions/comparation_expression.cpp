#include "comparation_expression.hpp"
#include "../../../hfsm_global.hpp"

#include <godot_cpp/classes/engine.hpp>
namespace Hfsm {

#pragma region ComparationExpression
ComparationExpression::ComparationExpression(const Ref<HFSMVariable> &p_variable, uint8_t p_op) :
		VariableExpression(p_variable) {
	ERR_FAIL_COND(!(p_op >= 0 && p_op < 6));
	op = p_op;
}
#pragma endregion // ComparationExpression

#pragma region ConstantComparationExpression
ConstantComparationExpression::ConstantComparationExpression(
		const Ref<HFSMVariable> &p_variable, uint8_t p_op, const Variant &p_value) :
		ComparationExpression(p_variable, p_op) {
#ifdef TOOLS_ENABLED
	if (!Engine::get_singleton()->is_editor_hint()) {
#endif
		CRASH_COND(!Variant::can_convert(p_value.get_type(), variable->get_type()));
#ifdef TOOLS_ENABLED
	}
#endif
	value = p_value;
}

#pragma endregion // ConstantComparationExpression

#pragma region VariableComparationExpression
VariableComparationExpression::VariableComparationExpression(
		const Ref<HFSMVariable> &p_variable, uint8_t p_op, const Ref<HFSMVariable> &p_value) :
		ComparationExpression(p_variable, p_op) {
#ifdef TOOLS_ENABLED
	if (!Engine::get_singleton()->is_editor_hint()) {
#endif
		CRASH_COND(!Variant::can_convert(p_value->get_type(), variable->get_type()));
#ifdef TOOLS_ENABLED
	}
#endif
	value = p_value;
}

#pragma endregion // VariableComparationExpression

} // namespace Hfsm
