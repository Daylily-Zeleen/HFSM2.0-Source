#pragma once

#include "transition_base.h"
#include "variable_expressions/comparation_expression.h"
#include "variable_expressions/trigger_expression.h"

namespace Hfsm {

class SoloTriggerExpression;
class UnionTriggerExpression;
class VariableExpression;

class VariableExpressionsTransition : public TransitionBase {
public:
	bool can_transit() override;
	~VariableExpressionsTransition();

private:
	bool and_mode = false;
	PackedStringArray forece_trigger_list;

	Vector<SoloTriggerExpression *> solo_triggers;
	Vector<UnionTriggerExpression *> union_triggers;
	Vector<VariableExpression *> normal_expressions;

	friend class TransitionConfig;
};

} // namespace Hfsm
