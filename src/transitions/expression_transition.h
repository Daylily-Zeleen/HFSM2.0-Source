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

	void set_expression_text(const String &expression_text);

private:
	Expression expression;
	HFSM *hfsm = nullptr;

	bool invalid = true;

	friend class TransitionRes;
};

} // namespace Hfsm
