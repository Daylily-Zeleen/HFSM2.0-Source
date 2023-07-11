#pragma once

#include "../../variable.h"

namespace Hfsm {

class VariableExpression {
protected:
	VariableExpression(const Ref<Variable> &p_variable) { variable = p_variable; }

public:
	enum ExpressionType {
		EXPRESSION_TYPE_NORMAL,
		EXPRESSION_TYPE_UNION_TRIGGER,
		EXPRESSION_TYPE_SOLO_TRIGGER,
	};

	/**
	 * @brief 判断是否可以推进
	 *
	 * @param and_mode 与其他表达式是否为 与逻辑
	 * @param r_result 返回当前的判定结果是否为真
	 * @return true 已得到总结果
	 * @return false 未能确定总结果
	 */
	virtual bool get_result(bool p_and_mode, bool &r_result) = 0;

	/**
	 * @brief Get the expression typ object
	 *
	 * @return uint8_t 0 通用 1 联合触发器 2 SOLO 触发器
	 */
	virtual ExpressionType get_expression_type() { return ExpressionType::EXPRESSION_TYPE_NORMAL; }

protected:
	// 变量
	Ref<Variable> variable;
};

} // namespace Hfsm
