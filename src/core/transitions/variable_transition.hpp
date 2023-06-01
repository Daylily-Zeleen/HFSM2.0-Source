#pragma once

#include "transition_base.hpp"
#include "variable_expressions/comparation_expression.hpp"
#include "variable_expressions/trigger_expression.hpp"

namespace Hfsm {

class SoloTriggerExpression;
class UnionTriggerExpression;
class VariableExpression;

class VariableTransition : public TransitionBase {
public:
	bool can_transit() override;
	~VariableTransition();

private:
	bool and_mode = false;
	PackedStringArray forece_trigger_list;

	Vector<SoloTriggerExpression *> solo_triggers;
	Vector<UnionTriggerExpression *> union_triggers;
	Vector<VariableExpression *> normal_expressions;

	friend class TransitionRes;
};

} // namespace Hfsm
