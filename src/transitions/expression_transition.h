#pragma once

#ifdef GDEXTENSION_BUILD
#include <godot_cpp/classes/expression.hpp>
#else
#include <core/math/expression.h>
#endif // GDEXTENSION_BUILD

#include "transition_base.h"

namespace Hfsm {
// 表达式转换
class ExpressionTransition : public TransitionBase {
public:
	bool can_transit() override;

	ExpressionTransition(HFSM *p_hfsm, const String &p_expression_text) :
			hfsm(p_hfsm) {
		invalid = expression.parse(p_expression_text, HfsmGlobal::get_singleton_names()) != OK;
	}

private:
	Expression expression;
	HFSM *hfsm = nullptr;

	bool invalid = true;
};

} // namespace Hfsm
