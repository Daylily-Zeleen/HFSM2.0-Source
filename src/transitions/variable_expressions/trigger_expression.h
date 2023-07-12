#pragma once

#include "variable_expression.h"

namespace Hfsm {

/**
 * @brief 普通触发器
 * 与其他类型的变量表达式一起考虑
 */
class TriggerExpression : public VariableExpression {
public:
	TriggerExpression(const Ref<Variable> &p_variable);

	bool get_result(bool p_and_mode, bool &r_result) override;
};

/**
 * @brief 独立触发器
 * 只考虑自己是否触发
 */
class SoloTriggerExpression : public TriggerExpression {
public:
	SoloTriggerExpression(const Ref<Variable> &p_variable);
	//  独立触发器只关注自己
	bool get_result(bool p_and_mode, bool &r_result) override;
	ExpressionType get_expression_type() override;
};

/**
 * @brief 联合触发器
 * 同时考虑其他 联合触发器（在上层检查时分离）
 */
class UnionTriggerExpression : public TriggerExpression {
public:
	UnionTriggerExpression(const Ref<Variable> &p_variable);

	bool get_result(bool p_and_mode, bool &r_result) override;
	ExpressionType get_expression_type() override;
};

} // namespace Hfsm
