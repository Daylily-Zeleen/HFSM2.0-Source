/**************************************************************************/
/*  expression_transition.cpp                                             */
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

#include "expression_transition.h"

#include "../../hfsm_global.h"
#include "../state.h"

namespace HFSM2 {

#pragma region ExpressionTransition

bool ExpressionTransition::can_transit() {
	ERR_FAIL_COND_V_MSG(invalid, false, godot::vformat("Expression Transition \"%s\"->\"%s\" invalid: %s", get_from_state()->get_name(), get_to_state()->get_name(), expression.get_error_text()));
	auto result = expression.execute(HFSMGlobal::get_singletons(), hfsm, true);
	if (expression.has_execute_failed()) {
		IF_DEBUG({
			WARN_PRINT_ONCE(String("HFSM: The ExpressionTransition '") +
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

} // namespace HFSM2
