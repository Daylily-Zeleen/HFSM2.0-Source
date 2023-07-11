#include "expression_transition.h"

#include "../../hfsm_global.h"
#include "../state.h"

namespace Hfsm {

#pragma region ExpressionTransition

bool ExpressionTransition::can_transit() {
	ERR_FAIL_COND_V(invalid, false);
	auto result = expression.execute(HfsmGlobal::get_singletons(), hfsm, false);
	if (expression.has_execute_failed()) {
		IF_DEBUG({
			WARN_PRINT_ONCE(String("Hfsm: The ExpressionTransition '") +
					String(get_from_state()->get_name()) + String("'->'") +
					String(get_to_state()->get_name()) + String("' of '") +
					(hfsm->get_owner()
									? String(hfsm->get_owner()->get_name())
									: String("")) +
					String("/") + hfsm->get_name() +
					String("' has execute failed,please check it."));
		})
		return false;
	} else {
		return result.booleanize();
	}
}
#pragma endregion

} // namespace Hfsm