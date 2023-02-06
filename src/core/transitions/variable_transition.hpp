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
	bool _and_mode = false;
	PackedStringArray _forece_trigger_list;

	Vector<SoloTriggerExpression *> _solo_triggers;
	Vector<UnionTriggerExpression *> _union_triggers;
	Vector<VariableExpression *> _normal_expressions;

	friend class TransitionRes;
};

} // namespace Hfsm
