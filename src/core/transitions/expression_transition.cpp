#include "expression_transition.hpp"
#include "../state.hpp"

namespace Hfsm {

#pragma region ExpressionTransition

void ExpressionTransition::set_expression_text(const String &expression_text) {
    if (_expression.parse(expression_text,
                          _hfsm->get_expression_objs_names()) != OK) {
        _valid = true;
    } else
        _valid = false;
}

bool ExpressionTransition::can_transit() {
    ERR_FAIL_COND_V(!is_vaild_expression(), false);
    auto result =
        _expression.execute(_hfsm->get_expression_objs(), _hfsm, false);
    if (_expression.has_execute_failed()) {
        WARN_PRINT_ONCE(String("Hfsm: The ExpressionTransition '") +
                        String(get_from_state()->get_name()) + String("'->'") +
                        String(get_to_state()->get_name()) + String("' of '") +
                        (_hfsm->get_owner()
                             ? String(_hfsm->get_owner()->get_name())
                             : String("")) +
                        String("/") + _hfsm->get_name() +
                        String("' has execute failed,please check it."));
        return false;
    } else {
        return result.booleanize();
    }
}
#pragma endregion

} // namespace Hfsm