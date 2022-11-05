#include "comparation_expression.hpp"
#include "../../../hfsm_global.hpp"

#include <godot_cpp/classes/engine.hpp>
namespace Hfsm {

#pragma region ComparationExpression
ComparationExpression::ComparationExpression(const Ref<HFSMVariable> &variable,
                                             uint8_t op)
    : VariableExpression(variable) {
    ERR_FAIL_COND(!(op >= 0 && op < 6));
    _op = op;
}
#pragma endregion

#pragma region ConstantComparationExpression
ConstantComparationExpression::ConstantComparationExpression(
    const Ref<HFSMVariable> &variable, uint8_t op, const Variant &value)
    : ComparationExpression(variable, op) {
    if (!Engine::get_singleton()->is_editor_hint()) {
        CRASH_COND(
            !Variant::can_convert(value.get_type(), _variable->get_type()));
    }
    _value = value;
}

#pragma endregion

#pragma region VariableComparationExpression
VariableComparationExpression::VariableComparationExpression(
    Ref<HFSMVariable> variable, uint8_t op, Ref<HFSMVariable> value)
    : ComparationExpression(variable, op) {
    if (!Engine::get_singleton()->is_editor_hint()) {
        CRASH_COND(
            !Variant::can_convert(value->get_type(), _variable->get_type()));
    }
    _value = value;
}

#pragma endregion

} // namespace Hfsm
