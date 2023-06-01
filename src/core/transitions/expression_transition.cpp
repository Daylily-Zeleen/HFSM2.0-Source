#include "expression_transition.hpp"
#include "../state.hpp"

namespace Hfsm {

#pragma region ExpressionTransition

void ExpressionTransition::set_expression_text(const String &expression_text) {
	if (expression.parse(expression_text, hfsm->get_expression_objs_names()) != OK) {
		valid = true;
	} else {
		valid = false;
	}
}

bool ExpressionTransition::can_transit() {
	ERR_FAIL_COND_V(!is_vaild(), false);
	auto result = expression.execute(hfsm->get_expression_objs(), hfsm, false);
	if (expression.has_execute_failed()) {
		WARN_PRINT_ONCE(String("Hfsm: The ExpressionTransition '") +
				String(get_from_state()->get_name()) + String("'->'") +
				String(get_to_state()->get_name()) + String("' of '") +
				(hfsm->get_owner()
								? String(hfsm->get_owner()->get_name())
								: String("")) +
				String("/") + hfsm->get_name() +
				String("' has execute failed,please check it."));
		return false;
	} else {
		return result.booleanize();
	}
}
#pragma endregion

} // namespace Hfsm